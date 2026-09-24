/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
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
#include <safeWatchdog/SafeWatchdog.h>

::safety::SafeWatchdog safeWatchdog;

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetyManager::SafetyManager() : _counter(0U) {}

void SafetyManager::init()
{
    Logger::debug(SAFETY, "SafetyManager initialized");
    safeWatchdog.init();
}

void SafetyManager::run() {}

void SafetyManager::shutdown() {}

void SafetyManager::cyclic()
{
    auto& supervisor = SafeSupervisor::getInstance();
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);
    ++_counter;
    // The safeWatchdog cyclic is called every 80ms, to service the watchdog thrice before the
    // timeout(i.e 250ms) occurs
    if (_counter >= WATCHDOG_CYCLIC_COUNTER)
    {
        supervisor.safeWatchdogSequenceMonitor.hit(SafeSupervisor::EnterLeaveSequence::ENTER);
        safeWatchdog.cyclic();
        _counter = 0U;
    }
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
}

} // namespace safety
