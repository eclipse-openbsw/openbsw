// Copyright 2024 Accenture.

#include "estd/math.h"

#include <gmock/gmock.h>

using namespace ::testing;

TEST(Math, abs_default_return_type)
{
    int32_t number = -1;
    // [EXAMPLE_MATH_ABS_DEFAULT_RETURN_TYPE_START]

    auto const absMinAuto = ::estd::abs(number);

    // The absMinAuto holds the absolute value of the number passed.
    EXPECT_EQ(absMinAuto, 1);
    EXPECT_THAT(absMinAuto, A<uint64_t>());

    // [EXAMPLE_MATH_ABS_DEFAULT_RETURN_TYPE_END]
}

// This shows the functionality of abs() for different InputType and ReturnType.
TEST(Math, abs_signed_min_to_unsigned)
{
    // [EXAMPLE_MATH_ABS_SIGNED_MIN_TO_UNSIGNED_START]

    // Minimum value of a signed integer of size x can have its absolute value be assigned to an
    // unsigned/signed integer of the same/greater size than x.

    // Case1: Unsigned with same size.
    auto absMinInt64_1 = ::estd::abs<uint64_t>(std::numeric_limits<int64_t>::min());
    // Returning absolute value
    EXPECT_EQ(absMinInt64_1, 9223372036854775808UL);
    EXPECT_THAT(absMinInt64_1, A<uint64_t>());

    // Case2: Signed with greater size than x.
    auto absMinInt64_2 = ::estd::abs<int64_t>(std::numeric_limits<int32_t>::min());
    // Returning absolute value
    EXPECT_EQ(absMinInt64_2, 2147483648U);
    EXPECT_THAT(absMinInt64_2, A<int64_t>());

    // Value of an unsigned integer of size X can have its absolute value be assigned to a
    // signed/unsigned integer of same/greater size than x.

    // Case1: Signed with same size.
    auto absMinInt64_3 = ::estd::abs<uint64_t>(9999999999999999999UL);
    // Returning absolute value
    EXPECT_EQ(absMinInt64_3, 9999999999999999999UL);
    EXPECT_THAT(absMinInt64_3, A<uint64_t>());

    // Case2: Unsigned with greater size than x.
    uint16_t shortVal = 9999;
    auto absMinInt16  = ::estd::abs<uint16_t>(shortVal);
    // Returning absolute value
    EXPECT_EQ(absMinInt16, 9999U);
    EXPECT_THAT(absMinInt16, A<uint16_t>());

    auto absMaxUInt64 = ::estd::abs<int64_t>(std::numeric_limits<uint32_t>::max());
    // Returning absolute value
    EXPECT_EQ(absMaxUInt64, 4294967295);
    EXPECT_THAT(absMaxUInt64, A<int64_t>());

    // [EXAMPLE_MATH_ABS_SIGNED_MIN_TO_UNSIGNED_END]
}

TEST(Math, abs_signed_floating_to_bigger_size_signed_floating)
{
    // [EXAMPLE_ABS_SIGNED_FLOATING_START]

    // Example to show that a value of a floating point can have its absolute value be assigned to a
    // floating point of the same size.
    auto absFloat = ::estd::abs<float>(-41.0f / 13.0f);
    // Returning absolute value
    EXPECT_EQ(absFloat, 41.0f / 13.0f);
    EXPECT_THAT(absFloat, A<float>());

    auto absDouble = ::estd::abs<double>(-41.0 / 13.0);
    // Returning absolute value
    EXPECT_EQ(absDouble, 41.0 / 13.0);
    EXPECT_THAT(absDouble, A<double>());

    // [EXAMPLE_ABS_SIGNED_FLOATING_END]
}
