/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <can/BxCanDevice.h>

namespace bios
{

// Bounded busy-wait for the INAK handshake. A plain cycle count is used
// instead of bsp::isEqualAfterTimeout so this register-level driver stays
// free of the bsp timer dependency - init() runs during early board
// bring-up, before the system timer is guaranteed to be running.
static constexpr uint32_t INIT_TIMEOUT_CYCLES = 100000U;

BxCanDevice::BxCanDevice(Config const& config)
: fConfig(config), fRxQueue{}, fRxHead(0U), fRxCount(0U), fInitialized(false)
{}

void BxCanDevice::enablePeripheralClock()
{
    // Enable CAN1 clock on APB1
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    // Small delay for clock stabilization
    uint32_t volatile dummy = RCC->APB1ENR;
    (void)dummy;
}

void BxCanDevice::configureGpio()
{
    GPIO_TypeDef* txPort = fConfig.txGpioPort;
    GPIO_TypeDef* rxPort = fConfig.rxGpioPort;

    // TX pin: AF mode, push-pull, high speed
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

    // RX pin: AF mode, pull-up
    rxPort->MODER &= ~(3U << (fConfig.rxPin * 2U));
    rxPort->MODER |= (2U << (fConfig.rxPin * 2U));
    rxPort->PUPDR &= ~(3U << (fConfig.rxPin * 2U));
    rxPort->PUPDR |= (1U << (fConfig.rxPin * 2U)); // Pull-up
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

bool BxCanDevice::enterInitMode()
{
    fConfig.baseAddress->MCR |= CAN_MCR_INRQ;
    uint32_t timeout = INIT_TIMEOUT_CYCLES;
    while ((fConfig.baseAddress->MSR & CAN_MSR_INAK) == 0U)
    {
        --timeout;
        if (timeout == 0U)
        {
            return false;
        }
    }
    return true;
}

bool BxCanDevice::leaveInitMode()
{
    fConfig.baseAddress->MCR &= ~CAN_MCR_INRQ;
    uint32_t timeout = INIT_TIMEOUT_CYCLES;
    while ((fConfig.baseAddress->MSR & CAN_MSR_INAK) != 0U)
    {
        --timeout;
        if (timeout == 0U)
        {
            return false;
        }
    }
    return true;
}

void BxCanDevice::configureBitTiming()
{
    // BTR: SJW | BS2 | BS1 | BRP (prescaler - 1)
    fConfig.baseAddress->BTR = ((fConfig.sjw - 1U) << CAN_BTR_SJW_Pos)
                               | ((fConfig.bs2 - 1U) << CAN_BTR_TS2_Pos)
                               | ((fConfig.bs1 - 1U) << CAN_BTR_TS1_Pos) | (fConfig.prescaler - 1U);
}

bool BxCanDevice::init()
{
    enablePeripheralClock();
    configureGpio();

    if (!enterInitMode())
    {
        return false;
    }

    // Exit sleep mode
    fConfig.baseAddress->MCR &= ~CAN_MCR_SLEEP;

    // Configure: auto bus-off management, auto retransmission, TX FIFO priority
    fConfig.baseAddress->MCR |= CAN_MCR_ABOM | CAN_MCR_TXFP;

    configureBitTiming();
    configureAcceptAllFilter();

    fInitialized = true;
    return true;
}

bool BxCanDevice::start()
{
    if (!fInitialized)
    {
        return false;
    }

    if (!leaveInitMode())
    {
        return false;
    }

    // RF0R contains rc_w1 flags (FULL0, FOVR0): always write it directly.
    // A read-modify-write would write back any flag that happens to be set
    // and silently clear it.
    // Snapshot the fill level (0-3) BEFORE the first write, then clear the
    // overrun flag and release exactly that many stale frames - same
    // bounded-drain pattern as receiveISR.
    uint8_t staleFrames       = static_cast<uint8_t>(fConfig.baseAddress->RF0R & CAN_RF0R_FMP0);
    fConfig.baseAddress->RF0R = CAN_RF0R_FOVR0;
    while (staleFrames > 0U)
    {
        --staleFrames;
        fConfig.baseAddress->RF0R = CAN_RF0R_RFOM0;
    }

    fConfig.baseAddress->IER |= CAN_IER_FMPIE0;
    return true;
}

void BxCanDevice::stop()
{
    fConfig.baseAddress->IER &= ~(CAN_IER_FMPIE0 | CAN_IER_TMEIE);
    enterInitMode();
}

bool BxCanDevice::transmit(::can::CANFrame const& frame)
{
    CAN_TypeDef* can = fConfig.baseAddress;

    // Find an empty TX mailbox
    uint8_t mailbox = 0xFFU;
    if ((can->TSR & CAN_TSR_TME0) != 0U)
    {
        mailbox = 0U;
    }
    else if ((can->TSR & CAN_TSR_TME1) != 0U)
    {
        mailbox = 1U;
    }
    else if ((can->TSR & CAN_TSR_TME2) != 0U)
    {
        mailbox = 2U;
    }
    else
    {
        return false; // All mailboxes full
    }

    // Set ID (see cpp2can CanId for the extended-qualifier encoding)
    uint32_t const id = frame.getId();
    if (::can::CanId::isExtended(id))
    {
        can->sTxMailBox[mailbox].TIR
            = ((id & ::can::CANFrame::MAX_FRAME_ID_EXTENDED) << CAN_TI0R_EXID_Pos) | CAN_TI0R_IDE;
    }
    else
    {
        can->sTxMailBox[mailbox].TIR = ((id & ::can::CANFrame::MAX_FRAME_ID) << CAN_TI0R_STID_Pos);
    }

    // Set DLC
    uint8_t dlc                   = frame.getPayloadLength();
    can->sTxMailBox[mailbox].TDTR = (dlc & 0xFU);

    // Set data bytes
    uint8_t const* data = frame.getPayload();
    can->sTxMailBox[mailbox].TDLR
        = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8U)
          | (static_cast<uint32_t>(data[2]) << 16U) | (static_cast<uint32_t>(data[3]) << 24U);
    can->sTxMailBox[mailbox].TDHR
        = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8U)
          | (static_cast<uint32_t>(data[6]) << 16U) | (static_cast<uint32_t>(data[7]) << 24U);

