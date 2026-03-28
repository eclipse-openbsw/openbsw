// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file CanSystem.cpp
 * \brief CAN system implementation for the NUCLEO-G474RE platform (FDCAN1).
 *
 * Contains the CanSystem lifecycle methods, FDCAN1 peripheral configuration
 * (500 kbit/s @ 170 MHz), and the extern "C" ISR trampolines that bridge
 * hardware interrupts into the openBSW async framework.
 */

#include "systems/CanSystem.h"

#include "async/Config.h"
#include "async/Hook.h"
#include "busid/BusId.h"
#include "mcu/mcu.h"
#include <cstdio>

namespace
{
// FDCAN1 configuration for STM32G474RE
// FDCAN1: PA11 (RX, AF9), PA12 (TX, AF9), 500 kbit/s @ 170 MHz
// NBTP: NSJW=1TQ, NBS1=14TQ, NBS2=2TQ, NBRP=20 → 500 kbit/s
::bios::FdCanDevice::Config const fdcan1Config = {
    FDCAN1, // peripheral
    20U,    // prescaler (170 MHz / 20 = 8.5 MHz → 17 TQ @ 500k)
    13U,    // tseg1 (14 TQ - 1)
    1U,     // tseg2 (2 TQ - 1)
    0U,     // sjw (1 TQ - 1)
    GPIOA,  // rxPort
    11U,    // rxPin
    9U,     // rxAf (AF9 = FDCAN1)
    GPIOA,  // txPort
    12U,    // txPin
    9U,     // txAf (AF9 = FDCAN1)
};
} // namespace

namespace systems
{

CanSystem::CanSystem(::async::ContextType context)
: ::lifecycle::SingleContextLifecycleComponent(context)
, ::etl::singleton_base<CanSystem>(*this)
, _context(context)
, _transceiver0(context, ::busid::CAN_0, fdcan1Config)
, _canRxRunnable(*this)
, _canTxRunnable(*this)
{}

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    _canRxRunnable.setEnabled(true);

    (void)_transceiver0.init();
    (void)_transceiver0.open();

    // Accept-all filter (set during init). No HW filter override.

    // Enable FDCAN IRQs after transceiver is open (not in setupApplicationsIsr,
    // because the async framework must be ready before ISRs fire)
    // Priority must be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
    // for FreeRTOS API safety, and <= 5 so BASEPRI (0x50) doesn't mask it.
    // Priority 8 (0x80) is masked by BASEPRI during FreeRTOS scheduling.
    // Force IE register — FdCanDevice::start() should set this, but
    // verify it persists after leaving init mode.
    FDCAN1->IE  = FDCAN_IE_RF0NE | FDCAN_IE_TCE;
    FDCAN1->ILS = 0U;
    FDCAN1->ILE = FDCAN_ILE_EINT0;

    SYS_SetPriority(FDCAN1_IT0_IRQn, 5);
    SYS_SetPriority(FDCAN1_IT1_IRQn, 5);
    NVIC_ClearPendingIRQ(FDCAN1_IT0_IRQn);
    NVIC_ClearPendingIRQ(FDCAN1_IT1_IRQn);
    SYS_EnableIRQ(FDCAN1_IT0_IRQn);
    SYS_EnableIRQ(FDCAN1_IT1_IRQn);

    // Schedule periodic TX callback poll (every 5ms) to check if DoCAN TX
    // response needs confirmation. This avoids async dispatch dedup issues
    // that prevent the ISR-based callback from reaching the DoCAN layer.
    // TX callback poll at 50ms — fast enough for DoCAN 1000ms timeout,
    // slow enough to not starve TASK_CAN (1ms poll killed ALL RX).
    ::async::scheduleAtFixedRate(
        _context, _canTxRunnable, _canTxTimeout, 50U, ::async::TimeUnit::MILLISECONDS);

    transitionDone();
}

void CanSystem::shutdown()
{
    (void)_transceiver0.close();
    _transceiver0.shutdown();

    _canRxRunnable.setEnabled(false);

    transitionDone();
}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    if (busId == ::busid::CAN_0)
    {
        return &_transceiver0;
    }
    return nullptr;
}

void CanSystem::dispatchRxTask() { ::async::execute(_context, _canRxRunnable); }

void CanSystem::dispatchTxTask() { ::async::execute(_context, _canTxRunnable); }

} // close namespace systems temporarily

extern volatile uint32_t g_rxIsrCount;
extern volatile uint32_t g_rxIsrCalls;
extern volatile uint32_t g_rxTaskCount;
extern volatile uint32_t g_rxDispatch;
extern volatile uint32_t g_txIsrCount;
static uint32_t s_pollCounter = 0U;

