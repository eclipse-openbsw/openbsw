/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
