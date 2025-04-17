// Copyright 2024 Accenture.

#include "safeLifecycle/SafetyManager.h"

#include "safeLifecycle/SafeSupervisor.h"
#include "safeLifecycle/SafetyLogger.h"

#include <safeLifecycle/SafetyShell.h>
#include <safeMemory/MemoryProtection.h>
#include <safeMemory/ProtectedRamScopedUnlock.h>
#include <safeWatchdog/SafeWatchdog.h>

::safety::SafeWatchdog safeWatchdog;

namespace safety
{

using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafetyManager::SafetyManager() : _counter(0) {}

void SafetyManager::init()
{
    Logger::warn(SAFETY, "SafetyManager initialized");
#ifdef PLATFORM_SUPPORT_MPU
    MemoryProtection::init();
    {
        SafetyShell const safetyShell;
        safeWatchdog.init();
    }
#endif
}

void SafetyManager::run() {}

void SafetyManager::shutdown() {}

void SafetyManager::cyclic()
{
#ifdef PLATFORM_SUPPORT_MPU
    bool const safeRamLockStatusOnEnter = safety::MemoryProtection::fusaGateIsLocked();
    safety::ProtectedRamScopedUnlock const protectedRamUnlock;
#endif
    auto& supervisor = SafeSupervisor::getInstance();
#ifdef PLATFORM_SUPPORT_MPU
    supervisor.mpuStatusCheckOnEnterMonitor.check(safeRamLockStatusOnEnter);
#endif
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
    supervisor.safetyManagerSequenceMonitor.hit(
        SafeSupervisor::SafetyManagerSequence::SAFETY_MANAGER_LEAVE);
#ifdef PLATFORM_SUPPORT_MPU
    bool const safeRamLockStatusOnLeave = safety::MemoryProtection::fusaGateIsLocked();
    supervisor.mpuStatusCheckOnLeaveMonitor.check(safeRamLockStatusOnLeave);
#endif
}
} // namespace safety
