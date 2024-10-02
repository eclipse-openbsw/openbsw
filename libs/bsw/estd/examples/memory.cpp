// Copyright 2024 Accenture.

#include "estd/memory.h"

#include "estd/none.h"

#include <gmock/gmock.h>

using namespace ::testing;

// [EXAMPLE_START]
// Sets all elements of the destination to the given value.
TEST(Memory, set)
{
    uint8_t destination[5] = {0};
    ::estd::memory::set(destination, 0xAA);

    EXPECT_THAT(destination, Each(0xAA));
}

// Copy data from source slice to destination slice.
TEST(Memory, copy)
{
    uint8_t const source[]  = {1, 2, 3};
    uint8_t destination[10] = {0};

    ::estd::slice<uint8_t> r = ::estd::memory::copy(destination, source);

    EXPECT_EQ(1, destination[0]);
    EXPECT_THAT(r, ElementsAre(1, 2, 3));

    EXPECT_EQ(destination, r.data());
    EXPECT_EQ(3U, r.size());
}

// [EXAMPLE_END]

// [COMPARISON_EXAMPLE_START]
// Checks whether the memory referred to by two slices contains the same data.
TEST(Memory, is_equal)
{
    uint8_t const a[] = {10, 20, 45, 32, 60};
    uint8_t const b[] = {10, 20, 45, 32, 60};
    uint8_t const c[] = {10, 20, 35, 32, 60};
    uint8_t const d[] = {10, 20, 35, 32};

    EXPECT_TRUE(::estd::memory::is_equal(a, b));
    EXPECT_FALSE(::estd::memory::is_equal(a, c));
    EXPECT_FALSE(::estd::memory::is_equal(b, c));
    EXPECT_FALSE(::estd::memory::is_equal(c, d));
    EXPECT_FALSE(::estd::memory::is_equal(d, c));
    EXPECT_FALSE(::estd::memory::is_equal(d, ::estd::none));
    EXPECT_FALSE(::estd::memory::is_equal(::estd::none, a));
    EXPECT_TRUE(::estd::memory::is_equal(::estd::none, ::estd::none));
}

//[COMPARISON_EXAMPLE_END]
