// Copyright 2024 Accenture.

#include <estd/tiny_ring.h>

#include <gtest/gtest.h>

TEST(TinyRingExample, construction_and_operation)
{
    // [EXAMPLE_TINY_RING_CONSTRUCTION_AND_OPERATION_START]
    // tiny_ring of size 3 is created.
    ::estd::tiny_ring<uint16_t, 3> r;
    // 'r.empty()' returns true, since there are no elements in the tiny_ring.
    ASSERT_TRUE(r.empty());

    // Pushing elements at the back of the tiny_ring using push_back() function.
    r.push_back(1);
    r.push_back(2);
    r.push_back(3);

    // 'r.full()' returns true, since tiny_ring is at maximum capacity.
    ASSERT_TRUE(r.full());

    // Get a copy of the oldest element in the tiny_ring.
    ASSERT_EQ(1, r.front());

    // Popping front element from the tiny_ring.
    r.pop_front();
    // The front element will be 2 after 'pop_front()'.
    ASSERT_EQ(2, r.front());
    // [EXAMPLE_TINY_RING_CONSTRUCTION_AND_OPERATION_END]
}

TEST(TinyRingExample, push_back)
{
    // [EXAMPLE_TINY_RING_PUSH_BACK_START]
    // tiny_ring of size 3 is created.
    ::estd::tiny_ring<uint16_t, 3> r;

    // Pushing elements at the back of the tiny_ring using push_back() function.
    r.push_back(1);
    r.push_back(2);
    r.push_back(3);

    // 'r.full()' returns true, since tiny_ring is at maximum capacity.
    ASSERT_TRUE(r.full());
    // The front element is 1.
    EXPECT_EQ(1, r.front());
    // Pushing element to overwrite the tiny_ring.
    r.push_back(4);
    // The front element will be 2 since 1 is overwritten.
    EXPECT_EQ(2, r.front());
    // [EXAMPLE_TINY_RING_PUSH_BACK_END]
}

TEST(TinyRingExample, data)
{
    // [EXAMPLE_TINY_RING_DATA_FUNCTION_START]
    ::estd::tiny_ring<uint16_t, 5> r;
    // r.data().size() returns 5, since the size of the tiny_ring is 5.
    ASSERT_EQ(5U, r.data().size());
    r.push_back(1);
    r.push_back(2);
    // r.data()[0] Returns 1, since 1 is at the index 0.
    ASSERT_EQ(1, r.data()[0]);
    // r.data()[1] Returns 2, since 2 is at the index 1.
    ASSERT_EQ(2, r.data()[1]);
    // [EXAMPLE_TINY_RING_DATA_FUNCTION_END]
}
