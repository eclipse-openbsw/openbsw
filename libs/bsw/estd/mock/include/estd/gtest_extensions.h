// Copyright 2024 Accenture.

#pragma once

#include <gmock/gmock.h>

namespace estd
{
namespace test
{
/* A matcher checking equality of both data() and size() of a slice, printing nicer output.
 * Usage:
 *
 * EXPECT_CALL(mock, send(Slice(my_array, 10)));
 *
 * EXPECT_THAT(my_slice, Slice(&my_array[42], 13));
 */
MATCHER_P2(
    Slice,
    dataPointer,
    expectedSize,
    "slice of " + ::testing::PrintToString(expectedSize) + " elements, starting at "
        + ::testing::PrintToString(dataPointer))
{
    if (dataPointer != arg.data())
    {
        *result_listener << "whose address " << ::testing::PrintToString(arg.data())
                         << " doesn't match";
    }

    if (static_cast<size_t>(expectedSize) != arg.size())
    {
        *result_listener << "whose size " << arg.size() << " doesn't match";
    }

    return (dataPointer == arg.data()) && (static_cast<size_t>(expectedSize) == arg.size());
}

MATCHER_P(
    Slice,
    expectedSlice,
    "slice of " + ::testing::PrintToString(expectedSlice.size()) + " elements, starting at "
        + ::testing::PrintToString(expectedSlice.data()))
{
    if (expectedSlice.data() != arg.data())
    {
        *result_listener << "whose address " << ::testing::PrintToString(arg.data())
                         << " doesn't match";
    }

    if (expectedSlice.size() != arg.size())
    {
        *result_listener << "whose size " << arg.size() << " doesn't match";
    }

    return (expectedSlice.data() == arg.data()) && (expectedSlice.size() == arg.size());
}

} // namespace test
} // namespace estd

