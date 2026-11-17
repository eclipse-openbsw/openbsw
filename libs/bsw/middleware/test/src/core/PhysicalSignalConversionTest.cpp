/********************************************************************************
 * Copyright (c) 2026 BMW AG
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "middleware/core/PhysicalSignalConversion.h"

#include <etl/array.h>
#include <etl/limits.h>
#include <gtest/gtest.h>
#include <cmath>

using middleware::core::physicalToRaw;

TEST(PhysicalSignalConversionTest, AppliesScaleOffsetAndTruncates)
{
    EXPECT_EQ(physicalToRaw(12.9, 0.5, 10.0, 5U), uint64_t{5U});
    EXPECT_EQ(physicalToRaw(7.1, -0.5, 10.0, 5U), uint64_t{5U});
    EXPECT_EQ(physicalToRaw(10.0, 0.5, 10.0, 5U), uint64_t{0U});
}

TEST(PhysicalSignalConversionTest, SaturatesToEachWireWidth)
{
    etl::array<uint8_t, 6U> const widths{{1U, 5U, 8U, 16U, 32U, 64U}};
    for (auto const width : widths)
    {
        SCOPED_TRACE(static_cast<unsigned>(width));
        uint64_t const maximum  = etl::numeric_limits<uint64_t>::max() >> (64U - width);
        double const upperBound = std::ldexp(1.0, width);
        EXPECT_EQ(physicalToRaw(upperBound, 1.0, 0.0, width), maximum);
        EXPECT_EQ(physicalToRaw(-1.0, 1.0, 0.0, width), uint64_t{0U});
        double const belowBound = std::nextafter(upperBound, 0.0);
        EXPECT_EQ(physicalToRaw(belowBound, 1.0, 0.0, width), static_cast<uint64_t>(belowBound));
    }
}

TEST(PhysicalSignalConversionTest, SaturatesInfinityForEitherScaleSign)
{
    double const infinity = etl::numeric_limits<double>::infinity();
    EXPECT_EQ(physicalToRaw(infinity, 0.5, 10.0, 5U), uint64_t{31U});
    EXPECT_EQ(physicalToRaw(-infinity, 0.5, 10.0, 5U), uint64_t{0U});
    EXPECT_EQ(physicalToRaw(infinity, -0.5, 10.0, 5U), uint64_t{0U});
    EXPECT_EQ(physicalToRaw(-infinity, -0.5, 10.0, 5U), uint64_t{31U});
}

TEST(PhysicalSignalConversionTest, SaturatesWhenFiniteArithmeticOverflows)
{
    double const maximum = etl::numeric_limits<double>::max();
    EXPECT_EQ(physicalToRaw(maximum, 1.0, -maximum, 64U), etl::numeric_limits<uint64_t>::max());
    EXPECT_EQ(physicalToRaw(-maximum, 1.0, maximum, 64U), uint64_t{0U});
    EXPECT_EQ(physicalToRaw(1.0, etl::numeric_limits<double>::min(), 0.0, 5U), uint64_t{31U});
}

TEST(PhysicalSignalConversionTest, NaNReturnsNoValue)
{
    EXPECT_FALSE(physicalToRaw(etl::numeric_limits<double>::quiet_NaN(), 0.5, 0.0, 5U));
}

TEST(PhysicalSignalConversionTest, InvalidConfigurationReturnsNoValue)
{
    double const infinity = etl::numeric_limits<double>::infinity();
    double const nan      = etl::numeric_limits<double>::quiet_NaN();
    EXPECT_FALSE(physicalToRaw(1.0, 0.0, 0.0, 5U));
    EXPECT_FALSE(physicalToRaw(1.0, infinity, 0.0, 5U));
    EXPECT_FALSE(physicalToRaw(1.0, nan, 0.0, 5U));
    EXPECT_FALSE(physicalToRaw(1.0, 1.0, infinity, 5U));
    EXPECT_FALSE(physicalToRaw(1.0, 1.0, nan, 5U));
    EXPECT_FALSE(physicalToRaw(1.0, 1.0, 0.0, 0U));
    EXPECT_FALSE(physicalToRaw(1.0, 1.0, 0.0, 65U));
}
