// Copyright 2024 Accenture.

#include "util/string/ConstString.h"

#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::util::string;

TEST(ConstString, testConstructors)
{
    {
        ConstString cut;
        ASSERT_EQ(0U, cut.length());
        ASSERT_EQ(nullptr, cut.data());
        ASSERT_TRUE(cut.isEmpty());
    }
    {
        char buffer[10];
        ConstString cut(buffer, 0);
        ASSERT_EQ(0U, cut.length());
        ASSERT_EQ(buffer, cut.data());
        ASSERT_TRUE(cut.isEmpty());
    }
    {
        ConstString cut(nullptr, 10);
        ASSERT_EQ(0U, cut.length());
        ASSERT_EQ(nullptr, cut.data());
        ASSERT_TRUE(cut.isEmpty());
    }
    {
        char const* string("abcd");
        ConstString cut(string);
        ASSERT_EQ(4U, cut.length());
        ASSERT_EQ(string, cut.data());
        ASSERT_FALSE(cut.isEmpty());
    }
    {
        char const* string = "abcd";
        ConstString src(string);
        ConstString cut(src);
        ASSERT_EQ(4U, cut.length());
        ASSERT_EQ(string, cut.data());
        ASSERT_FALSE(cut.isEmpty());
    }
    {
        ConstString cut(nullptr);
        ASSERT_EQ(0U, cut.length());
        ASSERT_EQ(nullptr, cut.data());
        ASSERT_TRUE(cut.isEmpty());
    }
}

TEST(ConstString, testAssignmentOperator)
{
    {
        char const* string = "abcd";
        ConstString src(string);
        ConstString cut;
        cut = src;
        ASSERT_EQ(4U, cut.length());
        ASSERT_EQ(string, cut.data());
    }
    {
        char const* string = "abcd";
        ConstString cut(string);
        cut = *(&cut);
        ASSERT_EQ(4U, cut.length());
        ASSERT_EQ(string, cut.data());
    }
}

TEST(ConstString, contains_returns_false_if_the_specified_string_isnt_part_of_the_const_string)
{
    ConstString text("abcdefg");
    ConstString searchterm("h");
    EXPECT_THAT(text.contains(searchterm), ::testing::Eq(false));
}

TEST(ConstString, contains_returns_true_if_the_specified_string_is_part_of_the_const_string)
{
    ConstString text("abcdefg");
    ConstString searchterm("f");
    EXPECT_THAT(text.contains(searchterm), ::testing::Eq(true));
}

TEST(
    ConstString,
    find_returns_the_offset_of_the_first_occurrence_if_the_string_was_found_in_the_const_string)
{
    ConstString text("fabbfacdefag");
    ConstString searchterm("fa");
    int32_t offset = 0;
    offset         = text.find(searchterm, offset);
    EXPECT_THAT(offset, ::testing::Eq(0));
    offset = text.find(searchterm, offset + 1);
    EXPECT_THAT(offset, ::testing::Eq(4));
    offset = text.find(searchterm, offset + 1);
    EXPECT_THAT(offset, ::testing::Eq(9));
}

TEST(ConstString, find_returns_not_found_because_search_string_is_longer_than_the_searched_string)
{
    ConstString text("fab");
    ConstString searchterm("fabfcdefg");
    EXPECT_THAT(text.find(searchterm, 0), ::testing::Lt(0));
}

TEST(ConstString, find_returns_a_netgative_value_if_the_string_was_not_found_in_the_const_string)
{
    ConstString text("fabfcdefg");
    ConstString searchterm("z");
    int32_t offset = 0;
    offset         = text.find(searchterm, offset);
    EXPECT_THAT(offset, ::testing::Lt(0));
}

TEST(ConstString, find_returns_a_zero_offset_if_the_searched_substring_is_equal_to_const_string)
{
    ConstString text("abc");
    ConstString searchterm("abc");
    int32_t const offset = text.find(searchterm, 0U);
    EXPECT_EQ(offset, 0);
}

TEST(ConstString, testSubscriptionOperator)
{
    {
        ConstString cut("abcd");
        ASSERT_EQ('a', cut[0]);
        ASSERT_EQ('b', cut[1]);
        ASSERT_EQ('c', cut[2]);
        ASSERT_EQ('d', cut[3]);
        ASSERT_THROW({ cut[4]; }, ::etl::exception);
    }
    {
        ConstString cut;
        ASSERT_THROW({ cut[0]; }, ::etl::exception);
    }
}

