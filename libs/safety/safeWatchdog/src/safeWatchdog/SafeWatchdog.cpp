// Copyright 2024 Accenture.

#include "safeWatchdog/SafeWatchdog.h"

#include "safeLifecycle/SafeSupervisor.h"
#include "safeLifecycle/SafetyLogger.h"

#ifdef PLATFORM_SUPPORT_WATCHDOG
#include <mcu/mcu.h>
#include <watchdog/Watchdog.h>
#endif

namespace safety
{
using ::util::logger::Logger;
using ::util::logger::SAFETY;

SafeWatchdog::SafeWatchdog() : _serviceCounter(SERVICE_COUNTER_INIT) {}

void SafeWatchdog::init()
{
    // Variables will be initialized in first cyclic call
    _serviceCounter = SERVICE_COUNTER_INIT;
}

void SafeWatchdog::cyclic()
{
    auto& safeSupervisor = safety::SafeSupervisor::getInstance();
    // Checks if the Watchdog configuration is valid
    safeSupervisor.safeWatchdogConfigMonitor.check(checkWdConfigs());

#ifdef PLATFORM_SUPPORT_WATCHDOG
    auto const currentServiceCounter = bsp::Watchdog::getWatchdogServiceCounter();
    // check if watchdog was not triggered more often than allowed
    if (currentServiceCounter > (_serviceCounter + 1U))
    {
        safeSupervisor.serviceWatchdogMonitor.trigger();
    }
    _serviceCounter = currentServiceCounter;
    bsp::Watchdog::serviceWatchdog();
#endif
    safeSupervisor.safeWatchdogSequenceMonitor.hit(
        safety::SafeSupervisor::EnterLeaveSequence::LEAVE);
}

void SafeWatchdog::enableMcuWatchdog()
{
#ifdef PLATFORM_SUPPORT_WATCHDOG
    bsp::Watchdog::enableWatchdog(WATCHDOG_TIME_MS, true);
#endif
}

bool SafeWatchdog::checkWdConfigs()
{
#ifdef PLATFORM_SUPPORT_WATCHDOG
    uint32_t const MCU_WD_TIMEOUT = 0x00007D00U; // 250ms * 128000/1000

    uint32_t const WD_CONFIG_MASK = WDOG_CS_EN_MASK | WDOG_CS_CLK_MASK;
    bool const isWdConfigValid
        = ((WDOG->CS & WD_CONFIG_MASK) == (WDOG_CS_EN(1U) | WDOG_CS_CLK(1U)));
    bool const isWdTimeOutValid = ((WDOG->TOVAL & 0x0000FFFFU) == MCU_WD_TIMEOUT);

    return (isWdConfigValid && isWdTimeOutValid);
#endif
    return true;
}

} // namespace safety
