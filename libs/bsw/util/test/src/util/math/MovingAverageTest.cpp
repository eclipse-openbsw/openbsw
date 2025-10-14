// Copyright 2024 Accenture.

#include "util/math/MovingAverage.h"

#include <gtest/gtest.h>

using namespace ::testing;

struct MovingAverageTest : Test
{
    MovingAverageTest() : _average(0U) {}

    static uint8_t const AVERAGE_SIZE = 5U;
    ::util::math::MovingAverage<uint32_t, AVERAGE_SIZE> _average;
};

TEST_F(MovingAverageTest, add_and_get_succeed)
{
    _average.add(35U);
    EXPECT_EQ(7U, _average.get());

    _average.add(65U);
    EXPECT_EQ(20U, _average.get());

    _average.add(25U);
    EXPECT_EQ(25U, _average.get());

    _average.add(30U);
    EXPECT_EQ(31U, _average.get());

    _average.add(20U);
    EXPECT_EQ(35U, _average.get());

    // Rewrite the 1st element
    _average.add(5U);
    EXPECT_EQ(29U, _average.get());
}
