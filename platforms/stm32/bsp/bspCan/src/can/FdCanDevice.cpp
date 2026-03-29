// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file FdCanDevice.cpp
 * \brief Implementation of the STM32G4 FDCAN hardware abstraction layer.
 *
 * ## Design overview
 *
 * ### STM32G4 FDCAN message RAM layout
 * Each FDCAN instance owns a fixed 212-word (848-byte) region in SRAMCAN.
 * FDCAN1 starts at SRAMCAN_BASE + 0x000, FDCAN2 at +0x350, FDCAN3 at +0x6A0.
 * Within each region the layout is:
 *   - Standard ID filters  (28 elements x 1 word)
 *   - Extended ID filters  ( 8 elements x 2 words)
 *   - RX FIFO 0            ( 3 elements x 4 words)
 *   - RX FIFO 1            ( 3 elements x 4 words)
 *   - TX Event FIFO        ( 3 elements x 2 words)
 *   - TX Buffers           ( 3 elements x 4 words)
 * See the detailed offset table in the code below.
 *
 * ### Circular RX queue (fRxHead + fRxCount)
 * receiveISR() copies frames from the hardware RX FIFO 0 into a fixed-size
 * circular buffer (fRxQueue[RX_QUEUE_SIZE]).  New frames are stored at
 * index `(fRxHead + fRxCount) % RX_QUEUE_SIZE`; the thread retrieves them
 * starting at fRxHead via getRxFrame() and advances the head with
 * clearRxQueue().  When the software queue is full, hardware FIFO elements
 * are still acknowledged (to prevent FIFO overrun) but the payload is
 * silently dropped.
 *
 * ### ISR re-entry prevention (snapshot drain)
 * receiveISR() reads the FIFO fill level (F0FL) once at entry and drains
 * only that many elements.  This prevents an infinite loop on a busy bus
 * where new frames arrive faster than the ISR can consume them.  After
 * draining, RF0N / RF0F / RF0L interrupt flags are cleared atomically
 * (write-1-to-clear on FDCAN->IR).
 *
 * ### ILS grouped bits (STM32G4-specific)
 * Unlike M_CAN on STM32H7 which offers per-source interrupt routing,
 * the STM32G4 FDCAN groups interrupt sources into coarse categories.
 * Setting FDCAN_ILS_SMSG routes the "Successful Message" group
 * (which includes TX Complete) to interrupt line 1, while the RX FIFO 0
 * group defaults to interrupt line 0.  Both lines are enabled via ILE.
 */

#include <can/FdCanDevice.h>

