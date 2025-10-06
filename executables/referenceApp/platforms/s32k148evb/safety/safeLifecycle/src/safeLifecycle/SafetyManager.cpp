// Copyright 2024 Accenture.

#include "safeLifecycle/SafetyManager.h"

#include <safeLifecycle/IsrHooks.h>
#include <safeMemory/MemoryProtection.h>
#include <safeMemory/ProtectedRamScopedUnlock.h>
#include <safeMemory/RomCheck.h>
#include <safeMemory/SafeMemory.h>
#include <safeMemory/SafetyShell.h>
#include <safeSupervisor/SafeSupervisor.h>
#include <safeSystem/SafeSystem.h>
#include <safeUtils/SafetyLogger.h>
#include <safeWatchdog/SafeWatchdog.h>

::safety::SafeWatchdog safeWatchdog;
::safety::SafeSystem safeSystem;

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetyManager::SafetyManager() : _counter(0) {}

void SafetyManager::init()
{
    Logger::warn(SAFETY, "SafetyManager initialized");
    {
        SafetyShell const safetyShell;
        resetCounterSafeIsrHook();
        safeWatchdog.init();
        safeSystem.init();
    }
    RomCheck::init();
}

void SafetyManager::run() {}

void SafetyManager::shutdown() {}

void SafetyManager::cyclic()
{
    bool const safeRamLockStatusOnEnter = MemoryProtection::fusaGateIsLocked();
    ProtectedRamScopedUnlock const protectedRamUnlock;
    auto& supervisor = SafeSupervisor::getInstance();
    supervisor.mpuStatusCheckOnEnterMonitor.check(safeRamLockStatusOnEnter);
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_ENTER);
    ++_counter;
    // The safeWatchdog cyclic is called every 80ms, to service the watchdog thrice before the
    // timeout(i.e 250ms) occurs
    if (_counter >= WATCHDOG_CYCLIC_COUNTER)
    {
        supervisor.safeWatchdogSequenceMonitor.hit(
            safety::SafeSupervisor::EnterLeaveSequence::ENTER);
        safeWatchdog.cyclic();
        _counter = 0;
    }
    safe_memory::cyclic();
    safeSystem.cyclic();
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
    bool const safeRamLockStatusOnLeave = MemoryProtection::fusaGateIsLocked();
    supervisor.mpuStatusCheckOnLeaveMonitor.check(safeRamLockStatusOnLeave);
}
} // namespace safety
