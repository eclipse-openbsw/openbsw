/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/Closure.h"

#include <cstdint>

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;

uint16_t globalX;
uint16_t globalY;

struct ClosureTest : ::testing::Test
{
    ClosureTest()
    {
        globalX = 0U;
        globalY = 0U;
    }

    void callback(uint16_t x) { _localX = x; }

    void callback2(uint16_t& x, uint16_t y)
    {
        _localX = x;
        _localY = y;
    }

    uint16_t _localX = 0U;
    uint16_t _localY = 0U;
};

void functionCallback(uint16_t x) { globalX = x; }

void functionCallback2(uint16_t& x, uint16_t y)
{
    globalX = x;
    globalY = y;
}

TEST_F(ClosureTest, Callback_FromObject)
{
    Callback<uint16_t> c
        = Callback<uint16_t>::fromObject<ClosureTest, &ClosureTest::callback>(*this);

    c(10U);

    EXPECT_EQ(10U, _localX);
}

TEST_F(ClosureTest, Callback_FromFunction)
{
    Callback<uint16_t> c = Callback<uint16_t>::fromFunction<&functionCallback>();

    c(10U);

    EXPECT_EQ(10U, globalX);
}

TEST_F(ClosureTest, BoundCallback_FromObject)
{
    BoundCallback<uint16_t, uint16_t> c
        = BoundCallback<uint16_t, uint16_t>::fromObject<ClosureTest, &ClosureTest::callback2>(
            *this, 10U);

    c(20U);

    EXPECT_EQ(10U, _localX);
    EXPECT_EQ(20U, _localY);
}

TEST_F(ClosureTest, BoundCallback_FromFunction)
{
    BoundCallback<uint16_t, uint16_t> c
        = BoundCallback<uint16_t, uint16_t>::fromFunction<&functionCallback2>(10U);

    c(20U);

    EXPECT_EQ(10U, globalX);
    EXPECT_EQ(20U, globalY);
}

TEST_F(ClosureTest, BoundCallback_GetParam1_Const)
{
    BoundCallback<uint16_t, uint16_t> const c
        = BoundCallback<uint16_t, uint16_t>::fromFunction<&functionCallback2>(10U);

    EXPECT_EQ(10U, c.getParam1());
}

TEST_F(ClosureTest, BoundCallback_GetParam1)
{
    BoundCallback<uint16_t, uint16_t> c
        = BoundCallback<uint16_t, uint16_t>::fromFunction<&functionCallback2>(10U);

    EXPECT_EQ(10U, c.getParam1());
}

} // anonymous namespace
