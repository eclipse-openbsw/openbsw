/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "routing/util.h"

#include <etl/limits.h>

#include <gmock/gmock.h>

namespace
{
struct ClampedCounterTest : ::testing::Test
{
    using T = uint8_t;

    static constexpr T MAX = ::etl::numeric_limits<T>::max();

    ::routing::ClampedCounter<T> c;
};

constexpr ClampedCounterTest::T ClampedCounterTest::MAX;

/**
 * \desc: Check that prefix increment works.
 */
TEST_F(ClampedCounterTest, prefix_increment)
{
    ASSERT_EQ(c, 0U);
    ASSERT_EQ(++c, 1U);
    ASSERT_EQ(++c, 2U);
    ASSERT_EQ(++c, 3U);
}

/**
 * \desc: Check that postfix incement works.
 */
TEST_F(ClampedCounterTest, postfix_incement)
{
    ASSERT_EQ(c++, 0U);
    ASSERT_EQ(c++, 1U);
    ASSERT_EQ(c++, 2U);
    ASSERT_EQ(c, 3U);
}

/**
 * \desc: Check that addition assignment works.
 */
TEST_F(ClampedCounterTest, addition_assignment)
{
    ASSERT_EQ(c, 0U);
    c += 0U;
    ASSERT_EQ(c, 0U);
    c += 1U;
    ASSERT_EQ(c, 1U);
    c += 10U;
    ASSERT_EQ(c, 11U);
    c += 100U;
    ASSERT_EQ(c, 111U);
}

/**
 * \desc: Check that counter does not exceed upper bound with prefix increment.
 */
TEST_F(ClampedCounterTest, prefix_increment_upper_bound)
{
    for (uint8_t i = 0U; i < MAX; ++i)
    {
        ++c;
    }
    ASSERT_EQ(++c, MAX);
    ASSERT_EQ(++c, MAX);
    ASSERT_EQ(++c, MAX);
}

/**
 * \desc: Check that counter does not exceed upper bound with postfix increment.
 */
TEST_F(ClampedCounterTest, postfix_increment_upper_bound)
{
    for (uint8_t i = 0U; i < MAX; ++i)
    {
        c++;
    }
    c++;
    ASSERT_EQ(c++, MAX);
    ASSERT_EQ(c++, MAX);
    ASSERT_EQ(c, MAX);
}

/**
 * \desc: Check that counter does not exceed upper bound with addition assignment.
 */
TEST_F(ClampedCounterTest, addition_assignment_upper_bound)
{
    c += 100U;
    ASSERT_EQ(c, 100U);
    c += 100U;
    ASSERT_EQ(c, 200U);
    c += 100U;
    ASSERT_EQ(c, MAX);
    c += 0U;
    ASSERT_EQ(c, MAX);
    c += 100U;
    ASSERT_EQ(c, MAX);
}

} // anonymous namespace
