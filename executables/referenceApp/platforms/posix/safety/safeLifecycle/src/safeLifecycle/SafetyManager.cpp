// Copyright 2025 Accenture.

#include "safeLifecycle/SafetyManager.h"

#include <safeSupervisor/SafeSupervisor.h>
#include <safeUtils/SafetyLogger.h>

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetyManager::SafetyManager() {}

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
