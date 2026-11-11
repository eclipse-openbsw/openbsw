/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <etl/limits.h>
#include <gtest/gtest.h>
#include <cstdint>

#include "middleware/core/Timer.h"

namespace middleware::core::test
{

TEST(TimerTest, DefaultConstructedTimerIsReset)
{
    Timer<uint8_t> timer;

    EXPECT_TRUE(timer.isReset());
    EXPECT_FALSE(timer.isExpired());
    EXPECT_FALSE(timer.isRunning());
    // Timer is initialized to Reset state (raw counter_ = 0).
    // getCounter() subtracts EXPIRED_TIME (1), causing unsigned underflow to
    // numeric_limits<T>::max().
    EXPECT_EQ(timer.getCounter(), etl::numeric_limits<uint8_t>::max());
}

TEST(TimerTest, ConstructedTimerStartsRunningAndCountsDownToExpired)
{
    Timer<uint8_t> timer(2U);

    EXPECT_TRUE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
    EXPECT_FALSE(timer.isReset());
    EXPECT_EQ(timer.getCounter(), 2U);

    timer.decrement();
    EXPECT_TRUE(timer.isRunning());
    EXPECT_EQ(timer.getCounter(), 1U);

    timer.decrement();
    EXPECT_FALSE(timer.isRunning());
    EXPECT_TRUE(timer.isExpired());
    EXPECT_EQ(timer.getCounter(), 0U);

    timer.decrement();
    // Once expired, further decrements should not change the state or counter.
    EXPECT_FALSE(timer.isRunning());
    EXPECT_TRUE(timer.isExpired());
    EXPECT_EQ(timer.getCounter(), 0U);
}

TEST(TimerTest, SetCounterResetAndExpireUpdateState)
{
    Timer<uint16_t> timer;

    timer.setCounter(7U);
    EXPECT_TRUE(timer.isRunning());
    EXPECT_EQ(timer.getCounter(), 7U);

    timer.expire();
    EXPECT_FALSE(timer.isRunning());
    EXPECT_TRUE(timer.isExpired());
    EXPECT_EQ(timer.getCounter(), 0U);

    timer.reset();
    EXPECT_TRUE(timer.isReset());
    EXPECT_FALSE(timer.isRunning());
    EXPECT_FALSE(timer.isExpired());
    EXPECT_EQ(timer.getCounter(), etl::numeric_limits<uint16_t>::max());
}

TEST(TickerTest, TickerTicksUntilExpiredAndCanRestart)
{
    Ticker<uint8_t, 2U> ticker;

    EXPECT_TRUE(ticker.isRunning());
    EXPECT_FALSE(ticker.isExpired());
    EXPECT_FALSE(ticker.isReset());
    EXPECT_EQ(ticker.elapsed(), 0U);

    EXPECT_TRUE(ticker.tick());
    EXPECT_TRUE(ticker.isRunning());
    EXPECT_EQ(ticker.elapsed(), 1U);

    EXPECT_FALSE(ticker.tick());
    EXPECT_TRUE(ticker.isExpired());
    EXPECT_FALSE(ticker.isRunning());
    EXPECT_EQ(ticker.elapsed(), 2U);

    EXPECT_FALSE(ticker.tick());
    EXPECT_TRUE(ticker.isExpired());
    EXPECT_EQ(ticker.elapsed(), 2U);

    ticker.restart();
    EXPECT_TRUE(ticker.isRunning());
    EXPECT_FALSE(ticker.isExpired());
    EXPECT_EQ(ticker.elapsed(), 0U);
}

TEST(TickerTest, LargestFiniteUint16PeriodExpiresAndCanRestart)
{
    constexpr uint32_t PERIOD{65534U};
    Ticker<uint16_t, PERIOD> ticker;

    for (uint32_t tick = 0U; tick < PERIOD - 1U; ++tick)
    {
        ASSERT_TRUE(ticker.tick());
    }
    EXPECT_FALSE(ticker.tick());
    EXPECT_TRUE(ticker.isExpired());

    ticker.restart();
    EXPECT_TRUE(ticker.isRunning());
    EXPECT_EQ(ticker.elapsed(), 0U);
}

TEST(TickerTest, MaximumUint16PeriodRemainsNonExpiringAfterRestart)
{
    constexpr uint16_t MAX_PERIOD{etl::numeric_limits<uint16_t>::max()};
    constexpr uint32_t OBSERVATION_TICKS{static_cast<uint32_t>(MAX_PERIOD) + 1U};
    Ticker<uint16_t, MAX_PERIOD> ticker;
    EXPECT_TRUE(ticker.isReset());

    ticker.expire();
    EXPECT_FALSE(ticker.tick());
    ticker.restart();
    for (uint32_t tick = 0U; tick < OBSERVATION_TICKS; ++tick)
    {
        ASSERT_TRUE(ticker.tick());
    }
    EXPECT_TRUE(ticker.isReset());
    EXPECT_FALSE(ticker.isExpired());
}

TEST(TickerTest, TickerExpiresAndResets)
{
    Ticker<uint16_t, 4U> ticker;

    ticker.expire();
    EXPECT_TRUE(ticker.isExpired());
    EXPECT_FALSE(ticker.isRunning());
    EXPECT_EQ(ticker.elapsed(), 4U);

    ticker.reset();
    EXPECT_TRUE(ticker.isReset());
    EXPECT_FALSE(ticker.isRunning());
    EXPECT_EQ(ticker.elapsed(), 0U);
}

} // namespace middleware::core::test