TEST(ConstString, testComparisonOperators)
{
    ASSERT_EQ(ConstString().compare(ConstString()), 0);
    ASSERT_EQ(ConstString("abcd").compare(ConstString("abcd")), 0);

    ASSERT_LT(ConstString("abcd").compare(ConstString("abce")), 0);
    ASSERT_GT(ConstString("abce").compare(ConstString("abcd")), 0);

    ASSERT_LT(ConstString("abcd").compare(ConstString("abcde")), 0);
    ASSERT_GT(ConstString("abcde").compare(ConstString("abcd")), 0);

    ASSERT_GT(ConstString("abcde").compare(ConstString("abcd")), 0);

    ASSERT_EQ(ConstString().compare(ConstString(), false), 0);
    ASSERT_EQ(ConstString("abcd").compare(ConstString("abcd"), false), 0);

    ASSERT_LT(ConstString("abcd").compare(ConstString("abce"), false), 0);
    ASSERT_GT(ConstString("abce").compare(ConstString("abcd"), false), 0);

    ASSERT_LT(ConstString("abcd").compare(ConstString("abcde"), false), 0);
    ASSERT_GT(ConstString("abcde").compare(ConstString("abcd"), false), 0);

    ASSERT_GT(ConstString("abcde").compare(ConstString("abcd"), false), 0);

    ASSERT_EQ(ConstString().compareIgnoreCase(ConstString()), 0);
    ASSERT_EQ(ConstString(".abcd}").compareIgnoreCase(ConstString(".ABCD}")), 0);
    ASSERT_EQ(ConstString(".ABCD}").compareIgnoreCase(ConstString(".abcd}")), 0);

    ASSERT_LT(ConstString(".ABCD}").compareIgnoreCase(ConstString(".abce}")), 0);
    ASSERT_LT(ConstString(".abcd}").compareIgnoreCase(ConstString(".ABCE}")), 0);
    ASSERT_GT(ConstString(".ABCE}").compareIgnoreCase(ConstString(".abcd}")), 0);
    ASSERT_GT(ConstString(".abce}").compareIgnoreCase(ConstString(".ABCD}")), 0);

    ASSERT_LT(ConstString(".ABCD").compareIgnoreCase(ConstString(".abcde")), 0);
    ASSERT_LT(ConstString(".abcd").compareIgnoreCase(ConstString(".ABCDE")), 0);
    ASSERT_GT(ConstString(".ABCDE").compareIgnoreCase(ConstString(".abcd")), 0);
    ASSERT_GT(ConstString(".abcde").compareIgnoreCase(ConstString(".ABCD")), 0);

    ASSERT_GT(ConstString(".ABCDE}").compareIgnoreCase(ConstString(".abcde")), 0);
    ASSERT_GT(ConstString(".abcde}").compareIgnoreCase(ConstString(".ABCDe")), 0);

    ASSERT_EQ(ConstString().compare(ConstString(), true), 0);
    ASSERT_EQ(ConstString(".abcd}").compare(ConstString(".ABCD}"), true), 0);
    ASSERT_EQ(ConstString(".ABCD}").compare(ConstString(".abcd}"), true), 0);

    ASSERT_LT(ConstString(".ABCD}").compare(ConstString(".abce}"), true), 0);
    ASSERT_LT(ConstString(".abcd}").compare(ConstString(".ABCE}"), true), 0);
    ASSERT_GT(ConstString(".ABCE}").compare(ConstString(".abcd}"), true), 0);
    ASSERT_GT(ConstString(".abce}").compare(ConstString(".ABCD}"), true), 0);

    ASSERT_LT(ConstString(".ABCD").compare(ConstString(".abcde"), true), 0);
    ASSERT_LT(ConstString(".abcd").compare(ConstString(".ABCDE"), true), 0);
    ASSERT_GT(ConstString(".ABCDE").compare(ConstString(".abcd"), true), 0);
    ASSERT_GT(ConstString(".abcde").compare(ConstString(".ABCD"), true), 0);

    ASSERT_GT(ConstString(".ABCDE}").compare(ConstString(".abcde"), true), 0);
    ASSERT_GT(ConstString(".abcde}").compare(ConstString(".ABCDe"), true), 0);

    ASSERT_TRUE(ConstString() == ConstString());
    ASSERT_TRUE(ConstString("abc") == ConstString("abc"));
    ASSERT_FALSE(ConstString() == ConstString("abc"));
    ASSERT_FALSE(ConstString("abc") == ConstString());
    ASSERT_FALSE(ConstString("abc") == ConstString("abcdef"));
    ASSERT_FALSE(ConstString("abcdef") == ConstString("abc"));

    ASSERT_FALSE(ConstString() != ConstString());
    ASSERT_FALSE(ConstString("abc") != ConstString("abc"));
    ASSERT_TRUE(ConstString() != ConstString("abc"));
    ASSERT_TRUE(ConstString("abc") != ConstString());
    ASSERT_TRUE(ConstString("abc") != ConstString("abcdef"));
    ASSERT_TRUE(ConstString("abcdef") != ConstString("abc"));

    ASSERT_FALSE(ConstString() < ConstString());
    ASSERT_FALSE(ConstString("abc") < ConstString("abc"));
    ASSERT_TRUE(ConstString() < ConstString("abc"));
    ASSERT_FALSE(ConstString("abc") < ConstString());
    ASSERT_TRUE(ConstString("abc") < ConstString("abcdef"));
    ASSERT_FALSE(ConstString("abcdef") < ConstString("abc"));

    ASSERT_TRUE(ConstString() <= ConstString());
    ASSERT_TRUE(ConstString("abc") <= ConstString("abc"));
    ASSERT_TRUE(ConstString() <= ConstString("abc"));
    ASSERT_FALSE(ConstString("abc") <= ConstString());
    ASSERT_TRUE(ConstString("abc") <= ConstString("abcdef"));
    ASSERT_FALSE(ConstString("abcdef") <= ConstString("abc"));

    ASSERT_FALSE(ConstString() > ConstString());
    ASSERT_FALSE(ConstString("abc") > ConstString("abc"));
    ASSERT_FALSE(ConstString() > ConstString("abc"));
    ASSERT_TRUE(ConstString("abc") > ConstString());
    ASSERT_FALSE(ConstString("abc") > ConstString("abcdef"));
    ASSERT_TRUE(ConstString("abcdef") > ConstString("abc"));

    ASSERT_TRUE(ConstString() >= ConstString());
    ASSERT_TRUE(ConstString("abc") >= ConstString("abc"));
    ASSERT_FALSE(ConstString() >= ConstString("abc"));
    ASSERT_TRUE(ConstString("abc") >= ConstString());
    ASSERT_FALSE(ConstString("abc") >= ConstString("abcdef"));
    ASSERT_TRUE(ConstString("abcdef") >= ConstString("abc"));
}
