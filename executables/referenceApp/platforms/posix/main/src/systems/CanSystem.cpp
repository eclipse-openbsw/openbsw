/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "systems/CanSystem.h"

#include <bsp/timer/SystemTimer.h>
#include <can/CanLogger.h>

namespace systems
{

using ::util::logger::CAN;
using ::util::logger::Logger;

static uint32_t const TIMEOUT_CAN_SYSTEM_IN_MS = 1U;
static int const MAX_SENT_PER_RUN              = 3;
static int const MAX_RECEIVED_PER_RUN          = 3;

namespace
{
// clang-format off
::can::SocketCanTransceiver::DeviceConfig canConfig
{
    "vcan0", ::busid::CAN_0,
#if defined(PLATFORM_CAN0_USE_FD)
        true, /*enableCanFd*/
        true  /*enableBitRateSwitch*/
#else
        false, /*enableCanFd*/
        false  /*enableBitRateSwitch*/
#endif
};
// clang-format on
} // namespace

CanSystem::CanSystem(::async::ContextType context)
: _timeout()
, _context(context)
, _canTransceiver(canConfig)
, _busOffPolicy(NUM_CAN_TRCV)
{
    setTransitionContext(context);
}

void CanSystem::init() { transitionDone(); }

void CanSystem::run()
{
    _canTransceiver.init();
    _canTransceiver.open();
    _canTransceiver.setStateListener(*this);

    // On startup, request recovery in case the interface was left in bus-off
    // confinement from a previous run.
    _canTransceiver.manualBusOffRecovery();

    ::async::scheduleAtFixedRate(
        _context, *this, _timeout, TIMEOUT_CAN_SYSTEM_IN_MS, ::async::TimeUnit::MILLISECONDS);
    transitionDone();
}

void CanSystem::shutdown()
{
    _timeout.cancel();
    _canTransceiver.removeStateListener();
    _canTransceiver.close();
    _canTransceiver.shutdown();
    transitionDone();
}

::can::ICanTransceiver* CanSystem::getCanTransceiver(uint8_t busId)
{
    if (busId == ::busid::CAN_0)
    {
        return &_canTransceiver;
    }
    return nullptr;
}

void CanSystem::canTransceiverStateChanged(
    ::can::ICanTransceiver& /* transceiver */,
    ::can::ICANTransceiverStateListener::CANTransceiverState const state)
{
    if (_sleeping)
    {
        return;
    }

    using State = ::can::ICANTransceiverStateListener::CANTransceiverState;
    constexpr uint8_t index = 0U; // single controller

    switch (state)
    {
        case State::BUS_OFF:
        {
            // On POSIX the error frame is both the occurrence signal and the state-change signal.
            // The policy declares a failure when the occurrence count reaches the trip count
            // and both gating conditions hold (network mode active, no memory corruption).
            if (_busOffPolicy.onControllerBusOff(index))
            {
                raiseBusOffFailure(index);
            }
            break;
        }
        case State::ACTIVE:
        {
            uint32_t const nowUs
                = static_cast<uint32_t>(::bsp::SystemTimer::getSystemTimeUs32Bit());
            if (_busOffPolicy.onControllerActive(index, nowUs))
            {
                _canTransceiver.manualBusOffRecovery();
                notifyBusOffRecovered(index);
            }
            break;
        }
        case State::PASSIVE:
        {
            _busOffPolicy.onControllerInactive(index);
            break;
        }
        default:
            break;
    }
}

void CanSystem::phyErrorOccurred(::can::ICanTransceiver& /* transceiver */)
{
    // PHY-level error; not used by the bus-off state machine.
}

void CanSystem::resetAllBusOffState()
{
    bool const wasActive = _busOffPolicy.isFailureActive(0U);
    _busOffPolicy.resetAll();
    _canTransceiver.manualBusOffRecovery();
    if (wasActive)
    {
        notifyBusOffRecovered(0U);
    }
}

void CanSystem::sleep()
{
    _pendingSleep.store(true);
}

void CanSystem::wake()
{
    _pendingWake.store(true);
}

void CanSystem::raiseBusOffFailure(uint8_t const index)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): Logger API is variadic by design.
    Logger::warn(CAN, "[CanSystem] controller %u: bus-off failure declared", index);
    if (_busOffFailsafeListener != nullptr)
    {
        _busOffFailsafeListener->onBusOffFailureRaised(index);
    }
}

void CanSystem::notifyBusOffRecovered(uint8_t const index)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg): Logger API is variadic by design.
    Logger::info(CAN, "[CanSystem] controller %u: bus-off recovered", index);
    if (_busOffFailsafeListener != nullptr)
    {
        _busOffFailsafeListener->onBusOffFailureCleared(index);
    }
}

void CanSystem::execute()
{
    if (_pendingSleep.exchange(false))
    {
        // Clear bus-off state on the way into sleep so the failure does not
        // persist across the sleep boundary. manualBusOffRecovery() drains the
        // kernel RX buffer of queued bus-off error frames before _sleeping is set.
        resetAllBusOffState();
        _sleeping = true;
    }

    if (_pendingWake.exchange(false))
    {
        _sleeping = false;
        // manualBusOffRecovery() drains the kernel RX buffer of queued bus-off error frames.
        resetAllBusOffState();
    }

    _canTransceiver.run(MAX_SENT_PER_RUN, MAX_RECEIVED_PER_RUN);
}

} // namespace systems