    // Request transmission
    can->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

    // Enable TX mailbox empty interrupt now that we have pending TX
    can->IER |= CAN_IER_TMEIE;

    return true;
}

uint8_t BxCanDevice::receiveISR(uint8_t const* filterBitField)
{
    CAN_TypeDef* can = fConfig.baseAddress;
    uint8_t received = 0U;

    // Snapshot fill level (same pattern as FDCAN receiveISR).
    // Prevents infinite loop if HW latches new frames during drain.
    // All RF0R writes below are direct assignments: RF0R holds rc_w1 flags
    // (FULL0, FOVR0) that a read-modify-write would clear by writing back.
    uint8_t toDrain = static_cast<uint8_t>(can->RF0R & CAN_RF0R_FMP0);
    while (toDrain > 0U)
    {
        toDrain--;
        if (fRxCount >= RX_QUEUE_SIZE)
        {
            // Queue full - release FIFO entry without storing
            can->RF0R = CAN_RF0R_RFOM0;
            continue;
        }

        // Read ID
        uint32_t rir = can->sFIFOMailBox[0].RIR;
        uint32_t id;
        if ((rir & CAN_RI0R_IDE) != 0U)
        {
            id = ::can::CanId::extended(
                (rir >> CAN_RI0R_EXID_Pos) & ::can::CANFrame::MAX_FRAME_ID_EXTENDED);
        }
        else
        {
            id = (rir >> CAN_RI0R_STID_Pos) & ::can::CANFrame::MAX_FRAME_ID;
        }

        // Filter check using BitFieldFilter byte array (if provided). The array
        // is indexed by CAN ID, so it applies to standard (11-bit) IDs only;
        // extended frames bypass it (a 29-bit index would be out of range).
        if ((filterBitField != nullptr) && ::can::CanId::isBase(id))
        {
            uint32_t byteIndex = id / 8U;
            uint32_t bitIndex  = id % 8U;
            if ((filterBitField[byteIndex] & (1U << bitIndex)) == 0U)
            {
                can->RF0R = CAN_RF0R_RFOM0;
                continue;
            }
        }

        // Read DLC and data. Classic CAN allows DLC 9-15 on the wire (all mean
        // 8 data bytes); clamp to the payload buffer size to avoid overrun.
        uint8_t dlc = static_cast<uint8_t>(can->sFIFOMailBox[0].RDTR & 0xFU);
        if (dlc > ::can::CANFrame::MAX_FRAME_LENGTH)
        {
            dlc = ::can::CANFrame::MAX_FRAME_LENGTH;
        }
        uint32_t rdlr = can->sFIFOMailBox[0].RDLR;
        uint32_t rdhr = can->sFIFOMailBox[0].RDHR;

        uint8_t data[8];
        data[0] = static_cast<uint8_t>(rdlr);
        data[1] = static_cast<uint8_t>(rdlr >> 8U);
        data[2] = static_cast<uint8_t>(rdlr >> 16U);
        data[3] = static_cast<uint8_t>(rdlr >> 24U);
        data[4] = static_cast<uint8_t>(rdhr);
        data[5] = static_cast<uint8_t>(rdhr >> 8U);
        data[6] = static_cast<uint8_t>(rdhr >> 16U);
        data[7] = static_cast<uint8_t>(rdhr >> 24U);

        // Store in queue. A hand-rolled circular buffer is used instead of an
        // etl container because the transceiver drains it in a batch: the ISR
        // appends while the task reads getRxFrame(0..N-1) and then releases
        // all N frames at once via clearRxQueue() (head advance + count reset).
        uint8_t idx   = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
        fRxQueue[idx] = ::can::CANFrame(id, data, dlc);
        fRxCount++;
        received++;

        // Release FIFO entry
        can->RF0R = CAN_RF0R_RFOM0;
    }

    return received;
}

