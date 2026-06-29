/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "time/TimestampProvider.h"

#include "bsp/timer/SystemTimerMock.h"

#include <gtest/gtest.h>

using namespace ::testing;

TEST(TimestampProviderTest, getTimestampUs32Bit_returns_system_time)
{
    StrictMock<SystemTimerMock> timer;

    EXPECT_CALL(timer, getSystemTimeUs32Bit()).WillOnce(Return(42U));

    EXPECT_EQ(42U, ::bsw::time::TimestampProvider::getTimestampUs32Bit());
}

TEST(TimestampProviderTest, getTimestampUs64Bit_returns_system_time)
{
    StrictMock<SystemTimerMock> timer;

    EXPECT_CALL(timer, getSystemTimeUs64Bit()).WillOnce(Return(0x1'0000'0042ULL));

    EXPECT_EQ(0x1'0000'0042ULL, ::bsw::time::TimestampProvider::getTimestampUs64Bit());
}

TEST(TimestampProviderTest, getTimestampMs32Bit_returns_system_time)
{
    StrictMock<SystemTimerMock> timer;

    EXPECT_CALL(timer, getSystemTimeMs32Bit()).WillOnce(Return(42U));

    EXPECT_EQ(42U, ::bsw::time::TimestampProvider::getTimestampMs32Bit());
}

TEST(TimestampProviderTest, getTimestampMs64Bit_returns_system_time)
{
    StrictMock<SystemTimerMock> timer;

    EXPECT_CALL(timer, getSystemTimeMs64Bit()).WillOnce(Return(0x1'0000'0042ULL));

    EXPECT_EQ(0x1'0000'0042ULL, ::bsw::time::TimestampProvider::getTimestampMs64Bit());
}
