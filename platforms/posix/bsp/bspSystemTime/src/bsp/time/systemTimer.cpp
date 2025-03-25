// Copyright 2024 Accenture.

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

// only used in statistics, where it should not overflow for 1-second intervals.
// one-nanosecond "ticks" are good enough for that
uint32_t getSystemTicks32Bit(void)
{
    return static_cast<uint32_t>(getSystemTimeNs() - SystemTimeNs);
}

uint64_t systemTicksToTimeNs(uint64_t ticks) { return ticks; }

uint64_t systemTicksToTimeUs(uint64_t ticks) { return ticks / 1000; }

uint32_t getFastTicks(void) { return getSystemTimeUs32Bit(); }

uint32_t getFastTicksPerSecond(void) { return 1000000; }
