// Copyright 2024 Accenture.

#include <bsp/timer/SystemTimer.h>
#include <etl/chrono.h>

#include <chrono>
using namespace ::std::chrono;

static auto const startTime = steady_clock::now();

uint64_t getSystemTimeNs()
{
    return duration_cast<nanoseconds>(steady_clock::now() - startTime).count();
}

extern "C"
{
etl::chrono::high_resolution_clock::rep etl_get_high_resolution_clock()
{
    return etl::chrono::high_resolution_clock::rep{static_cast<int64_t>(getSystemTimeNs())};
}

etl::chrono::system_clock::rep etl_get_system_clock()
{
    return etl::chrono::system_clock::rep(static_cast<int64_t>(getSystemTimeNs() / 1000 / 1000));
}

etl::chrono::steady_clock::rep etl_get_steady_clock()
{
    return etl::chrono::steady_clock::rep(
        static_cast<int64_t>(getSystemTimeNs() / 1000 / 1000 / 1000));
}
}

uint32_t getSystemTimeUs32Bit(void)
{
    auto elapsed = duration_cast<microseconds>(steady_clock::now() - startTime).count();
    return static_cast<uint32_t>(elapsed & 0xFFFFFFFF); // wrap naturally
}

uint32_t getSystemTimeMs32Bit(void)
{
    auto elapsed = duration_cast<milliseconds>(steady_clock::now() - startTime).count();
    return static_cast<uint32_t>(elapsed & 0xFFFFFFFF); // wrap naturally
}

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
