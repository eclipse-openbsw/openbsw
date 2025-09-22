// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
class SafeWatchdog
{
public:
    // [PUBLIC_API_START]
    /**
     * Initializes the last watchdog service counter with maximum value, the counter will be
     * corrected in first cyclic() call.
     */
    SafeWatchdog();

    /**
     * Initializes the last watchdog service counter and calls the SafeWatchdog::enableMcuWatchdog
     * method.
     */
    void init();
    /**
     * Checks if the watchdog is running with the initial settings. If check fails, triggers
     * safeMonitor event which causes the software system reset. Checks if the watchdog is
     * not serviced more often than allowed, if fails triggers the reset through safeMonitor.
     * After these checks, it services the watchdog.
     */
    void cyclic();
    /**
     * Calls the enableMcuWatchdog method of bspMcuWatchdog by passing the watchdog timeout value.
     */
    static void enableMcuWatchdog();
    // [PUBLIC_API_END]

private:
    static uint32_t const WATCHDOG_TIMEOUT_MS = 250U;
    uint32_t const SERVICE_COUNTER_INIT       = 0xFFFFFFFFU;
    uint32_t _serviceCounter;
};

} // namespace safety
