// Copyright 2024 Accenture.

#include "estd/static_assert.h"

#include "estd/array.h"

#include <gmock/gmock.h>

// [EXAMPLE_ESTD_STATIC_ASSERT_START]
TEST(Assert, ZeroLength)
{
    // Compilation stops with error message ESTD_STATIC_ASSERT, if the array is created with size 0.
    ::estd::array<int32_t, 4> zeroLengthArray{};

    ESTD_STATIC_ASSERT(zeroLengthArray.size() > 0);
}

// [EXAMPLE_ESTD_STATIC_ASSERT_END]

// [EXAMPLE_ESTD_STATIC_ASSERT_MESSAGE_START]
TEST(AssertMessage, ZeroLength)
{
    // If the array is created with size 0, compilation stops with error message provided as
    // argument.
    ::estd::array<int32_t, 3> zeroLengthArray{};

    ESTD_STATIC_ASSERT_MESSAGE(zeroLengthArray.size() > 0, "Zero sized array should be avoided");
}

// [EXAMPLE_ESTD_STATIC_ASSERT_MESSAGE_END]
