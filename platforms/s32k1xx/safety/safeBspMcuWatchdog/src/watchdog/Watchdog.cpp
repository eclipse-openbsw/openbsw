// Copyright 2024 Accenture.

#include "watchdog/Watchdog.h"

#include <bsp/timer/SystemTimer.h>
#include <interrupts/SuspendResumeAllInterruptsScopedLock.h>
#include <mcu/mcu.h>

namespace safety
{
namespace bsp
{
uint32_t Watchdog::watchdogServiceCounter = 0;

void Watchdog::enableWatchdog(
    uint32_t const timeout, bool const interruptActive, uint32_t const clockSpeed)
{
    ESR_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;

    if (interruptActive)
    {
        uint32_t const irqNumber = WDOG_EWM_IRQn;
        NVIC->ISER[irqNumber >> 5U]
            = (static_cast<uint32_t>(1U) << irqNumber); // Enable WDOG_EWM_IRQn
    }

    WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
    while ((WDOG->CS & WDOG_CS_ULK_MASK) == 0U)
    {
        // wait until registers are unlocked
    }
    WDOG->TOVAL            = (clockSpeed / 1000U) * timeout;
    uint32_t controlStatus = WDOG_CS_UPDATE(1U) | WDOG_CS_EN(1U) | WDOG_CS_CLK(1U)
                             | WDOG_CS_CMD32EN(1U) | WDOG_CS_STOP(1U) | WDOG_CS_WAIT(1U);

    if (interruptActive)
    {
        controlStatus |= WDOG_CS_INT(1U);
    }
    controlStatus |= (WDOG->CS & WDOG_CS_TST_MASK);
    WDOG->CS = controlStatus;

    while ((WDOG->CS & WDOG_CS_RCS_MASK) == 0U)
    {
        // wait until new configuration takes effect
    }
}

void Watchdog::disableWatchdog()
{
    ESR_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;

    WDOG->CNT             = FEATURE_WDOG_UNLOCK_VALUE;
    uint32_t const CS_TST = (WDOG->CS & WDOG_CS_TST_MASK);
    WDOG->CS              = (CS_TST | WATCHDOG_DISABLE);
    WDOG->TOVAL           = static_cast<uint32_t>(0xFFFFU);
}

void Watchdog::serviceWatchdog()
{
    ESR_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;

    watchdogServiceCounter++;

    WDOG->CNT = FEATURE_WDOG_TRIGGER_VALUE;
}

bool Watchdog::checkWatchdogConfiguration(uint32_t const timeout, uint32_t const clockSpeed)
{
    uint32_t const MCU_WD_TIMEOUT = (clockSpeed / 1000U) * timeout;
    uint32_t const WD_CONFIG_MASK = WDOG_CS_EN_MASK | WDOG_CS_CLK_MASK;
    bool const isWdConfigValid
        = ((WDOG->CS & WD_CONFIG_MASK) == (WDOG_CS_EN(1U) | WDOG_CS_CLK(1U)));
    bool const isWdTimeoutValid = ((WDOG->TOVAL & 0x0000FFFFU) == MCU_WD_TIMEOUT);

    return (isWdConfigValid && isWdTimeoutValid);
}

void Watchdog::startFastTestLow()
{
    ESR_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;

    WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
    while ((WDOG->CS & WDOG_CS_ULK_MASK) == 0U)
    {
        // wait until registers are unlocked
    }

    WDOG->TOVAL = WDOG_TOVAL_TOVALLOW_MASK;
    uint32_t const controlStatus
        = WDOG_CS_UPDATE(1U) | WDOG_CS_EN(1U) | WDOG_CS_CLK(1U) | WDOG_CS_TST(0x02U);

    WDOG->CS = controlStatus;

    while ((WDOG->CS & WDOG_CS_RCS_MASK) == 0U)
    {
        // wait until new configuration takes effect
    }
}

void Watchdog::startFastTestHigh()
{
    ESR_UNUSED const interrupts::SuspendResumeAllInterruptsScopedLock lock;

    WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
    while ((WDOG->CS & WDOG_CS_ULK_MASK) == 0U)
    {
        // wait until registers are unlocked
    }

    WDOG->TOVAL = WDOG_TOVAL_TOVALLOW_MASK;
    uint32_t const controlStatus
        = WDOG_CS_UPDATE(1U) | WDOG_CS_EN(1U) | WDOG_CS_CLK(1U) | WDOG_CS_TST(0x03U);

    WDOG->CS = controlStatus;

    while ((WDOG->CS & WDOG_CS_RCS_MASK) == 0U)
    {
        // wait until new configuration takes effect
    }
}

bool Watchdog::executeFastTest(uint32_t const timeout)
{
    if (isWdFastTestLow())
    {
        startFastTestHigh();
        sysDelayUs(timeout);
        // Watchdog reset should occur before reaching the return statement
        return false;
    }
    else if (isWdFastTestHigh())
    {
        setUserMode();
        return true;
    }
    else
    {
        startFastTestLow();
        sysDelayUs(timeout);
        // Watchdog reset should occur before reaching the return statement
        return false;
    }
}

void Watchdog::setUserMode()
{
    ESR_UNUSED const ::interrupts::SuspendResumeAllInterruptsScopedLock lock;

    WDOG->CNT = FEATURE_WDOG_UNLOCK_VALUE;
    while ((WDOG->CS & WDOG_CS_ULK_MASK) == 0U)
    {
        // wait until registers are unlocked
    }

    uint32_t const controlStatus
        = WDOG_CS_UPDATE(1U) | WDOG_CS_CLK(1U) | WDOG_CS_CMD32EN(1U) | WDOG_CS_TST(1U);

    WDOG->CS = controlStatus;

    while ((WDOG->CS & WDOG_CS_RCS_MASK) == 0U)
    {
        // wait until new configuration takes effect
    }
}

uint32_t Watchdog::getWatchdogServiceCounter() { return watchdogServiceCounter; }

bool Watchdog::isWdFastTestLow()
{
    auto const WDOG_CS_TST_FAST_LOW = WDOG_CS_TST(0x02U);
    return (WDOG->CS & WDOG_CS_TST_MASK) == WDOG_CS_TST_FAST_LOW;
}

bool Watchdog::isWdFastTestHigh()
{
    auto const WDOG_CS_TST_FAST_HIGH = WDOG_CS_TST(0x03U);
    return (WDOG->CS & WDOG_CS_TST_MASK) == WDOG_CS_TST_FAST_HIGH;
}

} // namespace bsp
} // namespace safety
