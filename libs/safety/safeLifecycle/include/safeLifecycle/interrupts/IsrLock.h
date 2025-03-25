// Copyright 2024 Accenture.

#pragma once

#include "safeLifecycle/interrupts/suspendResumeAllInterrupts.h"

namespace safety
{
class IsrLock final
{
public:
    IsrLock()
    {
        fOldMachineStateRegisterValue = getMachineStateRegisterValueAndSuspendAllInterrupts();
    }

    ~IsrLock() { resumeAllInterrupts(fOldMachineStateRegisterValue); }

private:
    uint32_t fOldMachineStateRegisterValue;
};

} // end namespace safety
