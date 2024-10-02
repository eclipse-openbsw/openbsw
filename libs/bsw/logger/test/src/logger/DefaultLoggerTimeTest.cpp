// Copyright 2024 Accenture.

#include "logger/DefaultLoggerTime.h"

#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

using namespace ::estd;
using namespace ::util;
using namespace ::logger;

namespace
{
struct Clock
{
    using duration   = chrono::microseconds;
    using rep        = duration::rep;
    using period     = duration::period;
    using time_point = chrono::time_point<Clock, duration>;

    static chrono::time_point<Clock, duration> now()
    {
        return chrono::time_point<Clock, duration>(chrono::microseconds(_now));
    }

    static rep _now;
};

Clock::rep Clock::_now = 0;

TEST(DefaultLoggerTime, testNowIsCalledFromClock)
{
    using TimepointType = chrono::time_point<Clock, chrono::duration<int16_t, milli>>;
    DefaultLoggerTime<TimepointType> cut("%hd");
    Clock::_now = 123000;
    ASSERT_EQ(123, cut.getTimestamp());
}

TEST(DefaultLoggerTime, testTimeIsFormattedAsExpected)
{
    using TimepointType = chrono::time_point<Clock, chrono::duration<int16_t, milli>>;
    DefaultLoggerTime<TimepointType> cut("%04hd");
    stream::declare::StringBufferOutputStream<40> outputStream;
    cut.formatTimestamp(outputStream, 127);
    ASSERT_EQ("0127", std::string(outputStream.getString()));
}

} // namespace
