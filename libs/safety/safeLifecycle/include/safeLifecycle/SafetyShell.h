#pragma once

#include "safeLifecycle/interrupts/suspendResumeAllInterrupts.h"

#include <safeMemory/MemoryProtection.h>

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
        _psw     = getMachineStateRegisterValueAndSuspendAllInterrupts();
        _oldLock = safety::MemoryProtection::fusaGateGetLockAndUnlock();
    }

    /**
     * Leave safety shell (revokes write access, allows only read access and unlocks the
     * interrupts).
     */
    ~SafetyShell()
    {
        safety::MemoryProtection::fusaGateRestoreLock(_oldLock);
        (void)resumeAllInterrupts(_psw);
    }

private:
    uint32_t _psw;
    bool _oldLock;
};

} // namespace safety

