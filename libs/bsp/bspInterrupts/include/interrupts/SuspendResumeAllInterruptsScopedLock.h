// Copyright 2024 Accenture.

#pragma once

#include "interrupts/suspendResumeAllInterrupts.h"

namespace interrupts
{
class SuspendResumeAllInterruptsScopedLock
{
public:
    // [PUBLICAPI_START]
    /**
     * Create a lock object instance with disabling of all interrupts
     * Store the current interrupt state on instance creation in a private member variable
     */
    SuspendResumeAllInterruptsScopedLock()
    : fOldMachineStateRegisterValue(getMachineStateRegisterValueAndSuspendAllInterrupts())
    {}

    /**
     * Destroy the lock object instance and restore the internally stored interrupt state from
     * before this object instance has been created
     */
    ~SuspendResumeAllInterruptsScopedLock() { resumeAllInterrupts(fOldMachineStateRegisterValue); }

    // [PUBLICAPI_END]

private:
    uint32_t fOldMachineStateRegisterValue;
};

} /* namespace interrupts */

