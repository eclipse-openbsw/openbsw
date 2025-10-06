// Copyright 2024 Accenture.

#pragma once

#include "safeMemory/MemoryProtection.h"

#include <interrupts/SuspendResumeAllInterruptsLock.h>

namespace safety
{
/**
 * This class uses the RAII pattern and is used to grant temporary access to safety RAM and
 * the ISRs are locked while the safety RAM is writeable.
 */
class SafetyShell final
{
public:
    /**
     * Enter safety shell (locks the interrupts and opens the memory).
     */
    SafetyShell()
    {
        lock.suspend();
        _oldLock = MemoryProtection::fusaGateGetLockAndUnlock();
    }

    /**
     * Leave safety shell (revokes write access, allows only read access and unlocks the
     * interrupts).
     */
    ~SafetyShell()
    {
        MemoryProtection::fusaGateRestoreLock(_oldLock);
        lock.resume();
    }

private:
    ::interrupts::SuspendResumeAllInterruptsLock lock;
    bool _oldLock;
};

} // namespace safety
