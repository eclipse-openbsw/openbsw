// Copyright 2024 Accenture.

#pragma once

#include "safeMemory/MemoryProtection.h"

#include <safeLifecycle/interrupts/suspendResumeAllInterrupts.h>

namespace safety
{
/**
 * This class is used to get temporary access to Safety RAM. ISRs are locked only during
 * the modification of MPU registers.
 */
class ProtectedRamScopedUnlock final
{
public:
    ProtectedRamScopedUnlock()
    {
        uint32_t const psw = safety::getMachineStateRegisterValueAndSuspendAllInterrupts();
        _oldLock           = MemoryProtection::fusaGateGetLockAndUnlock();
        (void)safety::resumeAllInterrupts(psw);
    }

    ~ProtectedRamScopedUnlock()
    {
        uint32_t const psw = safety::getMachineStateRegisterValueAndSuspendAllInterrupts();
        MemoryProtection::fusaGateRestoreLock(_oldLock);
        (void)safety::resumeAllInterrupts(psw);
    }

private:
    bool _oldLock;
};

/**
 * This class provides temporary access to Safety RAM. ISRs are not locked while changing
 * the MPU registers. It should only be used when ISRs are already locked.
 */
class ProtectedRamScopedUnlock_WithoutIsrLock final
{
public:
    ProtectedRamScopedUnlock_WithoutIsrLock()
    {
        _oldLock = MemoryProtection::fusaGateGetLockAndUnlock();
    }

    ~ProtectedRamScopedUnlock_WithoutIsrLock() { MemoryProtection::fusaGateRestoreLock(_oldLock); }

private:
    bool _oldLock;
};

} // namespace safety

