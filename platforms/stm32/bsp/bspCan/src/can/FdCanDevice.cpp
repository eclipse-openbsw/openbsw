/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <can/FdCanDevice.h>

namespace bios
{

// STM32G4 FDCAN message RAM layout (fixed, per instance):
// Each FDCAN instance has 212 words (848 bytes) of message RAM.
// FDCAN1 starts at SRAMCAN_BASE + 0x000
// FDCAN2 starts at SRAMCAN_BASE + 0x350
// FDCAN3 starts at SRAMCAN_BASE + 0x6A0
//
// Layout within each instance (offsets from instance base; RX/TX elements
// are spaced 18 words / 72 bytes apart regardless of configured data size):
//   Standard ID filters: 28 elements x  1 word  = 28 words  (0x000 - 0x06F)
//   Extended ID filters:  8 elements x  2 words = 16 words  (0x070 - 0x0AF)
//   RX FIFO0:             3 elements x 18 words = 54 words  (0x0B0 - 0x187)
//   RX FIFO1:             3 elements x 18 words = 54 words  (0x188 - 0x25F)
//   TX Event FIFO:        3 elements x  2 words =  6 words  (0x260 - 0x277)
//   TX Buffers:           3 elements x 18 words = 54 words  (0x278 - 0x34F)

// Message RAM stride between instances (212 words per instance, see above)
static constexpr uint32_t FDCAN2_RAM_OFFSET = 0x350U;
static constexpr uint32_t FDCAN3_RAM_OFFSET = 0x6A0U;

// uintptr_t so host unit tests with 64-bit fake RAM addresses work; on the
// 32-bit target this is identical to uint32_t.
static uintptr_t getInstanceRamBase(FDCAN_GlobalTypeDef* fdcan)
{
    if (fdcan == FDCAN1)
    {
        return SRAMCAN_BASE;
    }
#if defined(FDCAN2)
    if (fdcan == FDCAN2)
    {
        return SRAMCAN_BASE + FDCAN2_RAM_OFFSET;
    }
#endif
#if defined(FDCAN3)
    if (fdcan == FDCAN3)
    {
        return SRAMCAN_BASE + FDCAN3_RAM_OFFSET;
    }
#endif
    return SRAMCAN_BASE;
}

static constexpr uint32_t STD_FILTER_OFFSET = 0x000U;
static constexpr uint32_t RX_FIFO0_OFFSET   = 0x0B0U;
// STM32G4 message RAM: TX buffers at 0x278 (not 0x128 as standard M_CAN)
static constexpr uint32_t TX_BUFFER_OFFSET  = 0x278U;

// RX/TX element stride: 18 words (72 bytes) regardless of the configured
// data field size - section boundaries are fixed at max element size.
static constexpr uint32_t ELEMENT_SIZE = 72U;

// RX/TX buffer element word 0/1 encoding (R0/R1 resp. T0/T1)
static constexpr uint32_t ELEMENT_XTD       = 1U << 30U; // extended-ID flag
static constexpr uint32_t ELEMENT_STDID_POS = 18U;       // standard ID in bits [28:18]
static constexpr uint32_t ELEMENT_DLC_POS   = 16U;       // DLC in bits [19:16]

// TXBTIE mask covering all 3 TX buffers
static constexpr uint32_t TXBTIE_ALL_BUFFERS = 0x7U;

// Bounded busy-wait for the CCCR.INIT handshake. A plain cycle count is used
// instead of bsp::isEqualAfterTimeout so this register-level driver stays
// free of the bsp timer dependency - init() runs during early board
// bring-up, before the system timer is guaranteed to be running.
static constexpr uint32_t INIT_TIMEOUT_CYCLES = 100000U;

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

