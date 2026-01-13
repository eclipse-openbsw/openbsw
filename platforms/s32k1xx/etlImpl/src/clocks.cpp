// Copyright 2026 BMW.

#include <bsp/timer/SystemTimer.h>
#include <etl/chrono.h>

extern "C"
{
etl::chrono::high_resolution_clock::rep etl_get_high_resolution_clock()
{
    return etl::chrono::high_resolution_clock::rep{static_cast<int64_t>(getSystemTimeNs())};
}

etl::chrono::system_clock::rep etl_get_system_clock()
{
    return etl::chrono::system_clock::rep(static_cast<int64_t>(getSystemTimeUs()));
}

etl::chrono::steady_clock::rep etl_get_steady_clock()
{
    return etl::chrono::steady_clock::rep(static_cast<int64_t>(getSystemTimeMs() / 1000));
}
}