namespace bios
{

// STM32G4 FDCAN message RAM layout (fixed, per instance):
// Each FDCAN instance has 212 words (848 bytes) of message RAM.
// FDCAN1 starts at SRAMCAN_BASE + 0x000
// FDCAN2 starts at SRAMCAN_BASE + 0x350
// FDCAN3 starts at SRAMCAN_BASE + 0x6A0
//
// Layout within each instance (offsets from instance base):
//   Standard ID filters: 28 elements x 1 word  = 28 words  (0x000 - 0x06F)
//   Extended ID filters:  8 elements x 2 words  = 16 words  (0x070 - 0x0AF)
//   RX FIFO0:            3 elements x 4 words  = 12 words  (0x0B0 - 0x0DF)
//   RX FIFO1:            3 elements x 4 words  = 12 words  (0x0E0 - 0x10F)
//   TX Event FIFO:       3 elements x 2 words  =  6 words  (0x110 - 0x127)
//   TX Buffers:          3 elements x 4 words  = 12 words  (0x128 - 0x157)

static uint32_t getInstanceRamBase(FDCAN_GlobalTypeDef* fdcan)
{
    if (fdcan == FDCAN1)
    {
        return SRAMCAN_BASE;
    }
#if defined(FDCAN2)
    if (fdcan == FDCAN2)
    {
        return SRAMCAN_BASE + 0x350U;
    }
#endif
#if defined(FDCAN3)
    if (fdcan == FDCAN3)
    {
        return SRAMCAN_BASE + 0x6A0U;
    }
#endif
    return SRAMCAN_BASE;
}

static constexpr uint32_t STD_FILTER_OFFSET = 0x000U;
static constexpr uint32_t RX_FIFO0_OFFSET   = 0x0B0U;
// STM32G4 message RAM: TX buffers at 0x278 (not 0x128 as standard M_CAN)
static constexpr uint32_t TX_BUFFER_OFFSET  = 0x278U;

FdCanDevice::FdCanDevice(Config const& config)
: fConfig(config), fRxQueue{}, fRxHead(0U), fRxCount(0U), fInitialized(false), fFrameSentCallback()
{}

FdCanDevice::FdCanDevice(Config const& config, ::etl::delegate<void()> frameSentCallback)
: fConfig(config)
, fRxQueue{}
, fRxHead(0U)
, fRxCount(0U)
, fInitialized(false)
, fFrameSentCallback(frameSentCallback)
{}

void FdCanDevice::enablePeripheralClock()
{
    // Enable FDCAN clock on APB1
    RCC->APB1ENR1 |= RCC_APB1ENR1_FDCANEN;
    uint32_t volatile dummy = RCC->APB1ENR1;
    (void)dummy;

    // Select FDCAN kernel clock = PCLK1 (FDCANSEL=10 in CCIPR1[25:24])
    // Default after reset is HSE (00), which may not be enabled.
    RCC->CCIPR = (RCC->CCIPR & ~(3U << 24U)) | (2U << 24U);
}

void FdCanDevice::configureGpio()
{
    GPIO_TypeDef* txPort = fConfig.txGpioPort;
    GPIO_TypeDef* rxPort = fConfig.rxGpioPort;

    txPort->MODER &= ~(3U << (fConfig.txPin * 2U));
    txPort->MODER |= (2U << (fConfig.txPin * 2U));
    txPort->OSPEEDR |= (3U << (fConfig.txPin * 2U));
    if (fConfig.txPin < 8U)
    {
        txPort->AFR[0] &= ~(0xFU << (fConfig.txPin * 4U));
        txPort->AFR[0] |= (static_cast<uint32_t>(fConfig.txAf) << (fConfig.txPin * 4U));
    }
    else
    {
        txPort->AFR[1] &= ~(0xFU << ((fConfig.txPin - 8U) * 4U));
        txPort->AFR[1] |= (static_cast<uint32_t>(fConfig.txAf) << ((fConfig.txPin - 8U) * 4U));
    }

    rxPort->MODER &= ~(3U << (fConfig.rxPin * 2U));
    rxPort->MODER |= (2U << (fConfig.rxPin * 2U));
    rxPort->PUPDR &= ~(3U << (fConfig.rxPin * 2U));
    rxPort->PUPDR |= (1U << (fConfig.rxPin * 2U));
    if (fConfig.rxPin < 8U)
    {
        rxPort->AFR[0] &= ~(0xFU << (fConfig.rxPin * 4U));
        rxPort->AFR[0] |= (static_cast<uint32_t>(fConfig.rxAf) << (fConfig.rxPin * 4U));
    }
    else
    {
        rxPort->AFR[1] &= ~(0xFU << ((fConfig.rxPin - 8U) * 4U));
        rxPort->AFR[1] |= (static_cast<uint32_t>(fConfig.rxAf) << ((fConfig.rxPin - 8U) * 4U));
    }
}

static constexpr uint32_t INIT_TIMEOUT_CYCLES = 100000U;

bool FdCanDevice::enterInitMode()
{
    fConfig.baseAddress->CCCR |= FDCAN_CCCR_INIT;
    uint32_t timeout = INIT_TIMEOUT_CYCLES;
    while ((fConfig.baseAddress->CCCR & FDCAN_CCCR_INIT) == 0U)
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }
    // Enable configuration change
    fConfig.baseAddress->CCCR |= FDCAN_CCCR_CCE;
    return true;
}

bool FdCanDevice::leaveInitMode()
{
    fConfig.baseAddress->CCCR &= ~FDCAN_CCCR_INIT;
    uint32_t timeout = INIT_TIMEOUT_CYCLES;
    while ((fConfig.baseAddress->CCCR & FDCAN_CCCR_INIT) != 0U)
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }
    return true;
}

void FdCanDevice::configureBitTiming()
{
    // Nominal bit timing for classic CAN mode
    // NBTP: NSJW | NBRP | NTSEG1 | NTSEG2
    fConfig.baseAddress->NBTP = ((fConfig.nsjw) << FDCAN_NBTP_NSJW_Pos)
                                | ((fConfig.prescaler - 1U) << FDCAN_NBTP_NBRP_Pos)
                                | ((fConfig.nts1) << FDCAN_NBTP_NTSEG1_Pos)
                                | ((fConfig.nts2) << FDCAN_NBTP_NTSEG2_Pos);
}

