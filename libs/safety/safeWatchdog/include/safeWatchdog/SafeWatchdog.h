// Copyright 2024 Accenture.

#ifndef GUARD_C6B62A33_2D22_4527_BAEF_D5952662B626
#define GUARD_C6B62A33_2D22_4527_BAEF_D5952662B626

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

    static uint8_t getCsTst();

private:
    static bool checkWdConfigs();

private:
    static uint32_t const WATCHDOG_TIME_MS = 250U;
    uint32_t const SERVICE_COUNTER_INIT    = 0xFFFFFFFFU;
    uint32_t _serviceCounter;
};

} // namespace safety

#endif /* GUARD_C6B62A33_2D22_4527_BAEF_D5952662B626 */
