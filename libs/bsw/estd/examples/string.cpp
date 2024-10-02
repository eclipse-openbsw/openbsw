// Copyright 2024 Accenture.

#include "estd/string.h"

#include "estd/assert.h"

#include <gtest/gtest.h>

TEST(StringExample, assign)
{
    // [EXAMPLE_STRING_ASSIGN_START]
    estd::declare::string<10> s;
    s = "Hello";
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
    // [EXAMPLE_STRING_ASSIGN_END]
}

TEST(StringExample, append)
{
    // [EXAMPLE_STRING_APPEND_START]
    estd::declare::string<10> s("Hello");

    s.append("1234", 4);
    ASSERT_EQ("Hello1234", s);
    // [EXAMPLE_STRING_APPEND_END]
}

TEST(StringExample, append_char)
{
    // [EXAMPLE_STRING_APPEND_CHAR_START]
    estd::declare::string<10> s;
    s += "Hello";
    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('\0', s[5]);
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
    // [EXAMPLE_STRING_APPEND_CHAR_END]
}

TEST(StringExample, resize)
{
    // [EXAMPLE_STRING_RESIZE_START]
    estd::declare::string<10> s("Hello");
    ASSERT_EQ(5U, s.size());

    s.resize(7, 'c');
    ASSERT_EQ(7U, s.size());

    ASSERT_EQ('H', s[0]);
    ASSERT_EQ('e', s[1]);
    ASSERT_EQ('l', s[2]);
    ASSERT_EQ('l', s[3]);
    ASSERT_EQ('o', s[4]);
    ASSERT_EQ('c', s[5]);
    ASSERT_EQ('c', s[6]);
    ASSERT_EQ('\0', s[7]);
    // [EXAMPLE_STRING_RESIZE_END]
}

TEST(StringExample, string_equal)
{
    // [EXAMPLE_STRING_EQUAL_START]
    estd::declare::string<10> s("Hello");
    ASSERT_FALSE(s == "ABC");
    ASSERT_TRUE(s == "Hello");
    // [EXAMPLE_STRING_EQUAL_END]
}

TEST(StringExample, length)
{
    // [EXAMPLE_STRING_LENGTH_START]
    estd::declare::string<10> s("Hello");
    ASSERT_EQ(5U, s.length());
    ASSERT_EQ(5U, s.size());
    ASSERT_EQ(10U, s.max_size());
    // [EXAMPLE_STRING_LENGTH_END]
}

TEST(StringExample, strtonum)
{
    // [EXAMPLE_STRING_TO_NUM_START]
    uint8_t const bytes[] = {'0', 'x', 'F'};
    auto const res        = ::estd::strtonum<uint8_t>(bytes, 16);
    EXPECT_EQ(res.get(), 0xFU);
    // [EXAMPLE_STRING_TO_NUM_END]
}
