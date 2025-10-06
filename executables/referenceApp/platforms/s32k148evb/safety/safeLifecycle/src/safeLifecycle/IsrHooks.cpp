// Copyright 2024 Accenture.

#include "safeLifecycle/IsrHooks.h"

#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <safeMemory/MemoryProtection.h>
#include <safeMemory/ProtectedRamScopedUnlock.h>

#include <platform/estdint.h>

uint32_t counterSafeIsrHook;

extern "C"
{
void resetCounterSafeIsrHook() { counterSafeIsrHook = 0U; }

void SafePreIsrHook()
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;

    if (counterSafeIsrHook > 0)
    {
        ::safety::ProtectedRamScopedUnlock_WithoutIsrLock const unlockRam;
        ++counterSafeIsrHook;
    }
    else
    {
        if (!::safety::MemoryProtection::fusaGateIsLocked())
        {
            counterSafeIsrHook = 1U;
            ::safety::MemoryProtection::fusaTaskStackCloseIsr();
            ::safety::MemoryProtection::fusaGateClose();
        }
    }
}

void SafePostIsrHook()
{
    ::interrupts::SuspendResumeAllInterruptsScopedLock const lock;

    if (counterSafeIsrHook > 1)
    {
        ::safety::ProtectedRamScopedUnlock_WithoutIsrLock const unlockRam;
        --counterSafeIsrHook;
    }
    else if (counterSafeIsrHook == 1)
    {
        ::safety::MemoryProtection::fusaTaskStackOpenIsr();
        ::safety::MemoryProtection::fusaGateOpen();
        counterSafeIsrHook = 0U;
    }
}
}
