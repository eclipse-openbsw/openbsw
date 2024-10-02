// Copyright 2024 Accenture.

#include <estd/assert.h>
#include <estd/chrono.h>

#include <gtest/gtest.h>

#include <iostream>

namespace
{
namespace chrono_example
{
struct Clock
{
    using duration = estd::chrono::minutes;
};
} // namespace chrono_example

TEST(UniquePoolPtrExample, sum1)
{
    // [EXAMPLE_CHRONO_DURATION_START]

    // using milli = duration<int32_t>;
    estd::chrono::duration<int32_t, estd::milli> src(1001);
    {
        // using duration_cast to pass the value
        estd::chrono::duration<int32_t, estd::micro> cut
            = estd::chrono::duration_cast<estd::chrono::duration<int32_t, estd::micro>>(src);

        // using count to print duration
        std::cout << "Duration: " << cut.count() << "ms\n";

        // using extreme duration values from duration
        estd::chrono::duration<int8_t, estd::milli>::min().count();
        estd::chrono::duration<int8_t, estd::milli>::max().count();
    }

    // using seconds = duration<int64_t>;
    auto const ts1 = ::estd::chrono::seconds(42);

    // using milliseconds = duration<int64_t, milli>;
    auto const ts2 = ::estd::chrono::milliseconds(42000);

    // duration_cast using equality operator

    estd_assert(::estd::chrono::duration_cast<::estd::chrono::seconds>(ts2) == ts1);
    // [EXAMPLE_CHRONO_DURATION_END]
}

TEST(UniquePoolPtrExample, sum2)
{
    // [EXAMPLE_CHRONO_TIME_POINT_START]

    // time_point created
    estd::chrono::time_point<chrono_example::Clock, estd::chrono::milliseconds> cut;

    // the time after the clock start is 0
    std::cout << "time_point: " << cut.time_since_epoch().count() << "ms\n";

    // value of 715 micro seconds is passed to time_point
    estd::chrono::time_point<chrono_example::Clock, estd::chrono::microseconds> cut1(
        estd::chrono::microseconds(715));

    // the time after the clock start is 715 micro seconds
    std::cout << "time_point: " << cut1.time_since_epoch().count() << "ms\n";

    // using extreme duration values from time_point
    estd::chrono::time_point<chrono_example::Clock, estd::chrono::duration<int8_t, estd::milli>>::
        min()
            .time_since_epoch()
            .count();

    estd::chrono::time_point<chrono_example::Clock, estd::chrono::duration<int8_t, estd::milli>>::
        max()
            .time_since_epoch()
            .count();

    // [EXAMPLE_CHRONO_TIME_POINT_END]
}

} // namespace
