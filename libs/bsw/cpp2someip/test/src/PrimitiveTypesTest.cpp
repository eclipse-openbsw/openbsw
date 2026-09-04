/********************************************************************************
 * Copyright (c) 2026 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "someip/PrimitiveTypes.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::someip;
using namespace ::someip::internal;

struct FloatFixture : public ::testing::TestWithParam<float_t>
{};

/*
 * IEC559 Not Supported tests
 */

TEST_P(FloatFixture, SimplePackFloat)
{
    float_t value = GetParam();

    uint32_t notSupported = PackIEEE754Dispatcher<false>::packIEEE754<uint32_t, float_t, 8U>(value);
    uint32_t supported    = PackIEEE754Dispatcher<true>::packIEEE754<uint32_t, float_t, 8U>(value);
    EXPECT_EQ(supported, notSupported);
}

TEST_P(FloatFixture, SimpleUnpackFloat)
{
    float_t value = GetParam();

    uint32_t packed = PackIEEE754Dispatcher<true>::packIEEE754<uint32_t, float_t, 8U>(value);

    float_t notSupported
        = UnpackIEEE754Dispatcher<false>::unpackIEEE754<float_t, uint32_t, 8U>(packed);
    float_t supported = UnpackIEEE754Dispatcher<true>::unpackIEEE754<float_t, uint32_t, 8U>(packed);
    EXPECT_EQ(supported, notSupported);
}

INSTANTIATE_TEST_SUITE_P(
    FloatIEEE754Test,
    FloatFixture,
    ::testing::Values(
        float_t(1.2345),
        float_t(0.0),
        float_t(-0.0),
        float_t(-1.2345),
        float_t(10.2345),
        float_t(-10.2345),
        std::numeric_limits<float_t>::infinity(),
        -std::numeric_limits<float_t>::infinity()));

struct DoubleFixture : public ::testing::TestWithParam<double_t>
{};

TEST_P(DoubleFixture, SimplePackDouble)
{
    double_t value = GetParam();

    uint64_t notSupported
        = PackIEEE754Dispatcher<false>::packIEEE754<uint64_t, double_t, 11U>(value);
    uint64_t supported = PackIEEE754Dispatcher<true>::packIEEE754<uint64_t, double_t, 11U>(value);
    EXPECT_EQ(supported, notSupported);
}

TEST_P(DoubleFixture, SimpleUnpackDouble)
{
    double_t value = GetParam();

    uint64_t packed = PackIEEE754Dispatcher<true>::packIEEE754<uint64_t, double_t, 11U>(value);

    double_t notSupported
        = UnpackIEEE754Dispatcher<false>::unpackIEEE754<double_t, uint64_t, 11U>(packed);
    double_t supported
        = UnpackIEEE754Dispatcher<true>::unpackIEEE754<double_t, uint64_t, 11U>(packed);
    EXPECT_EQ(supported, notSupported);
}

INSTANTIATE_TEST_SUITE_P(
    DoubleIEEE754Test,
    DoubleFixture,
    ::testing::Values(
        double_t(1.2345),
        double_t(0.0),
        double_t(-0.0),
        double_t(-1.2345),
        double_t(10.2345),
        double_t(-10.2345),
        std::numeric_limits<double_t>::infinity(),
        -std::numeric_limits<double_t>::infinity()));

TEST(PrimitiveTypes, FloatUnpack)
{
    float_t value = std::numeric_limits<float_t>::quiet_NaN();

    uint32_t packed = PackIEEE754Dispatcher<true>::packIEEE754<uint32_t, float_t, 8U>(value);

    float_t notSupported
        = UnpackIEEE754Dispatcher<false>::unpackIEEE754<float_t, uint32_t, 8U>(packed);
    float_t supported = UnpackIEEE754Dispatcher<true>::unpackIEEE754<float_t, uint32_t, 8U>(packed);
    EXPECT_NE(supported, supported);
    EXPECT_NE(notSupported, notSupported);
}

TEST(PrimitiveTypes, DoubleUnpack)
{
    double_t value = std::numeric_limits<double_t>::quiet_NaN();

    uint64_t packed = PackIEEE754Dispatcher<true>::packIEEE754<uint64_t, double_t, 11U>(value);

    double_t notSupported
        = UnpackIEEE754Dispatcher<false>::unpackIEEE754<double_t, uint64_t, 11U>(packed);
    double_t supported
        = UnpackIEEE754Dispatcher<true>::unpackIEEE754<double_t, uint64_t, 11U>(packed);
    EXPECT_NE(supported, supported);
    EXPECT_NE(notSupported, notSupported);
}

} // anonymous namespace