void FdCanDevice::configureMessageRam()
{
    // STM32G4 has fixed message RAM layout — no configuration registers.
    // RXGFC configures global filter behavior and list sizes only.
    fConfig.baseAddress->RXGFC = (0U << FDCAN_RXGFC_LSS_Pos) | (0U << FDCAN_RXGFC_LSE_Pos);

    // TX buffer: use FIFO/queue mode
    fConfig.baseAddress->TXBC = 0U; // FIFO mode (TFQM=0)
}

void FdCanDevice::init()
{
    enablePeripheralClock();
    configureGpio();
    enterInitMode();

    // Classic CAN mode (no FD), auto retransmission disabled
    fConfig.baseAddress->CCCR &= ~FDCAN_CCCR_FDOE;
    fConfig.baseAddress->CCCR &= ~FDCAN_CCCR_BRSE;

    configureBitTiming();
    configureMessageRam();
    configureAcceptAllFilter();

    fInitialized = true;
}

void FdCanDevice::start()
{
    if (!fInitialized)
    {
        return;
    }

    // Configure HW filter in init mode (before leaveInitMode)
    if (fFilterIds != nullptr && fFilterCount > 0U)
    {
        configureFilterList(fFilterIds, fFilterCount);
    }
    else
    {
        configureAcceptAllFilter();
    }

    // Enable RX interrupt only at startup. TCE is managed per-TX by
    // transmit(frame, true) and disabled by transmitISR() — matching S32K's
    // selective per-buffer interrupt enable/disable pattern.
    fConfig.baseAddress->IE     = FDCAN_IE_RF0NE;
    fConfig.baseAddress->ILS    = 0U;
    fConfig.baseAddress->ILE    = FDCAN_ILE_EINT0;
    fConfig.baseAddress->TXBTIE = 0x7U; // Required for TC generation per RM0440

    leaveInitMode();

    // Write IE again after leaving init mode (persistence workaround).
    fConfig.baseAddress->IE = FDCAN_IE_RF0NE;
}

void FdCanDevice::stop()
{
    fConfig.baseAddress->IE &= ~(FDCAN_IE_RF0NE | FDCAN_IE_TCE);
    enterInitMode();
}

bool FdCanDevice::transmit(::can::CANFrame const& frame)
{
    FDCAN_GlobalTypeDef* fdcan = fConfig.baseAddress;

    // Check TX FIFO free level
    if ((fdcan->TXFQS & FDCAN_TXFQS_TFFL) == 0U)
    {
        return false; // TX FIFO full
    }

    // Get put index
    uint32_t putIdx = (fdcan->TXFQS & FDCAN_TXFQS_TFQPI) >> FDCAN_TXFQS_TFQPI_Pos;

    // Calculate TX buffer element address in message RAM
    // STM32G4 FDCAN uses 18-word (72-byte) element spacing in message RAM,
    // regardless of configured data field size (TXESC). Section boundaries
    // are fixed at max element size.
    static constexpr uint32_t TX_ELEMENT_SIZE = 72U; // 18 words × 4 bytes
    uint32_t ramBase = getInstanceRamBase(fdcan);
    uint32_t* txBuf  = reinterpret_cast<uint32_t*>(ramBase + TX_BUFFER_OFFSET + (putIdx * TX_ELEMENT_SIZE));

    // Word 0: ID
    uint32_t id = frame.getId();
    if ((id & 0x80000000U) != 0U)
    {
        txBuf[0] = ((id & 0x1FFFFFFFU)) | (1U << 30U); // XTD bit
    }
    else
    {
        txBuf[0] = ((id & 0x7FFU) << 18U);
    }

    // Word 1: DLC, no FD, no BRS
    uint8_t dlc = frame.getPayloadLength();
    txBuf[1]    = (static_cast<uint32_t>(dlc) << 16U);

    // Words 2-3: Data
    uint8_t const* data = frame.getPayload();
    txBuf[2]            = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8U)
               | (static_cast<uint32_t>(data[2]) << 16U) | (static_cast<uint32_t>(data[3]) << 24U);
    txBuf[3] = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8U)
               | (static_cast<uint32_t>(data[6]) << 16U) | (static_cast<uint32_t>(data[7]) << 24U);

    // Request transmission
    fdcan->TXBAR = (1U << putIdx);

    return true;
}

