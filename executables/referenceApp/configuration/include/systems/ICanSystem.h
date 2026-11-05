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

#include <busid/BusId.h>
#include <platform/estdint.h>

namespace can
{
class ICanTransceiver;
class IBusOffFailsafeListener;

/**
 * Interface for lifecycle systems that provide access to CAN transceivers
 * and bus-off state management.
 */
class ICanSystem
{
protected:
    ~ICanSystem() = default;

public:
    /**
     * Returns a pointer to a ::can::ICanTransceiver for a given \p busId or nullptr if
     * there is no transceiver for the provided \p busId.
     */
    virtual ICanTransceiver* getCanTransceiver(uint8_t busId) = 0;

    /**
     * Register (or unregister with nullptr) the application-layer failsafe
     * listener. The listener is called on bus-off failure raise/clear transitions.
     */
    virtual void setBusOffFailsafeListener(IBusOffFailsafeListener* listener) = 0;

    /**
     * Unconditionally resets bus-off policy state and requests transceiver
     * recovery. Models Sleep->WakeUp and MCU Reset recovery paths.
     */
    virtual void resetAllBusOffState() = 0;

    /**
     * Sleep simulation: clears bus-off state and gates further transceiver
     * events until wake() is called.
     */
    virtual void sleep() = 0;

    /**
     * Wake simulation: re-enables transceiver events and calls resetAllBusOffState().
     */
    virtual void wake() = 0;

    /**
     * Returns true if the bus-off failure is currently declared for `controllerId`.
     */
    virtual bool isBusOffFailureActive(uint8_t controllerId) const = 0;
};
} // namespace can
