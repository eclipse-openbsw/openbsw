/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "safeWatchdog/SafeWatchdog.h"

#include <safeSupervisor/SafeSupervisor.h>

#ifdef PLATFORM_SUPPORT_WATCHDOG
#include <mcu/mcu.h>
#include <watchdog/Watchdog.h>
#endif

namespace safety
{

SafeWatchdog::SafeWatchdog() : _serviceCounter(SERVICE_COUNTER_INIT) {}

void SafeWatchdog::init()
{
    // Variables will be initialized in first cyclic call
    _serviceCounter = SERVICE_COUNTER_INIT;
    enableMcuWatchdog();
}

void SafeWatchdog::cyclic()
{
    auto& safeSupervisor = SafeSupervisor::getInstance();
#ifdef PLATFORM_SUPPORT_WATCHDOG
    safeSupervisor.safeWatchdogConfigMonitor.check(
        bsp::Watchdog::checkWatchdogConfiguration(WATCHDOG_TIMEOUT_MS));

    auto const currentServiceCounter = bsp::Watchdog::getWatchdogServiceCounter();
    // check if watchdog was not triggered more often than allowed
    if (currentServiceCounter > (_serviceCounter + 1U))
    {
        safeSupervisor.serviceWatchdogMonitor.trigger();
    }
    _serviceCounter = currentServiceCounter;
    bsp::Watchdog::serviceWatchdog();
#endif
    safeSupervisor.safeWatchdogSequenceMonitor.hit(SafeSupervisor::EnterLeaveSequence::LEAVE);
}

void SafeWatchdog::enableMcuWatchdog()
{
#ifdef PLATFORM_SUPPORT_WATCHDOG
    bsp::Watchdog::enableWatchdog(WATCHDOG_TIMEOUT_MS, true);
#endif
}

} // namespace safety