namespace systems { // reopen

void CanSystem::CanTxRunnable::execute()
{
    ::bios::FdCanTransceiver::pollTxCallback(::busid::CAN_0);

    // Dump pipeline counters every 5 seconds via raw UART
    if (++s_pollCounter >= 5000U)
    {
        s_pollCounter = 0U;
        // Write directly to USART2 TDR (blocking, ~8ms for 100 chars at 115200)
        char buf[100];
        int n = snprintf(buf, sizeof(buf),
            "\r\nPIPE:%lu,%lu,%lu,%lu,%lu\r\n",
            g_rxIsrCalls, g_rxIsrCount, g_rxDispatch, g_rxTaskCount, g_txIsrCount);
        uint32_t volatile* usart2 = reinterpret_cast<uint32_t volatile*>(0x40004400U);
        for (int i = 0; i < n; i++)
        {
            while ((usart2[0x1C / 4] & (1U << 7)) == 0U) {} // Wait TXE
            usart2[0x28 / 4] = static_cast<uint32_t>(buf[i]);
        }
    }
}

CanSystem::CanRxRunnable::CanRxRunnable(CanSystem& parent) : _parent(parent), _enabled(false) {}

static uint32_t s_rxRunCount = 0U;

void CanSystem::CanRxRunnable::execute()
{
    if (_enabled)
    {
        _parent._transceiver0.receiveTask();
    }
    // Dump counters every 100 RX runnable calls (~every 100 CAN frames)
    if (++s_rxRunCount % 10U == 0U)
    {
        char buf[80];
        int n = snprintf(buf, sizeof(buf), "\r\nPIPE:%lu,%lu,%lu,%lu,%lu\r\n",
            g_rxIsrCalls, g_rxIsrCount, g_rxDispatch, g_rxTaskCount, g_txIsrCount);
        uint32_t volatile* usart2 = reinterpret_cast<uint32_t volatile*>(0x40004400U);
        for (int i = 0; i < n; i++)
        {
            while ((usart2[0x1C / 4] & (1U << 7)) == 0U) {}
            usart2[0x28 / 4] = static_cast<uint32_t>(buf[i]);
        }
    }
}

} // namespace systems

// Pipeline counters for diagnosing frame loss (volatile, readable via debugger)
volatile uint32_t g_rxIsrCount    = 0U; // Frames drained from HW FIFO by receiveISR
volatile uint32_t g_rxIsrCalls    = 0U; // Times RF0N ISR path entered
volatile uint32_t g_rxTaskCount   = 0U; // Frames processed by receiveTask
volatile uint32_t g_rxDispatch    = 0U; // Times dispatchRxTask called
volatile uint32_t g_txIsrCount    = 0U; // Times TC ISR path entered

extern "C"
{
/**
 * CAN receive interrupt service routine trampoline for FDCAN1.
 *
 * Disables the RX FIFO interrupt (RF0NE) to prevent re-entry while the 3-deep
 * hardware FIFO refills, reads pending frames under an async lock, and dispatches
 * the CanRxRunnable to drain the software queue. The RX interrupt is re-enabled
 * in receiveTask() after processing completes.
 *
 * \note ISR context — must not call blocking APIs or allocate memory.
 * \note RF0NE is disabled at entry and re-enabled either here (no frames) or
 *       in receiveTask() (frames dispatched) to prevent ISR storm.
 */
/**
 * Combined RX + TX ISR — all FDCAN1 interrupts routed to IT0.
 * Checks IR register to determine if RX, TX, or both occurred.
 */
void call_can_isr_RX()
{
    ::asyncEnterIsrGroup(ISR_GROUP_CAN);

    // Check for RX (RF0NE)
    uint32_t volatile* fdcan = reinterpret_cast<uint32_t volatile*>(0x40006400U);
    uint32_t ir              = fdcan[0x50 / 4]; // IR register

    if ((ir & 0x01U) != 0U) // RF0N — RX FIFO 0 new message
    {
        g_rxIsrCalls++;
        uint8_t framesReceived;
        {
            ::async::LockType const lock;
            framesReceived = ::bios::FdCanTransceiver::receiveInterrupt(::busid::CAN_0);
        }
        g_rxIsrCount += framesReceived;

        if (framesReceived > 0)
        {
            g_rxDispatch++;
            ::systems::CanSystem::instance().dispatchRxTask();
        }
    }

    if ((ir & 0x80U) != 0U) // TC (bit 7) — Transmission Complete
    {
        g_txIsrCount++;
        ::bios::FdCanTransceiver::transmitInterrupt(::busid::CAN_0);
    }

    // Defensive: restore IE if it got corrupted
    uint32_t volatile* fdcan_ie = reinterpret_cast<uint32_t volatile*>(0x40006454U);
    if ((*fdcan_ie & 0x81U) != 0x81U) // RF0NE(bit0) + TCE(bit7)
    {
        *fdcan_ie |= 0x81U;
    }

    ::asyncLeaveIsrGroup(ISR_GROUP_CAN);
}

void call_can_isr_TX()
{
    // Not used — all interrupts routed to IT0
}
}
