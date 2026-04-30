// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file CanSystem.cpp
 * \brief CAN system implementation for the NUCLEO-F413ZH platform (bxCAN1).
 *
 * Contains the CanSystem lifecycle methods, bxCAN1 peripheral configuration
 * (500 kbit/s @ 48 MHz APB1), and the extern "C" ISR trampolines that bridge
 * hardware interrupts into the openBSW async framework.
 */

#include "systems/CanSystem.h"

#include "async/Config.h"
#include "async/Hook.h"
#include "busid/BusId.h"
#include "mcu/mcu.h"

namespace
{
// bxCAN1 configuration for STM32F413ZH
// CAN1: PD0 (RX, AF9), PD1 (TX, AF9), 500 kbit/s @ 48 MHz APB1
// BTR: SJW=1TQ, BS1=13TQ, BS2=2TQ, prescaler=6 → 500 kbit/s
// (from CubeMX IOC: cuberzccf4fg.ioc)
::bios::BxCanDevice::Config const can1Config = {
    CAN1,  // peripheral
    6U,    // prescaler (48 MHz / 6 = 8 MHz → 16 TQ @ 500k)
    13U,   // bs1 (13 TQ)
    2U,    // bs2 (2 TQ)
    1U,    // sjw (1 TQ)
    GPIOD, // rxPort
    0U,    // rxPin (PD0)
    9U,    // rxAf (AF9 = CAN1)
    GPIOD, // txPort
    1U,    // txPin (PD1)
    9U,    // txAf (AF9 = CAN1)
};
} // namespace

namespace systems
{

CanSystem::CanSystem(::async::ContextType context)
: ::lifecycle::SingleContextLifecycleComponent(context)
, ::etl::singleton_base<CanSystem>(*this)
, _context(context)
, _transceiver0(context, ::busid::CAN_0, can1Config)
, _canRxRunnable(*this)
{}

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    _canRxRunnable.setEnabled(true);

    (void)_transceiver0.init();
    (void)_transceiver0.open();

    // Enable CAN IRQs after transceiver is open (not in setupApplicationsIsr,
    // because the async framework must be ready before ISRs fire)
    // Priority must equal configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (6)
    // so BASEPRI (0x60) masks the ISR during FreeRTOS critical sections.
    // Original priority 8 was ALWAYS masked. Priority 5 is NEVER masked.
    // Priority 6 = threshold = correctly managed by FreeRTOS.
    SYS_SetPriority(CAN1_RX0_IRQn, 6);
    SYS_SetPriority(CAN1_TX_IRQn, 6);
    NVIC_ClearPendingIRQ(CAN1_RX0_IRQn);
    NVIC_ClearPendingIRQ(CAN1_TX_IRQn);
    SYS_EnableIRQ(CAN1_RX0_IRQn);
    SYS_EnableIRQ(CAN1_TX_IRQn);

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
 * CAN receive interrupt service routine trampoline for bxCAN1.
 *
 * Disables the RX FIFO interrupt (FMPIE0) to prevent re-entry while the 3-deep
 * hardware FIFO refills, reads pending frames under an async lock, and dispatches
 * the CanRxRunnable to drain the software queue. The RX interrupt is re-enabled
 * in receiveTask() after processing completes.
 *
 * \note ISR context — must not call blocking APIs or allocate memory.
 * \note FMPIE0 is disabled at entry and re-enabled either here (no frames) or
 *       in receiveTask() (frames dispatched) to prevent ISR storm.
 */
void call_can_isr_RX()
{
    ::bios::BxCanTransceiver::disableRxInterrupt(::busid::CAN_0);
    ::asyncEnterIsrGroup(ISR_GROUP_CAN);

    uint8_t framesReceived;
    {
        ::async::LockType const lock;
        framesReceived = ::bios::BxCanTransceiver::receiveInterrupt(::busid::CAN_0);
    }

    if (framesReceived > 0)
    {
        ::systems::CanSystem::instance().dispatchRxTask();
    }
    else
    {
        ::bios::BxCanTransceiver::enableRxInterrupt(::busid::CAN_0);
    }

    ::asyncLeaveIsrGroup(ISR_GROUP_CAN);
}

/**
 * CAN transmit interrupt service routine trampoline for bxCAN1.
 *
 * Enters the CAN ISR group, forwards the TX-complete event to the BxCanTransceiver,
 * and leaves the ISR group.
 *
 * \note ISR context — must not call blocking APIs or allocate memory.
 */
void call_can_isr_TX()
{
    ::asyncEnterIsrGroup(ISR_GROUP_CAN);
    ::bios::BxCanTransceiver::transmitInterrupt(::busid::CAN_0);
    ::asyncLeaveIsrGroup(ISR_GROUP_CAN);
}
}