    // Select FDCAN kernel clock = PCLK1 (FDCANSEL=10 in CCIPR[25:24])
    // Default after reset is HSE (00), which may not be enabled.
    RCC->CCIPR = (RCC->CCIPR & ~RCC_CCIPR_FDCANSEL) | RCC_CCIPR_FDCANSEL_1;
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

bool FdCanDevice::enterInitMode()
{
    fConfig.baseAddress->CCCR |= FDCAN_CCCR_INIT;
    uint32_t timeout = INIT_TIMEOUT_CYCLES;
    while ((fConfig.baseAddress->CCCR & FDCAN_CCCR_INIT) == 0U)
    {
        --timeout;
        if (timeout == 0U)
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
        --timeout;
        if (timeout == 0U)
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
    // STM32G4 has fixed message RAM layout - no configuration registers.
    // RXGFC configures global filter behavior and list sizes only.
    fConfig.baseAddress->RXGFC = (0U << FDCAN_RXGFC_LSS_Pos) | (0U << FDCAN_RXGFC_LSE_Pos);

    // TX buffer: use FIFO/queue mode
    fConfig.baseAddress->TXBC = 0U; // FIFO mode (TFQM=0)
}

bool FdCanDevice::init()
{
    enablePeripheralClock();
    configureGpio();

    if (!enterInitMode())
    {
        return false;
    }

    // Classic CAN mode: clear FD operation (FDOE) and bit-rate switching
    // (BRSE). Automatic retransmission stays enabled - DAR is left at its reset
    // default of 0, matching normal CAN behaviour.
    fConfig.baseAddress->CCCR &= ~FDCAN_CCCR_FDOE;
    fConfig.baseAddress->CCCR &= ~FDCAN_CCCR_BRSE;

    configureBitTiming();
    configureMessageRam();
    configureAcceptAllFilter();

    fInitialized = true;
    return true;
}

bool FdCanDevice::start()
{
    if (!fInitialized)
    {
        return false;
    }

    if (!leaveInitMode())
    {
        return false;
    }

    // IE/ILS/ILE/TXBTIE are not CCE-protected, so they can be written after
    // leaving init mode. Writing them only after a successful leaveInitMode
    // keeps all interrupts masked on a failed start - same contract as
    // BxCanDevice::start().
    // Enable RX interrupt only at startup. TCE is managed per-TX by
    // transmit(frame, true) and disabled by transmitISR() - matching S32K's
    // selective per-buffer interrupt enable/disable pattern.
    fConfig.baseAddress->IE     = FDCAN_IE_RF0NE;
    fConfig.baseAddress->ILS    = 0U;
    fConfig.baseAddress->ILE    = FDCAN_ILE_EINT0;
    // TXBTIE required for TC interrupt generation per RM0440
    fConfig.baseAddress->TXBTIE = TXBTIE_ALL_BUFFERS;
    return true;
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
    uintptr_t ramBase = getInstanceRamBase(fdcan);
    uint32_t* txBuf
        = reinterpret_cast<uint32_t*>(ramBase + TX_BUFFER_OFFSET + (putIdx * ELEMENT_SIZE));

    // Word 0 (T0): ID (see cpp2can CanId for the extended-qualifier encoding)
    uint32_t const id = frame.getId();
    if (::can::CanId::isExtended(id))
    {
        txBuf[0] = (id & ::can::CANFrame::MAX_FRAME_ID_EXTENDED) | ELEMENT_XTD;
    }
    else
    {
        txBuf[0] = ((id & ::can::CANFrame::MAX_FRAME_ID) << ELEMENT_STDID_POS);
    }

    // Word 1 (T1): DLC, no FD, no BRS
    uint8_t dlc = frame.getPayloadLength();
    txBuf[1]    = (static_cast<uint32_t>(dlc) << ELEMENT_DLC_POS);

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
        // Match FlexCANDevice::enableTransmitInterrupt(): clear the stale
        // completion flag first, then enable the interrupt mask.
        fConfig.baseAddress->IR = FDCAN_IR_TC;   // clear stale TC flag
        fConfig.baseAddress->IE |= FDCAN_IE_TCE; // enable TC interrupt
    }
    // S32K calls enableTransmitInterrupt() BEFORE writing CODE=TRANSMIT.
    // Here: TCE enabled BEFORE TXBAR write (inside transmit()).
    return transmit(frame);
}

uint8_t FdCanDevice::receiveISR(uint8_t const* filterBitField)
{
    FDCAN_GlobalTypeDef* fdcan = fConfig.baseAddress;
    uintptr_t ramBase          = getInstanceRamBase(fdcan);
    uint8_t received           = 0U;

    // NOTE: RF0NE disable moved to CanSystem ISR trampoline (if needed).
    // Disabling here is unsafe because receiveTask() may not run if the
    // async dispatch is dedup-dropped, permanently blocking all RX.

    // NOTE: RF0L (message lost) is cleared below with RF0N and RF0F.
    // Production code could increment an overrun counter here if needed.

    // Clear RF0N BEFORE reading F0FL so late arrivals re-trigger ISR
    // (once RF0NE is re-enabled by receiveTask).
    fdcan->IR = FDCAN_IR_RF0N | FDCAN_IR_RF0F | FDCAN_IR_RF0L;

    // Snapshot fill level - drain only what's here now.
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
        uint32_t const* rxBuf = reinterpret_cast<uint32_t const*>(
            ramBase + RX_FIFO0_OFFSET + (getIdx * ELEMENT_SIZE));

        // Word 0 (R0): ID
        uint32_t r0 = rxBuf[0];
        uint32_t id;
        if ((r0 & ELEMENT_XTD) != 0U)
        {
            id = ::can::CanId::extended(r0 & ::can::CANFrame::MAX_FRAME_ID_EXTENDED);
        }
        else
        {
            id = (r0 >> ELEMENT_STDID_POS) & ::can::CANFrame::MAX_FRAME_ID;
        }

        // Applies to standard (11-bit) IDs only; extended frames bypass this
        // bit-field filter (a 29-bit index would be out of range).
        if ((filterBitField != nullptr) && ::can::CanId::isBase(id))
        {
            uint32_t byteIndex = id / 8U;
            uint32_t bitIndex  = id % 8U;
            if ((filterBitField[byteIndex] & (1U << bitIndex)) == 0U)
            {
                fdcan->RXF0A = getIdx;
                continue;
            }
        }

        // Word 1 (R1): DLC. Classic CAN allows DLC 9-15 (all mean 8 bytes);
        // clamp to the payload buffer size to avoid overrun.
        uint32_t r1 = rxBuf[1];
        uint8_t dlc = static_cast<uint8_t>((r1 >> ELEMENT_DLC_POS) & 0xFU);
        if (dlc > ::can::CANFrame::MAX_FRAME_LENGTH)
        {
            dlc = ::can::CANFrame::MAX_FRAME_LENGTH;
        }

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

        // Store in queue. A hand-rolled circular buffer is used instead of an
        // etl container because the transceiver drains it in a batch: the ISR
        // appends while the task reads getRxFrame(0..N-1) and then releases
        // all N frames at once via clearRxQueue() (head advance + count reset).
        uint8_t idx   = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
        fRxQueue[idx] = ::can::CANFrame(id, data, dlc);
        fRxCount++;
        received++;

        // Acknowledge
        fdcan->RXF0A = getIdx;
    }

    // RF0N was already cleared at entry - no need to clear again here.
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

void FdCanDevice::configureFilterList(::etl::span<uint32_t const> idList)
{
    uintptr_t ramBase = getInstanceRamBase(fConfig.baseAddress);

    // The message RAM holds STD_FILTER_COUNT standard filter elements;
    // surplus entries are ignored.
    size_t const count = (idList.size() < STD_FILTER_COUNT) ? idList.size() : STD_FILTER_COUNT;

    // Reject non-matching, configure standard ID filter elements
    fConfig.baseAddress->RXGFC = (2U << FDCAN_RXGFC_ANFS_Pos)   // Reject non-matching std
                                 | (2U << FDCAN_RXGFC_ANFE_Pos) // Reject non-matching ext
                                 | (static_cast<uint32_t>(count) << FDCAN_RXGFC_LSS_Pos);

    // Write filter elements to message RAM (standard filter area)
    uint32_t* filterRam = reinterpret_cast<uint32_t*>(ramBase + STD_FILTER_OFFSET);

    for (size_t i = 0U; i < count; i++)
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

void FdCanDevice::enableRxInterrupt() { fConfig.baseAddress->IE |= FDCAN_IE_RF0NE; }

} // namespace bios
