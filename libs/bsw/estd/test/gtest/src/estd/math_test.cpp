// Copyright 2024 Accenture.

#include "estd/math.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::testing;

/**
 * \desc
 * This test verifies that the default return type of ::estd::abs() is uint64_t.
 */
TEST(Math, abs_default_return_type)
{
    int32_t number        = -1;
    auto const absMinAuto = ::estd::abs(number);
    EXPECT_EQ(absMinAuto, 1);
    EXPECT_THAT(absMinAuto, A<uint64_t>());
}

/**
 * \desc
 * This test verifies that the minimum value of a signed integer can have its absolute
 * value be assigned to an unsigned integer of the same size.
 */
TEST(Math, abs_signed_min_to_unsigned)
{
    auto absMinInt64 = ::estd::abs<uint64_t>(std::numeric_limits<int64_t>::min());
    EXPECT_EQ(absMinInt64, 9223372036854775808UL);
    EXPECT_THAT(absMinInt64, A<uint64_t>());

    auto absMinInt32 = ::estd::abs<uint32_t>(std::numeric_limits<int32_t>::min());
    EXPECT_EQ(absMinInt32, 2147483648U);
    EXPECT_THAT(absMinInt32, A<uint32_t>());

    auto absMinInt16 = ::estd::abs<uint16_t>(std::numeric_limits<int16_t>::min());
    EXPECT_EQ(absMinInt16, 32768);
    EXPECT_THAT(absMinInt16, A<uint16_t>());
}

/**
 * \desc
 * This test verifies that the minimum value of a signed integer of size X can have its absolute
 * value be assigned to a signed integer of size greater than X.
 */
TEST(Math, abs_signed_min_to_bigger_signed)
{
    auto absMinInt64 = ::estd::abs<int64_t>(std::numeric_limits<int32_t>::min());
    EXPECT_EQ(absMinInt64, 2147483648U);
    EXPECT_THAT(absMinInt64, A<int64_t>());

    auto absMinInt32 = ::estd::abs<int32_t>(std::numeric_limits<int16_t>::min());
    EXPECT_EQ(absMinInt32, 32768);
    EXPECT_THAT(absMinInt32, A<int32_t>());

    auto absMinInt16 = ::estd::abs<int16_t>(std::numeric_limits<int8_t>::min());
    EXPECT_EQ(absMinInt16, 128);
    EXPECT_THAT(absMinInt16, A<int16_t>());
}

/**
 * \desc
 * This test verifies that a value of an unsigned integer of size X can have its absolute value be
 * assigned to a signed integer of size greater than X.
 */
TEST(Math, abs_unsigned_max_to_bigger_signed)
{
    auto absMaxUInt64 = ::estd::abs<int64_t>(std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(absMaxUInt64, 4294967295);
    EXPECT_THAT(absMaxUInt64, A<int64_t>());

    auto absMaxUInt32 = ::estd::abs<int32_t>(std::numeric_limits<uint16_t>::max());
    EXPECT_EQ(absMaxUInt32, 65535);
    EXPECT_THAT(absMaxUInt32, A<int32_t>());

    auto absMaxUInt16 = ::estd::abs<int16_t>(std::numeric_limits<uint8_t>::max());
    EXPECT_EQ(absMaxUInt16, 255);
    EXPECT_THAT(absMaxUInt16, A<int16_t>());
}

/**
 * \desc
 * This test verifies that a value of an unsigned integer of size X can have its absolute value be
 * assigned to an unsigned integer of size X.
 */
TEST(Math, abs_unsigned_to_same_size_unsigned)
{
    auto absMinInt64 = ::estd::abs<uint64_t>(9999999999999999999UL);
    EXPECT_EQ(absMinInt64, 9999999999999999999UL);
    EXPECT_THAT(absMinInt64, A<uint64_t>());

    auto absMinInt32 = ::estd::abs<uint32_t>(999999999U);
    EXPECT_EQ(absMinInt32, 999999999U);
    EXPECT_THAT(absMinInt32, A<uint32_t>());

    uint16_t shortVal = 9999;
    auto absMinInt16  = ::estd::abs<uint16_t>(shortVal);
    EXPECT_EQ(absMinInt16, 9999U);
    EXPECT_THAT(absMinInt16, A<uint16_t>());
}

/**
 * \desc
 * This test verifies that a value of a floating point can have its absolute value be
 * assigned to a floatingpoint of the same size.
 */
TEST(Math, abs_signed_floating_to_bigger_size_signed_floating)
{
    auto absFloat = ::estd::abs<float>(-41.0f / 13.0f);
    EXPECT_EQ(absFloat, 41.0f / 13.0f);
    EXPECT_THAT(absFloat, A<float>());

    auto absLowestFloat = ::estd::abs<float>(std::numeric_limits<float>::lowest());
    EXPECT_EQ(absLowestFloat, std::numeric_limits<float>::max());
    EXPECT_THAT(absLowestFloat, A<float>());

    ASSERT_LT(-std::numeric_limits<float>::min(), 0);
    auto absMinFloat = ::estd::abs<float>(-std::numeric_limits<float>::min());
    EXPECT_EQ(absMinFloat, std::numeric_limits<float>::min());
    EXPECT_THAT(absMinFloat, A<float>());

    auto absDouble = ::estd::abs<double>(-41.0 / 13.0);
    EXPECT_EQ(absDouble, 41.0 / 13.0);
    EXPECT_THAT(absDouble, A<double>());

    auto absLowestDouble = ::estd::abs<double>(std::numeric_limits<double>::lowest());
    EXPECT_EQ(absLowestDouble, std::numeric_limits<double>::max());
    EXPECT_THAT(absLowestDouble, A<double>());

    ASSERT_LT(-std::numeric_limits<double>::min(), 0);
    auto absMinDouble = ::estd::abs<double>(-std::numeric_limits<double>::min());
    EXPECT_EQ(absMinDouble, std::numeric_limits<double>::min());
    EXPECT_THAT(absMinDouble, A<double>());
}
} // namespace
