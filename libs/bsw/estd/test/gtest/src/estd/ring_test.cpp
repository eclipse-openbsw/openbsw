// Copyright 2024 Accenture.

#include "estd/ring.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

uint8_t mem[1000];

TEST(A_ring, is_initially_empty)
{
    ::estd::slice<uint8_t> s(mem);
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);

    EXPECT_EQ(1000U - (3 * 4 + 5 * 2), s.size());

    EXPECT_TRUE(r.empty());
    EXPECT_EQ(5U, r.length());
}

TEST(A_ring, can_be_pushed_and_popped_a_single_element)
{
    ::estd::slice<uint8_t> s(mem);
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);
    r.push_back(4);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(4, r.front());
    EXPECT_EQ(4, r.at(0));
    r.pop_front();
    EXPECT_TRUE(r.empty());

    r.push_back(5);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(5, r.front());
    EXPECT_EQ(5, r.at(0));
    r.pop_front();
    EXPECT_TRUE(r.empty());
}

TEST(A_ring, can_be_pushed_and_popped_two_elements)
{
    ::estd::slice<uint8_t> s(mem);
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);
    r.push_back(5);
    r.push_back(6);
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(5, r.front());
    EXPECT_EQ(5, r.at(0));
    EXPECT_EQ(6, r.at(1));
    r.pop_front();
    EXPECT_FALSE(r.empty());
    EXPECT_EQ(6, r.front());
    r.pop_front();
    EXPECT_TRUE(r.empty());
}

TEST(A_ring, overwrites_when_full)
{
    ::estd::slice<uint8_t> s(mem);
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);
    r.push_back(0);
    EXPECT_EQ(0, r.front());
    r.push_back(1);
    EXPECT_EQ(0, r.front());
    r.push_back(2);
    EXPECT_EQ(0, r.front());
    r.push_back(3);
    EXPECT_EQ(0, r.front());
    r.push_back(4);
    EXPECT_EQ(0, r.front());
    r.push_back(5);
    EXPECT_EQ(1, r.front());
    r.push_back(6);
    EXPECT_EQ(2, r.front());

    EXPECT_EQ(2, r.at(0));
    EXPECT_EQ(3, r.at(1));
    EXPECT_EQ(4, r.at(2));
    EXPECT_EQ(5, r.at(3));
    EXPECT_EQ(6, r.at(4));

    r.pop_front();
    EXPECT_EQ(3, r.front());
    r.pop_front();
    EXPECT_EQ(4, r.front());
    r.pop_front();
    EXPECT_EQ(5, r.front());
    r.pop_front();
    EXPECT_EQ(6, r.front());

    EXPECT_FALSE(r.empty());
    r.pop_front();
    EXPECT_TRUE(r.empty());
}

TEST(A_ring, gives_correct_used_value)
{
    ::estd::slice<uint8_t> s(mem);
    ::estd::ring<uint16_t>& r = *::estd::ring<uint16_t>::make(5, s);

    EXPECT_EQ(0U, r.used());

    r.push_back(0);
    EXPECT_EQ(1U, r.used());
    r.push_back(1);
    EXPECT_EQ(2U, r.used());
    r.push_back(2);
    EXPECT_EQ(3U, r.used());
    r.push_back(3);
    EXPECT_EQ(4U, r.used());
    r.push_back(4);
    EXPECT_EQ(5U, r.used());
    r.push_back(5);
    EXPECT_EQ(5U, r.used());
    r.push_back(6);
    EXPECT_EQ(5U, r.used());
    r.pop_front();
    EXPECT_EQ(4U, r.used());
    r.pop_front();
    EXPECT_EQ(3U, r.used());
    r.pop_front();
    EXPECT_EQ(2U, r.used());
    r.pop_front();
    EXPECT_EQ(1U, r.used());
    r.pop_front();
    EXPECT_EQ(0U, r.used());
}

} // namespace
