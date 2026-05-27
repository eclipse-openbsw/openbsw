/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
