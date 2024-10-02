// Copyright 2024 Accenture.

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
    if (::interrupts::DisableEnableAllInterruptsMock::instantiated())
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
    if (::interrupts::DisableEnableAllInterruptsMock::instantiated())
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
    if (::interrupts::DisableEnableAllInterruptsMock::instantiated())
    {
        return ::interrupts::DisableEnableAllInterruptsMock::instance().areInterruptsDisabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        != interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

bool areInterruptsEnabled(void)
{
    if (::interrupts::DisableEnableAllInterruptsMock::instantiated())
    {
        return ::interrupts::DisableEnableAllInterruptsMock::instance().areInterruptsEnabled();
    }
    return (
        interrupts::DisableEnableAllInterruptsMock::disableAllInterruptsCount
        == interrupts::DisableEnableAllInterruptsMock::enableAllInterruptsCount);
}

} // extern "C" {
