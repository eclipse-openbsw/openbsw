// Copyright 2025 BMW AG

#include "etl_chrono_stub.h"

#include <etl/chrono.h>

namespace
{
int64_t gTestClockUs = 0;
} // namespace

namespace middleware::time::test
{

void resetTestClock() { gTestClockUs = 0; }

} // namespace middleware::time::test

extern "C"
{
etl::chrono::high_resolution_clock::rep etl_get_high_resolution_clock()
{
    int64_t const result = gTestClockUs;
    gTestClockUs += 1000;
    return etl::chrono::high_resolution_clock::rep{result};
}

etl::chrono::system_clock::rep etl_get_system_clock()
{
    int64_t const result = gTestClockUs;
    gTestClockUs += 1000;
    return etl::chrono::system_clock::rep{result};
}

etl::chrono::steady_clock::rep etl_get_steady_clock()
{
    int64_t const result = gTestClockUs;
    gTestClockUs += 1000;
    return etl::chrono::steady_clock::rep{result};
}
}