bool FdCanDevice::transmit(::can::CANFrame const& frame, bool txInterruptNeeded)
{
    if (txInterruptNeeded)
    {
        // Match S32K enableTransmitInterrupt(): clear flag first, then enable mask.
        // FlexCANDevice.h:132-136:
        //   fpDevice->IFLAG1 = fTxInterruptMask0;  // clear flag
        //   fpDevice->IMASK1 |= fTxInterruptMask0; // enable mask
        fConfig.baseAddress->IR = FDCAN_IR_TC;   // clear stale TC flag
        fConfig.baseAddress->IE |= FDCAN_IE_TCE; // enable TC interrupt
    }
    // S32K: enableTransmitInterrupt() called BEFORE CODE=TRANSMIT.
    // Here: TCE enabled BEFORE TXBAR write (inside transmit()).
    return transmit(frame);
}

} // namespace bios — temporarily close
namespace bios { // reopen

uint8_t FdCanDevice::receiveISR(uint8_t const* filterBitField)
{
    FDCAN_GlobalTypeDef* fdcan = fConfig.baseAddress;
    uint32_t ramBase           = getInstanceRamBase(fdcan);
    uint8_t received           = 0U;

    // NOTE: RF0NE disable moved to CanSystem ISR trampoline (if needed).
    // Disabling here is unsafe because receiveTask() may not run if the
    // async dispatch is dedup-dropped, permanently blocking all RX.

    // NOTE: RF0L (message lost) is cleared below with RF0N and RF0F.
    // Production code could increment an overrun counter here if needed.

    // Clear RF0N BEFORE reading F0FL so late arrivals re-trigger ISR
    // (once RF0NE is re-enabled by receiveTask).
    fdcan->IR = FDCAN_IR_RF0N | FDCAN_IR_RF0F | FDCAN_IR_RF0L;

    // Snapshot fill level — drain only what's here now.
    uint8_t toDrain
        = static_cast<uint8_t>((fdcan->RXF0S & FDCAN_RXF0S_F0FL) >> FDCAN_RXF0S_F0FL_Pos);

    while (toDrain > 0U)
    {
        toDrain--;

        if (fRxCount >= RX_QUEUE_SIZE)
        {
            // Acknowledge without storing
            uint32_t getIdx = (fdcan->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos;
            fdcan->RXF0A    = getIdx;
            continue;
        }

        uint32_t getIdx = (fdcan->RXF0S & FDCAN_RXF0S_F0GI) >> FDCAN_RXF0S_F0GI_Pos;

        // Read RX FIFO element from message RAM
        // STM32G4 FDCAN uses 18-word (72-byte) element spacing in message RAM.
        static constexpr uint32_t RX_ELEMENT_SIZE = 72U; // 18 words × 4 bytes
        uint32_t const* rxBuf
            = reinterpret_cast<uint32_t const*>(ramBase + RX_FIFO0_OFFSET + (getIdx * RX_ELEMENT_SIZE));

        // Word 0: ID
        uint32_t r0 = rxBuf[0];
        uint32_t id;
        if ((r0 & (1U << 30U)) != 0U)
        {
            id = (r0 & 0x1FFFFFFFU) | 0x80000000U; // Extended
        }
        else
        {
            id = (r0 >> 18U) & 0x7FFU; // Standard
        }

        if (filterBitField != nullptr)
        {
            uint32_t byteIndex = id / 8U;
            uint32_t bitIndex  = id % 8U;
            if ((filterBitField[byteIndex] & (1U << bitIndex)) == 0U)
            {
                fdcan->RXF0A = getIdx;
                continue;
            }
        }

        // Word 1: DLC
        uint32_t r1 = rxBuf[1];
        uint8_t dlc = static_cast<uint8_t>((r1 >> 16U) & 0xFU);

        // Words 2-3: Data
        uint32_t d0 = rxBuf[2];
        uint32_t d1 = rxBuf[3];
        uint8_t data[8];
        data[0] = static_cast<uint8_t>(d0);
        data[1] = static_cast<uint8_t>(d0 >> 8U);
        data[2] = static_cast<uint8_t>(d0 >> 16U);
        data[3] = static_cast<uint8_t>(d0 >> 24U);
        data[4] = static_cast<uint8_t>(d1);
        data[5] = static_cast<uint8_t>(d1 >> 8U);
        data[6] = static_cast<uint8_t>(d1 >> 16U);
        data[7] = static_cast<uint8_t>(d1 >> 24U);

        // Store in queue
        uint8_t idx   = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
        fRxQueue[idx] = ::can::CANFrame(id, data, dlc);
        fRxCount++;
        received++;

        // Acknowledge
        fdcan->RXF0A = getIdx;
    }

    // RF0N was already cleared at entry — no need to clear again here.
    // Any frame arriving during the drain loop will re-set RF0N and
    // trigger a new ISR after we return.
    return received;
}

void FdCanDevice::transmitISR()
{
    FDCAN_GlobalTypeDef* fdcan = fConfig.baseAddress;

    // Disable TCE (match S32K disableTransmitInterrupt pattern)
    fdcan->IE &= ~FDCAN_IE_TCE;

    // Clear TC flag
    fdcan->IR = FDCAN_IR_TC;

    // Invoke callback delegate if set (match FlexCANDevice::transmitISR)
    if (fFrameSentCallback.is_valid())
    {
        fFrameSentCallback();
    }
}

bool FdCanDevice::isBusOff() const { return (fConfig.baseAddress->PSR & FDCAN_PSR_BO) != 0U; }

uint8_t FdCanDevice::getTxErrorCounter() const
{
    return static_cast<uint8_t>((fConfig.baseAddress->ECR >> FDCAN_ECR_TEC_Pos) & 0xFFU);
}

uint8_t FdCanDevice::getRxErrorCounter() const
{
    return static_cast<uint8_t>((fConfig.baseAddress->ECR >> FDCAN_ECR_REC_Pos) & 0x7FU);
}

void FdCanDevice::configureAcceptAllFilter()
{
    // Accept all frames: set global filter to accept non-matching into FIFO0
    fConfig.baseAddress->RXGFC
        = (0U << FDCAN_RXGFC_ANFS_Pos)    // Accept non-matching std into RX FIFO0
          | (0U << FDCAN_RXGFC_ANFE_Pos); // Accept non-matching ext into RX FIFO0
}

void FdCanDevice::configureFilterList(uint32_t const* idList, uint8_t count)
{
    uint32_t ramBase = getInstanceRamBase(fConfig.baseAddress);

    // Reject non-matching, configure standard ID filter elements
    fConfig.baseAddress->RXGFC = (2U << FDCAN_RXGFC_ANFS_Pos)   // Reject non-matching std
                                 | (2U << FDCAN_RXGFC_ANFE_Pos) // Reject non-matching ext
                                 | (static_cast<uint32_t>(count) << FDCAN_RXGFC_LSS_Pos);

    // Write filter elements to message RAM (standard filter area)
    uint32_t* filterRam = reinterpret_cast<uint32_t*>(ramBase + STD_FILTER_OFFSET);

    for (uint8_t i = 0U; i < count && i < 28U; i++)
    {
        // Standard filter element: classic filter (ID + mask) for exact match
        // SFT = 10 (classic), SFEC = 001 (store in RX FIFO0)
        // SFID1 = target ID, SFID2 = 0x7FF (all 11 bits must match)
        filterRam[i] = (2U << 30U)                     // SFT = 10 (classic filter)
                       | (1U << 27U)                   // SFEC = store in FIFO0
                       | ((idList[i] & 0x7FFU) << 16U) // SFID1 = filter ID
                       | 0x7FFU;                       // SFID2 = mask (exact match)
    }
}

::can::CANFrame const& FdCanDevice::getRxFrame(uint8_t index) const
{
    return fRxQueue[(fRxHead + index) % RX_QUEUE_SIZE];
}

uint8_t FdCanDevice::getRxCount() const { return fRxCount; }

void FdCanDevice::clearRxQueue()
{
    fRxHead  = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
    fRxCount = 0U;
}

void FdCanDevice::disableRxInterrupt() { fConfig.baseAddress->IE &= ~FDCAN_IE_RF0NE; }

void FdCanDevice::enableRxInterrupt()
{
    fConfig.baseAddress->IE |= FDCAN_IE_RF0NE;
}

uint32_t FdCanDevice::getHwFifoFillLevel() const
{
    return (fConfig.baseAddress->RXF0S & FDCAN_RXF0S_F0FL) >> FDCAN_RXF0S_F0FL_Pos;
}

} // namespace bios
