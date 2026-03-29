// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   BxCanDevice.cpp
 * \brief  Register-level driver implementation for the STM32 bxCAN (CAN1) peripheral.
 *
 * Design overview
 * ===============
 * This file implements a bare-metal STM32 bxCAN driver that talks directly to
 * CAN1 registers (CAN_TypeDef).  Key design decisions:
 *
 * 1. **TX path** — The three hardware TX mailboxes are scanned in order (0-1-2).
 *    The first empty mailbox is loaded with the frame and TXRQ is set.  TMEIE is
 *    enabled so that transmitISR() fires when a mailbox becomes empty again;
 *    once all three are idle TMEIE is masked to suppress spurious interrupts.
 *
 * 2. **RX path** — The bxCAN FIFO0 is a 3-deep hardware FIFO.  receiveISR()
 *    drains FIFO0 into a 32-entry circular software queue (fRxQueue).  If the
 *    software queue is full the hardware FIFO entry is released silently.
 *
 * 3. **ISR safety** — The transceiver layer uses the disableRxInterrupt() /
 *    enableRxInterrupt() pair (FMPIE0 mask bit) to create a critical section
 *    when it reads or clears the software RX queue.  This avoids disabling
 *    global interrupts while still preventing receiveISR() from modifying the
 *    queue concurrently.
 *
 * 4. **Filter banks** — Two modes are provided: accept-all (mask mode, bank 0)
 *    and identifier-list (list mode, banks 0..13, 2 IDs per bank).  Filters
 *    are programmed while the peripheral is in init mode; an additional
 *    software bit-field filter in receiveISR() gives per-ID granularity.
 */

#include <can/BxCanDevice.h>

namespace bios
{

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
#if !defined(UNIT_TEST)
    uint32_t timeout = 100000U;
    while ((fConfig.baseAddress->MSR & CAN_MSR_INAK) == 0U)
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }
#else
    // In unit tests, MSR doesn't auto-track MCR. Simulate HW behavior.
    fConfig.baseAddress->MSR |= CAN_MSR_INAK;
#endif
    return true;
}

bool BxCanDevice::leaveInitMode()
{
    fConfig.baseAddress->MCR &= ~CAN_MCR_INRQ;
#if !defined(UNIT_TEST)
    uint32_t timeout = 100000U;
    while ((fConfig.baseAddress->MSR & CAN_MSR_INAK) != 0U)
    {
        if (--timeout == 0U)
        {
            return false;
        }
    }
#else
    fConfig.baseAddress->MSR &= ~CAN_MSR_INAK;
#endif
    return true;
}

void BxCanDevice::configureBitTiming()
{
    // BTR: SJW | BS2 | BS1 | BRP (prescaler - 1)
    fConfig.baseAddress->BTR = ((fConfig.sjw - 1U) << CAN_BTR_SJW_Pos)
                               | ((fConfig.bs2 - 1U) << CAN_BTR_TS2_Pos)
                               | ((fConfig.bs1 - 1U) << CAN_BTR_TS1_Pos) | (fConfig.prescaler - 1U);
}

void BxCanDevice::init()
{
    enablePeripheralClock();
    configureGpio();
    enterInitMode();

    // Exit sleep mode
    fConfig.baseAddress->MCR &= ~CAN_MCR_SLEEP;

    // Configure: auto bus-off management, auto retransmission, TX FIFO priority
    fConfig.baseAddress->MCR |= CAN_MCR_ABOM | CAN_MCR_TXFP;

    configureBitTiming();
    configureAcceptAllFilter();

    fInitialized = true;
}

void BxCanDevice::start()
{
    if (!fInitialized)
    {
        return;
    }
    leaveInitMode();

    // Drain any frames that arrived while in init mode
#if !defined(UNIT_TEST)
    while ((fConfig.baseAddress->RF0R & CAN_RF0R_FMP0) != 0U)
    {
        fConfig.baseAddress->RF0R |= CAN_RF0R_RFOM0;
    }
#endif
    // Clear overrun flag
    fConfig.baseAddress->RF0R |= CAN_RF0R_FOVR0;

    fConfig.baseAddress->IER |= CAN_IER_FMPIE0;
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

    // Set ID (standard 11-bit)
    uint32_t id = frame.getId();
    if ((id & 0x80000000U) != 0U)
    {
        // Extended ID
        can->sTxMailBox[mailbox].TIR = ((id & 0x1FFFFFFFU) << CAN_TI0R_EXID_Pos) | CAN_TI0R_IDE;
    }
    else
    {
        // Standard ID
        can->sTxMailBox[mailbox].TIR = ((id & 0x7FFU) << CAN_TI0R_STID_Pos);
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
    uint8_t toDrain = static_cast<uint8_t>(can->RF0R & CAN_RF0R_FMP0);
    while (toDrain > 0U)
    {
        toDrain--;
        if (fRxCount >= RX_QUEUE_SIZE)
        {
            // Queue full — release FIFO entry without storing
            can->RF0R |= CAN_RF0R_RFOM0;
            continue;
        }

        // Read ID
        uint32_t rir = can->sFIFOMailBox[0].RIR;
        uint32_t id;
        if ((rir & CAN_RI0R_IDE) != 0U)
        {
            id = ((rir >> CAN_RI0R_EXID_Pos) & 0x1FFFFFFFU) | 0x80000000U;
        }
        else
        {
            id = (rir >> CAN_RI0R_STID_Pos) & 0x7FFU;
        }

        // Filter check using BitFieldFilter byte array (if provided)
        if (filterBitField != nullptr)
        {
            uint32_t byteIndex = id / 8U;
            uint32_t bitIndex  = id % 8U;
            if ((filterBitField[byteIndex] & (1U << bitIndex)) == 0U)
            {
                can->RF0R |= CAN_RF0R_RFOM0;
                continue;
            }
        }

        // Read DLC and data
        uint8_t dlc   = static_cast<uint8_t>(can->sFIFOMailBox[0].RDTR & 0xFU);
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

        // Store in queue
        uint8_t idx   = (fRxHead + fRxCount) % RX_QUEUE_SIZE;
        fRxQueue[idx] = ::can::CANFrame(id, data, dlc);
        fRxCount++;
        received++;

        // Release FIFO entry
        can->RF0R |= CAN_RF0R_RFOM0;
    }

    return received;
}

void BxCanDevice::transmitISR()
{
    // Clear TX request completed flags
    fConfig.baseAddress->TSR |= CAN_TSR_RQCP0 | CAN_TSR_RQCP1 | CAN_TSR_RQCP2;

    // Disable TMEIE if all mailboxes are now empty (prevents spurious interrupts)
    if ((fConfig.baseAddress->TSR & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2))
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

void BxCanDevice::configureFilterList(uint32_t const* idList, uint8_t count)
{
    CAN_TypeDef* can = fConfig.baseAddress;
    can->FMR |= CAN_FMR_FINIT;

    // Use filter banks 0..N/2 in 32-bit list mode (2 IDs per bank)
    uint8_t bankIdx = 0U;
    for (uint8_t i = 0U; i < count && bankIdx < 14U; i += 2U, bankIdx++)
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
