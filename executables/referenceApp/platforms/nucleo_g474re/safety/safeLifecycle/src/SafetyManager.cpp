/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "safeLifecycle/SafetyManager.h"

#include <safeSupervisor/SafeSupervisor.h>
#include <safeUtils/SafetyLogger.h>
#include <watchdog/Watchdog.h>

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetyManager::SafetyManager() : _counter(0U) {}

void SafetyManager::init()
{
    Logger::debug(SAFETY, "SafetyManager initialized");

    if (bsp::Watchdog::isResetFromWatchdog())
    {
        Logger::warn(SAFETY, "Previous reset was caused by IWDG watchdog");
        bsp::Watchdog::clearResetFlag();
    }
}

void SafetyManager::run()
{
    // Enable IWDG here (not in init) - the cyclic scheduler is already running
    // at this point, so the 250ms timeout won't fire before the first kick.
    bsp::Watchdog::enableWatchdog(bsp::Watchdog::DEFAULT_TIMEOUT);
    Logger::debug(SAFETY, "IWDG watchdog enabled (250ms timeout)");
}

void SafetyManager::shutdown() {}

void SafetyManager::cyclic()
{
    auto& supervisor = SafeSupervisor::getInstance();
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);

    _counter++;
    if (_counter >= WATCHDOG_CYCLIC_COUNTER)
    {
        _counter = 0U;
        bsp::Watchdog::serviceWatchdog();
    }

    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
}
} // namespace safety