void BxCanDevice::transmitISR()
{
    // Snapshot TSR, then clear the request-completed flags with a direct
    // write. TSR is rc_w1 (RQCP, TXOK, ALST, TERR): a read-modify-write
    // would write back and clear whichever status flags were set.
    uint32_t const tsr       = fConfig.baseAddress->TSR;
    fConfig.baseAddress->TSR = CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2;

    // Disable TMEIE if all mailboxes are now empty (prevents spurious interrupts)
    if ((tsr & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2))
        == (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2))
    {
        fConfig.baseAddress->IER &= ~CAN_IER_TMEIE;
    }
}

bool BxCanDevice::isBusOff() const { return (fConfig.baseAddress->ESR & CAN_ESR_BOFF) != 0U; }

uint8_t BxCanDevice::getTxErrorCounter() const
{
    return static_cast<uint8_t>((fConfig.baseAddress->ESR >> CAN_ESR_TEC_Pos) & 0xFFU);
}

uint8_t BxCanDevice::getRxErrorCounter() const
{
    return static_cast<uint8_t>((fConfig.baseAddress->ESR >> CAN_ESR_REC_Pos) & 0xFFU);
}

void BxCanDevice::configureAcceptAllFilter()
{
    // Filter bank 0: accept all standard + extended frames into FIFO0
    CAN_TypeDef* can = fConfig.baseAddress;
    can->FMR |= CAN_FMR_FINIT;        // Enter filter init mode
    can->FA1R &= ~(1U << 0U);         // Deactivate filter 0
    can->sFilterRegister[0].FR1 = 0U; // ID = 0 (don't care)
    can->sFilterRegister[0].FR2 = 0U; // Mask = 0 (accept all)
    can->FS1R |= (1U << 0U);          // 32-bit scale
    can->FM1R &= ~(1U << 0U);         // Mask mode (not list)
    can->FFA1R &= ~(1U << 0U);        // Assign to FIFO0
    can->FA1R |= (1U << 0U);          // Activate filter 0
    can->FMR &= ~CAN_FMR_FINIT;       // Leave filter init mode
}

void BxCanDevice::configureFilterList(::etl::span<uint32_t const> idList)
{
    CAN_TypeDef* can = fConfig.baseAddress;
    can->FMR |= CAN_FMR_FINIT;

    // Use filter banks 0..N/2 in 32-bit list mode (2 IDs per bank)
    size_t const count = idList.size();
    size_t bankIdx     = 0U;
    for (size_t i = 0U; i < count && bankIdx < FILTER_BANK_COUNT; i += 2U, bankIdx++)
    {
        can->FA1R &= ~(1U << bankIdx);
        can->FS1R |= (1U << bankIdx); // 32-bit scale
        can->FM1R |= (1U << bankIdx); // List mode

        // Standard IDs shifted to STID position
        can->sFilterRegister[bankIdx].FR1 = (idList[i] << CAN_RI0R_STID_Pos);
        if ((i + 1U) < count)
        {
            can->sFilterRegister[bankIdx].FR2 = (idList[i + 1U] << CAN_RI0R_STID_Pos);
        }
        else
        {
            can->sFilterRegister[bankIdx].FR2 = (idList[i] << CAN_RI0R_STID_Pos);
        }

        can->FFA1R &= ~(1U << bankIdx); // FIFO0
        can->FA1R |= (1U << bankIdx);   // Activate
    }

    can->FMR &= ~CAN_FMR_FINIT;
}

::can::CANFrame const& BxCanDevice::getRxFrame(uint8_t index) const
{
    return fRxQueue[(fRxHead + index) % RX_QUEUE_SIZE];
}

uint8_t BxCanDevice::getRxCount() const { return fRxCount; }

void BxCanDevice::clearRxQueue()
{
    fRxHead  = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
    fRxCount = 0U;
}

void BxCanDevice::disableRxInterrupt() { fConfig.baseAddress->IER &= ~CAN_IER_FMPIE0; }

void BxCanDevice::enableRxInterrupt() { fConfig.baseAddress->IER |= CAN_IER_FMPIE0; }

} // namespace bios
