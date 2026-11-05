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

#include <platform/estdint.h>

namespace can
{

/**
 * Integration seam for bus-off failsafe output actions. Implement this
 * interface to respond to bus-off failure declaration and clearance events
 * produced by CanBusOffPolicy and surfaced by the platform CanSystem.
 *
 * Register an instance via ICanSystem::setBusOffFailsafeListener(); passing
 * nullptr leaves the seam inert.
 */
class IBusOffFailsafeListener
{
protected:
    ~IBusOffFailsafeListener() = default;

public:
    /**
     * Called once a bus-off failure is newly declared (rising edge) for
     * `controllerId`. The implementation should apply application-level
     * failsafe outputs for the functions carried by this controller.
     */
    virtual void onBusOffFailureRaised(uint8_t controllerId) = 0;

    /**
     * Called once a bus-off failure clears (any recovery path: counter,
     * Sleep->WakeUp, MCU Reset, or explicit diagnostic clear) for
     * `controllerId`. The implementation should release failsafe outputs
     * and return to normal control.
     */
    virtual void onBusOffFailureCleared(uint8_t controllerId) = 0;
};

} // namespace can
