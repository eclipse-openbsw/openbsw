/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <atomic>

#include <bsp/timer/SystemTimer.h>
#include <can/SocketCanTransceiver.h>
#include <can/transceiver/ICANTransceiverStateListener.h>
#include <canBusOff/CanBusOffPolicy.h>
#include <lifecycle/AsyncLifecycleComponent.h>
#include <systems/IBusOffFailsafeListener.h>
#include <systems/ICanSystem.h>

namespace systems
{

class CanSystem final
: public ::can::ICanSystem
, public ::lifecycle::AsyncLifecycleComponent
, public ::can::ICANTransceiverStateListener
, private ::async::IRunnable
{
public:
    // [PUBLIC_API_START]
    explicit CanSystem(::async::ContextType context);
    CanSystem(CanSystem const&)            = delete;
    CanSystem& operator=(CanSystem const&) = delete;

    ::can::ICanTransceiver* getCanTransceiver(uint8_t busId) override;

    void init() final;
    void run() final;
    void shutdown() final;

    void canTransceiverStateChanged(
        ::can::ICanTransceiver& transceiver,
        ::can::ICANTransceiverStateListener::CANTransceiverState state) override;

    void phyErrorOccurred(::can::ICanTransceiver& transceiver) override;

    void resetAllBusOffState() override;
    void sleep() override;
    void wake() override;

    bool isSleeping() const { return _sleeping; }

    void setBusOffFailsafeListener(::can::IBusOffFailsafeListener* listener) override
    {
        _busOffFailsafeListener = listener;
    }

    void setNetworkMode(bool underNetworkMode) { _busOffPolicy.setNetworkMode(underNetworkMode); }
    bool isUnderNetworkMode() const { return _busOffPolicy.isUnderNetworkMode(); }

    void setMemoryAreaDataCorruptionDetermined(bool determined)
    {
        _busOffPolicy.setMemoryAreaDataCorruptionDetermined(determined);
    }
    bool isMemoryAreaDataCorruptionDetermined() const
    {
        return _busOffPolicy.isMemoryAreaDataCorruptionDetermined();
    }

    uint8_t getBusOffCount(uint8_t controllerId) const
    {
        return _busOffPolicy.getBusOffCount(controllerId);
    }

    bool isBusOffFailureActive(uint8_t controllerId) const override
    {
        return _busOffPolicy.isFailureActive(controllerId);
    }
    // [PUBLIC_API_END]

private:
    void execute() final;
    void raiseBusOffFailure(uint8_t index);
    void notifyBusOffRecovered(uint8_t index);

private:
    static constexpr uint8_t NUM_CAN_TRCV = 1U;

    ::async::TimeoutType _timeout;
    ::async::ContextType _context;

    ::can::SocketCanTransceiver _canTransceiver;

    ::canBusOff::CanBusOffPolicy _busOffPolicy;

    ::can::IBusOffFailsafeListener* _busOffFailsafeListener{nullptr};

    std::atomic<bool> _pendingSleep{false};
    std::atomic<bool> _pendingWake{false};
    bool _sleeping{false};
};

} // namespace systems
