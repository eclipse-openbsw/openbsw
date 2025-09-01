// Copyright 2024 Accenture.

#pragma once

#include <platform/estdint.h>

namespace safety
{
class SafeWatchdog
{
public:
    SafeWatchdog();

    void init();
    void cyclic();
    static void enableMcuWatchdog();

private:
    static bool checkWdConfigs();

private:
    static uint32_t const WATCHDOG_TIME_MS = 250U;
    uint32_t const SERVICE_COUNTER_INIT    = 0xFFFFFFFFU;
    uint32_t _serviceCounter;
};

} // namespace safety
