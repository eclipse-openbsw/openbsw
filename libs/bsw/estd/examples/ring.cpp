// Copyright 2024 Accenture.

#include <estd/ring.h>

#include <gtest/gtest.h>

TEST(RingExample, construction_and_operation)
{
    // [EXAMPLE_RING_CONSTRUCTION_AND_OPERATION_START]
    // The array of size 1000 is created.
    uint8_t mem[1000];
    // The slice object is referencing to array named mem.
    ::estd::slice<uint8_t> s(mem);
    // ring of size 5 is created from slice object.
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);

    // Checks whether the ring r is empty.
    ASSERT_TRUE(r.empty());

    // Checking the size of the ring.
    EXPECT_EQ(5U, r.length());

    // Pushing elements at the back of the ring using push_back() function.
    r.push_back(10);
    r.push_back(20);
    r.push_back(30);

    // Checks the number of used index of the ring.
    EXPECT_EQ(3, r.used());

    // Element at particular index is accessed using at() function.
    EXPECT_EQ(10, r.at(0));
    EXPECT_EQ(20, r.at(1));
    EXPECT_EQ(30, r.at(2));

    // The oldest or the front element can be accessed using front() function.
    EXPECT_EQ(10, r.front());
    // Popping front element from the ring.
    r.pop_front();
    // The front element will be 20 after pop_front().
    ASSERT_EQ(20, r.front());
    r.pop_front();
    r.pop_front();
    // 'r.empty()' returns true, since there are no elements in the ring.
    EXPECT_TRUE(r.empty());
    // [EXAMPLE_RING_CONSTRUCTION_AND_OPERATION_END]
}

TEST(RingExample, push_back)
{
    // [EXAMPLE_RING_PUSH_BACK_START]
    // The array of size 100 is created.
    uint8_t mem[100];
    // The slice object is referencing to array named mem.
    ::estd::slice<uint8_t> s(mem);
    // ring of size 3 is created from slice object.
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(3, s);

    // Pushing elements at the back of the ring using push_back() function.
    r.push_back(10);
    r.push_back(20);
    r.push_back(30);
    EXPECT_EQ(10, r.at(0));
    // Checks if the ring is full. It returns true, since ring is at maximum capacity.
    EXPECT_TRUE(r.full());

    // Pushing element to overwrite the ring.
    r.push_back(40);

    // Overwritten elements.
    EXPECT_EQ(20, r.at(0));
    EXPECT_EQ(30, r.at(1));
    EXPECT_EQ(40, r.at(2));
    // [EXAMPLE_RING_PUSH_BACK_END]
}
