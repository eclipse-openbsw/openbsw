// Copyright 2024 Accenture.

#include <async/Config.h>
#include <bsp/timer/SystemTimer.h>

#include <chrono>

uint64_t getSystemTimeNs()
{
    using namespace ::std::chrono;
    steady_clock::time_point const now = steady_clock::now();
    auto const duration                = now.time_since_epoch();
    return duration_cast<nanoseconds>(duration).count();
}

uint32_t getSystemTimeUs32Bit(void) { return getSystemTimeNs() / 1000; }

uint32_t getSystemTimeMs32Bit(void) { return getSystemTimeNs() / 1000 / 1000; }

void sysDelayUs(uint32_t const delay)
{
    uint32_t const t = getSystemTimeUs32Bit();
    while ((getSystemTimeUs32Bit() - t) < delay) {}
}

uint64_t const SystemTimeNs = getSystemTimeNs();

uint32_t getSystemTicks32Bit(void)
{
    return ((getSystemTimeNs() - SystemTimeNs) * ASYNC_CONFIG_TICK_IN_US) / 1000;
}
