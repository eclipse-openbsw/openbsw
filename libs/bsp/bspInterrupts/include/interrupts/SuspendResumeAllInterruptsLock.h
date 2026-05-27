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
