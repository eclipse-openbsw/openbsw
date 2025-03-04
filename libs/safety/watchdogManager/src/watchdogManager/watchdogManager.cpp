// Copyright 2024 Accenture.

#include "watchdogManager/watchdogManager.h"

#include "bsp/timer/SystemTimer.h"
#include "commonDebug.h"
#ifdef PLATFORM_SUPPORT_WATCHDOG
#include "mcu/mcu.h"
#include "watchdog/Watchdog.h"
#endif

namespace safety
{
bool WatchdogManager::startTest()
{
    if (isWDFastTestLow())
    {
#ifdef PLATFORM_SUPPORT_WATCHDOG
        bsp::Watchdog::startFastTestHigh();
#endif
        static uint32_t const TIME_OUT_FAST_TEST_HIGH_US = 6000U;
        sysDelayUs(TIME_OUT_FAST_TEST_HIGH_US);
        // Watchdog reset should occur before reaching the return statement
        return false;
    }
    else if (isWDFastTestHigh())
    {
#ifdef PLATFORM_SUPPORT_WATCHDOG
        bsp::Watchdog::setUserMode();
#endif
        LOGSYNCHRON("MCU watchdog fast tests successful\r\n");
    }
    else
    {
#ifdef PLATFORM_SUPPORT_WATCHDOG
        bsp::Watchdog::startFastTestLow();
#endif
        static uint32_t const TIME_OUT_FAST_TEST_LOW_US = 6000U;
        sysDelayUs(TIME_OUT_FAST_TEST_LOW_US);
        // Watchdog reset should occur before reaching the return statement
        return false;
    }
    return true;
}

bool WatchdogManager::isWDFastTestLow()
{
#ifdef PLATFORM_SUPPORT_WATCHDOG
    return (WDOG->CS & WDOG_CS_TST_MASK) == WDOG_CS_TST(0x02U);
#endif
    return false;
}

bool WatchdogManager::isWDFastTestHigh()
{
#ifdef PLATFORM_SUPPORT_WATCHDOG
    return (WDOG->CS & WDOG_CS_TST_MASK) == WDOG_CS_TST(0x03U);
#endif
    return false;
}
} // namespace safety
