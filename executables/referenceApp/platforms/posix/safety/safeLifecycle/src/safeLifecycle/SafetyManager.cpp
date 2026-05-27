/********************************************************************************
 * Copyright (c) 2025 Accenture
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

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

void SafetyManager::init() { Logger::debug(SAFETY, "SafetyManager initialized"); }

void SafetyManager::run() {}

void SafetyManager::shutdown() {}

void SafetyManager::cyclic()
{
    auto& supervisor = SafeSupervisor::getInstance();
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);

    // Cyclic safety checks

    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
}
} // namespace safety
