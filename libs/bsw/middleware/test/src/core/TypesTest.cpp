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

#include "middleware/core/types.h"

namespace middleware::core::test
{

TEST(AbsoluteToleranceEqualTest, ReturnsTrueForEqualFloatValues)
{
    AbsoluteToleranceEqual const equal{};

    EXPECT_TRUE(equal(1.0F, 1.0F));
}

TEST(AbsoluteToleranceEqualTest, ReturnsTrueAtFloatToleranceBoundary)
{
    AbsoluteToleranceEqual const equal{};
    float const tolerance = ::etl::numeric_limits<float>::min();

    EXPECT_TRUE(equal(0.0F, tolerance));
    EXPECT_TRUE(equal(0.0F, -tolerance));
}

TEST(AbsoluteToleranceEqualTest, ReturnsFalseOutsideFloatTolerance)
{
    AbsoluteToleranceEqual const equal{};
    float const tolerance = ::etl::numeric_limits<float>::min();

    EXPECT_FALSE(equal(0.0F, 2.0F * tolerance));
    EXPECT_FALSE(equal(0.0F, -2.0F * tolerance));
}

TEST(AbsoluteToleranceEqualTest, ReturnsTrueForEqualDoubleValues)
{
    AbsoluteToleranceEqual const equal{};

    EXPECT_TRUE(equal(1.0, 1.0));
}

TEST(AbsoluteToleranceEqualTest, ReturnsTrueAtDoubleToleranceBoundary)
{
    AbsoluteToleranceEqual const equal{};
    double const tolerance = ::etl::numeric_limits<double>::min();

    EXPECT_TRUE(equal(0.0, tolerance));
    EXPECT_TRUE(equal(0.0, -tolerance));
}

TEST(AbsoluteToleranceEqualTest, ReturnsFalseOutsideDoubleTolerance)
{
    AbsoluteToleranceEqual const equal{};
    double const tolerance = ::etl::numeric_limits<double>::min();

    EXPECT_FALSE(equal(0.0, 2.0 * tolerance));
    EXPECT_FALSE(equal(0.0, -2.0 * tolerance));
}

} // namespace middleware::core::test
