/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "interrupts/SuspendResumeAllInterruptsMock.h"

namespace interrupts
{
uint32_t SuspendResumeAllInterruptsMock::getOldIntEnabledStatusValueAndSuspendAllInterruptsCount
    = 0U;
uint32_t SuspendResumeAllInterruptsMock::getMachineStateRegisterValueAndSuspendAllInterruptsCount
    = 0U;
uint32_t SuspendResumeAllInterruptsMock::resumeAllInterruptsCount = 0U;
} // namespace interrupts

extern "C"
{
OldIntEnabledStatusValueType getOldIntEnabledStatusValueAndSuspendAllInterrupts()
{
    if (::interrupts::SuspendResumeAllInterruptsMock::is_valid())
    {
        return ::interrupts::SuspendResumeAllInterruptsMock::instance()
            .getOldIntEnabledStatusValueAndSuspendAllInterrupts();
    }
    return ++::interrupts::SuspendResumeAllInterruptsMock::
        getOldIntEnabledStatusValueAndSuspendAllInterruptsCount;
}

uint32_t getMachineStateRegisterValueAndSuspendAllInterrupts(void)
{
    if (::interrupts::SuspendResumeAllInterruptsMock::is_valid())
    {
        return ::interrupts::SuspendResumeAllInterruptsMock::instance()
            .getMachineStateRegisterValueAndSuspendAllInterrupts();
    }
    return ++::interrupts::SuspendResumeAllInterruptsMock::
        getMachineStateRegisterValueAndSuspendAllInterruptsCount;
}

void resumeAllInterrupts(OldIntEnabledStatusValueType oldIntEnabledStatusValue)
{
    if (::interrupts::SuspendResumeAllInterruptsMock::is_valid())
    {
        ::interrupts::SuspendResumeAllInterruptsMock::instance().resumeAllInterrupts(
            oldIntEnabledStatusValue);
    }
    else
    {
        ++::interrupts::SuspendResumeAllInterruptsMock::resumeAllInterruptsCount;
    }
}

} // extern "C" {
