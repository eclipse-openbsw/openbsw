// Copyright 2024 Accenture.

#include "estd/tiny_ring.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

TEST(A_tiny_ring, is_initially_empty)
{
    ::estd::tiny_ring<uint16_t, 5> r;
    ASSERT_TRUE(r.empty());
}

TEST(A_tiny_ring, data_returns_slice_of_correct_size_and_values)
{
    ::estd::tiny_ring<uint16_t, 5> r;
    ASSERT_EQ(5U, r.data().size());
    r.push_back(1);
    r.push_back(2);
    ASSERT_EQ(1, r.data()[0]);
    ASSERT_EQ(2, r.data()[1]);
}

TEST(A_tiny_ring, can_be_pushed_and_popped_a_single_element)
{
    ::estd::tiny_ring<uint16_t, 5> r;
    r.push_back(4);
    ASSERT_FALSE(r.empty());
    ASSERT_EQ(4, r.front());
    r.pop_front();
    ASSERT_TRUE(r.empty());

    r.push_back(5);
    ASSERT_FALSE(r.empty());
    ASSERT_EQ(5, r.front());
    r.pop_front();
    ASSERT_TRUE(r.empty());
}

TEST(A_tiny_ring, can_be_pushed_and_popped_two_elements)
{
    ::estd::tiny_ring<uint16_t, 5> r;
    r.push_back(5);
    r.push_back(6);
    ASSERT_FALSE(r.empty());
    ASSERT_EQ(5, r.front());
    r.pop_front();
    ASSERT_FALSE(r.empty());
    ASSERT_EQ(6, r.front());
    r.pop_front();
    ASSERT_TRUE(r.empty());
}

TEST(A_tiny_ring, overwrites_when_full)
{
    ::estd::tiny_ring<uint16_t, 5> r;
    r.push_back(0);
    ASSERT_EQ(0, r.front());
    r.push_back(1);
    ASSERT_EQ(0, r.front());
    r.push_back(2);
    ASSERT_EQ(0, r.front());
    r.push_back(3);
    ASSERT_EQ(0, r.front());
    r.push_back(4);
    ASSERT_EQ(0, r.front());
    r.push_back(5);
    ASSERT_EQ(1, r.front());
    r.push_back(6);
    ASSERT_EQ(2, r.front());

    r.pop_front();
    ASSERT_EQ(3, r.front());
    r.pop_front();
    ASSERT_EQ(4, r.front());
    r.pop_front();
    ASSERT_EQ(5, r.front());
    r.pop_front();
    ASSERT_EQ(6, r.front());

    ASSERT_FALSE(r.empty());
    r.pop_front();
    ASSERT_TRUE(r.empty());
}

/*
 * Ensure the tiny ring doesn't write past the end of the internal array when _read + _count == N,
 * and _read != 0. A previous version of tiny_ring misapplied the order of operations, allowing a
 * remainder (modulo) operation to take place before an addition when calculating an array write
 * offset. This test ensures that mistake can't happen again.
 */
TEST(A_tiny_ring, loops_back_when_read_pointer_plus_count_full)
{
    ::estd::tiny_ring<uint16_t, 5> r;

    // push 4/5 elements
    r.push_back(0xFFF0);
    r.push_back(0xFFF1);
    r.push_back(0xFFF2);
    r.push_back(0xFFF3);

    // pop the first, 3/5 taken
    r.pop_front();

    // push 2 more elements, so 5/5 written
    r.push_back(0xFFF4);
    // On this push, _read + _count == N. With the bug, this would overwrite the _read & _count
    // variables to be 0xFF and 0xF5 respectively, but in the fixed version this correctly pushes
    // 0xFFF5 to the first index of the array.
    r.push_back(0xFFF5);

    ASSERT_EQ(0xFFF1, r.front());
    ASSERT_THAT(r.data(), ElementsAre(0xFFF5, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4));

    r.push_back(0xFFF6);
    ASSERT_EQ(0xFFF2, r.front());
    ASSERT_THAT(r.data(), ElementsAre(0xFFF5, 0xFFF6, 0xFFF2, 0xFFF3, 0xFFF4));
}

/**
 * \desc
 * This test ensures, that a tiny_ring with maximum capacity (255) report full() == true when adding
 * 256 elements.
 */
TEST(A_tiny_ring, with_maximum_capacity_is_full_when_pushing_N_elements)
{
    ::estd::tiny_ring<uint16_t, 255> ring;
    for (uint16_t i = 1; i <= 255; ++i)
    {
        ring.push_back(i);
    }
    EXPECT_TRUE(ring.full());
}

/**
 * \desc
 * This test ensures, that a tiny_ring with maximum capacity (255) can also store this number of
 * elements and return them correctly.
 */
TEST(A_tiny_ring, with_maximum_capacity_can_store_and_read_N_elements)
{
    ::estd::tiny_ring<uint16_t, 255> ring;
    for (uint16_t i = 1; i <= 255; ++i)
    {
        ring.push_back(i);
    }
    for (uint16_t i = 1; i <= 255; ++i)
    {
        EXPECT_EQ(i, ring.front());
        ring.pop_front();
    }
    EXPECT_TRUE(ring.empty());
}
