// Copyright 2024 Accenture.

#include "bsp/timer/isEqualAfterTimeout.h"

#include "bsp/timer/SystemTimerMock.h"

#include <gtest/gtest.h>

using namespace ::testing;

TEST(IsEqualAfterTimeoutTest, Check)
{
    StrictMock<SystemTimerMock> timer;

    EXPECT_CALL(timer, getSystemTimeUs()).Times(1).WillOnce(Return(11U));

    uint32_t address = 0x000000FFU;
    EXPECT_FALSE(::bsp::isEqualAfterTimeout<uint32_t>(&address, 0xFFFFFF00U, 0x0000FFFFU, 10U));

    EXPECT_CALL(timer, getSystemTimeUs())
        .Times(3)
        .WillOnce(Return(12U))
        .WillOnce(Return(20U))
        .WillOnce(Return(23U));
    EXPECT_TRUE(::bsp::isEqualAfterTimeout<uint32_t>(&address, 0xFFFFFFFFU, 0x000000FFU, 10U));
}
