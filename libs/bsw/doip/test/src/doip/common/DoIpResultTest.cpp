/********************************************************************************
 * Copyright (c) 2025 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "doip/common/DoIpResult.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;
using namespace ::doip;

using TestDoIpResult = DoIpResult<uint16_t>;

TEST(DoIpResult, TestInitialization)
{
    {
        TestDoIpResult cut;
        EXPECT_EQ(0U, cut.getResult());
        EXPECT_EQ(0U, cut.getNackCode());
    }
    {
        TestDoIpResult cut(0x1234U, 0x45U);
        EXPECT_EQ(0x1234U, cut.getResult());
        EXPECT_EQ(0x45U, cut.getNackCode());
    }
}

TEST(DoIpResult, TestComparison)
{
    EXPECT_TRUE(TestDoIpResult() == TestDoIpResult());
    EXPECT_FALSE(TestDoIpResult() != TestDoIpResult());
    EXPECT_TRUE(TestDoIpResult(0x1234U, 0x45U) == TestDoIpResult(0x1234U, 0x45U));
    EXPECT_FALSE(TestDoIpResult(0x1234U, 0x45U) != TestDoIpResult(0x1234U, 0x45U));
    EXPECT_FALSE(TestDoIpResult(0x123U, 0x45U) == TestDoIpResult(0x1234U, 0x45U));
    EXPECT_TRUE(TestDoIpResult(0x123U, 0x45U) != TestDoIpResult(0x1234U, 0x45U));
    EXPECT_FALSE(TestDoIpResult(0x1234U, 0x43U) == TestDoIpResult(0x1234U, 0x45U));
    EXPECT_TRUE(TestDoIpResult(0x1234U, 0x43U) != TestDoIpResult(0x1234U, 0x45U));
}

} // anonymous namespace
