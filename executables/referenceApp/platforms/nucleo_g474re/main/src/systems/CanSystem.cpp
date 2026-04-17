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

// --- DiagListener: counts 0x7E0 frames that pass through notifyListeners filter ---
CanSystem::DiagListener::DiagListener() : _filter()
{
    _filter.add(0x7E0U);
}

void CanSystem::DiagListener::frameReceived(::can::CANFrame const& /* canFrame */) {}

::can::IFilter& CanSystem::DiagListener::getFilter() { return _filter; }

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    _canRxRunnable.setEnabled(true);

    // Accept all CAN frames (no HW filter). fFilterIds/fFilterCount are
    // uninitialized in FdCanDevice — must explicitly set to accept-all.
    _transceiver0.fDevice.fFilterIds   = nullptr;
    _transceiver0.fDevice.fFilterCount = 0U;

    (void)_transceiver0.init();
    (void)_transceiver0.open();

    // Register diagnostic listener BEFORE DoCAN to count 0x7E0 after filter match
    _transceiver0.addCANFrameListener(_diagListener);

    // Enable FDCAN IRQs after transceiver is open (not in setupApplicationsIsr,
    // because the async framework must be ready before ISRs fire)
    // Enable RX interrupt only. TCE is managed by FdCanDevice per-TX
    // (enabled before listener TX, disabled by transmitISR).
    FDCAN1->IE  = FDCAN_IE_RF0NE;
    FDCAN1->ILS = 0U;
    FDCAN1->ILE = FDCAN_ILE_EINT0;

    // Priority must be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (6)
    // for FreeRTOS API safety. Priority 5 is ABOVE the threshold — unsafe
    // for xTaskNotifyFromISR.
    SYS_SetPriority(FDCAN1_IT0_IRQn, 6);
    SYS_SetPriority(FDCAN1_IT1_IRQn, 6);
    NVIC_ClearPendingIRQ(FDCAN1_IT0_IRQn);
    NVIC_ClearPendingIRQ(FDCAN1_IT1_IRQn);
    SYS_EnableIRQ(FDCAN1_IT0_IRQn);
    SYS_EnableIRQ(FDCAN1_IT1_IRQn);

    // Schedule periodic TX callback poll. The ISR dispatches canTxRunnable
    // via async::execute, but dedup drops repeated dispatches when the
    // runnable is already queued. The periodic timer ensures pollTxCallback
    // runs within 50ms regardless of dedup.
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

void CanSystem::CanTxRunnable::execute()
{
    // Process TX completion in task context. ISR dispatches here via
    // async::execute after TC fires. pollTxCallback checks the TX queue
    // and invokes DoCAN's canFrameSentAsyncCallback for each completed frame.
    ::bios::FdCanTransceiver::pollTxCallback(::busid::CAN_0);
}

CanSystem::CanRxRunnable::CanRxRunnable(CanSystem& parent) : _parent(parent), _enabled(false) {}

void CanSystem::CanRxRunnable::execute()
{
    if (_enabled)
    {
        _parent._transceiver0.receiveTask();
    }
}

} // namespace systems

extern "C"
{
/**
 * Combined RX + TX ISR — all FDCAN1 interrupts routed to IT0.
 * Checks IR register to determine if RX, TX, or both occurred.
 */
void call_can_isr_RX()
{
    ::asyncEnterIsrGroup(ISR_GROUP_CAN);

    uint32_t const ir = FDCAN1->IR;

    if ((ir & FDCAN_IR_RF0N) != 0U)
    {
        ::async::LockType const lock;
        uint8_t framesReceived
            = ::bios::FdCanTransceiver::receiveInterrupt(::busid::CAN_0);

        if (framesReceived > 0)
        {
            ::systems::CanSystem::instance().dispatchRxTask();
        }
    }

    if ((ir & FDCAN_IR_TC) != 0U)
    {
        ::bios::FdCanTransceiver::transmitInterrupt(::busid::CAN_0);
    }

    ::asyncLeaveIsrGroup(ISR_GROUP_CAN);
}

void call_can_isr_TX()
{
    // Not used — all interrupts routed to IT0
}
}
