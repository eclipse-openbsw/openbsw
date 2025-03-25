// Copyright 2024 Accenture.

#pragma once

#include "interrupts/suspendResumeAllInterrupts.h"

namespace interrupts
{
class SuspendResumeAllInterruptsLock
{
public:
    // [PUBLICAPI_START]
    /**
     * Suspend all interrupts and store previous state in an class internal variable
     */
    void suspend()
    {
        fOldMachineStateRegisterValue = getMachineStateRegisterValueAndSuspendAllInterrupts();
    }

    /**
     * Resume all interrupts restoring the interrupt state that has been saved during the suspend()
     * call from the class internal variable
     */
    void resume() { resumeAllInterrupts(fOldMachineStateRegisterValue); }

    // [PUBLICAPI_END]

private:
    uint32_t fOldMachineStateRegisterValue{};
};

} /* namespace interrupts */

