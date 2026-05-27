/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "interrupts/disableEnableAllInterruptsMock.h"

namespace interrupts
{
uint32_t DisableEnableAllInterruptsMock::disableAllInterruptsCount = 0U;
uint32_t DisableEnableAllInterruptsMock::enableAllInterruptsCount  = 0U;
} // namespace interrupts

extern "C"
{
void disableAllInterrupts(void)
{
    if (::interrupts::DisableEnableAllInterruptsMock::is_valid())
    {
        ::interrupts::DisableEnableAllInterruptsMock::instance().disableAllInterrupts();
    }
    else
    {
        ++::interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount;
    }
}

void enableAllInterrupts(void)
{
    if (::interrupts::DisableEnableAllInterruptsMock::is_valid())
    {
        ::interrupts::DisableEnableAllInterruptsMock::instance().enableAllInterrupts();
    }
    else
    {
        ++::interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount;
    }
}

bool areInterruptsDisabled(void)
{
    if (::interrupts::DisableEnableAllInterruptsMock::is_valid())
    {
        return ::interrupts::DisableEnableAllInterruptsMock::instance().areInterruptsDisabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        != interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

bool areInterruptsEnabled(void)
{
    if (::interrupts::DisableEnableAllInterruptsMock::is_valid())
    {
        return ::interrupts::DisableEnableAllInterruptsMock::instance().areInterruptsEnabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        == interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

} // extern "C" {
