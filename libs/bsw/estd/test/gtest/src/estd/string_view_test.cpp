// Copyright 2024 Accenture.

#include "estd/string_view.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(String_view, Size)
{
    ::estd::string_view view;

    ASSERT_EQ(0U, view.size());
    ASSERT_EQ(0U, view.length());

    view = ::estd::string_view("hello");

    ASSERT_EQ(5U, view.size());
    ASSERT_EQ(5U, view.max_size());
    ASSERT_EQ(5U, view.length());
}

TEST(String_view, ConstructorFromConstChar)
{
    ::estd::string_view view("Hello");

    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ('\0', view[5]);
    ASSERT_EQ(5U, view.size());
    ASSERT_EQ(5U, view.max_size());
}

TEST(String_view, ConstructorFromConstCharCount)
{
    ::estd::string_view view("Hello", 3);

    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ('l', view[3]);
    ASSERT_EQ(3U, view.size());
}

TEST(String_view, ConstructorCopy)
{
    ::estd::string_view view("Hello");
    ::estd::string_view view2(view);

    ASSERT_EQ(view[0], view2[0]);
    ASSERT_EQ(view[1], view2[1]);
    ASSERT_EQ(view[5], view2[5]);
    ASSERT_EQ(view.size(), view2.size());

    static_assert(
        std::is_constructible<::estd::string_view, ::std::nullptr_t>::value == false,
        "string_view should not be constructible from a nullptr.");
    static_assert(
        std::is_constructible<::estd::string_view, ::std::nullptr_t, size_t>::value == false,
        "string_view should not be constructible from a nullptr.");
}

TEST(String_view, AssignmentStringView)
{
    ::estd::string_view view;
    ::estd::string_view view2("Hello");

    view = view2;
    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ('\0', view[5]);
    ASSERT_EQ(5U, view.size());
}

TEST(String_view, AssignmentCharConst)
{
    ::estd::string_view view;

    view = ::estd::string_view("Hello");
    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ('\0', view[5]);
    ASSERT_EQ(5U, view.size());
}

/***
 * Operator Missing here
 */

TEST(String_view, OperatorIndex)
{
    char const* str          = "Hello";
    ::estd::string_view view = ::estd::string_view("Hello");

    ASSERT_EQ(str[0], view[0]);
    ASSERT_EQ(str[3], view[3]);
    ASSERT_EQ(5U, view.size());
}

TEST(String_view, At)
{
    char const* str          = "Hello";
    ::estd::string_view view = ::estd::string_view(str);

    ASSERT_EQ(str[0], view.at(0));
    ASSERT_EQ(str[3], view.at(3));

    ASSERT_EQ(5U, view.size());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.at(10), ::estd::assert_exception);
}

TEST(String_view, Front)
{
    ::estd::string_view view = ::estd::string_view("Hello");

    ASSERT_EQ('H', view.front());
    ASSERT_EQ(5U, view.size());
}

TEST(String_view, Back)
{
    ::estd::string_view view = ::estd::string_view("Hello");

    ASSERT_EQ('o', view.back());
    ASSERT_EQ(5U, view.size());
}

TEST(String_view, Empty)
{
    ::estd::string_view view;

    ASSERT_TRUE(view.empty());
    view = ::estd::string_view("Hello");
    ASSERT_TRUE(!view.empty());
    ASSERT_EQ(5U, view.size());
}

TEST(String_view, RemovePrefix)
{
    ::estd::string_view view = ::estd::string_view("Hello");

    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);

    ASSERT_EQ(5U, view.size());
    view.remove_prefix(2);

    ASSERT_EQ('l', view[0]);
    ASSERT_EQ('l', view[1]);

    ASSERT_EQ(3U, view.size());

    view.remove_prefix(0);
    ASSERT_EQ(3U, view.size());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.remove_prefix(10), ::estd::assert_exception);
}

TEST(String_view, RemoveSuffix)
{
    ::estd::string_view view = ::estd::string_view("Hello");

    ASSERT_EQ('l', view[3]);
    ASSERT_EQ('o', view[4]);

    ASSERT_EQ(5U, view.size());
    view.remove_suffix(2);

    ASSERT_TRUE(view == ::estd::string_view("Hel"));

    ASSERT_EQ(3U, view.size());

    view.remove_suffix(0);
    ASSERT_EQ(3U, view.size());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.remove_suffix(10), ::estd::assert_exception);
}

TEST(String_view, Compare)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = ::estd::string_view("Hello");

    ASSERT_EQ(view.compare(view2), 0);
    ASSERT_EQ(view2.compare(view), 0);

    view = ::estd::string_view("Iello");
    ASSERT_EQ(view[0], 'I');
    ASSERT_GT(view.compare(view2), 0);
    ASSERT_LT(view2.compare(view), 0);

    view = ::estd::string_view("Hello");
    ASSERT_EQ(view.compare(0, ::estd::string_view::npos, view2), 0);
    ASSERT_EQ(view2.compare(0, ::estd::string_view::npos, view), 0);
    view = ::estd::string_view("Iello");
    ASSERT_EQ(view.compare(1, ::estd::string_view::npos, view2), 29);
    ASSERT_GT(view.compare(view2), 0);
    ASSERT_LT(view2.compare(view), 0);
    view = ::estd::string_view("Helmo");
    ASSERT_EQ(view.compare(0, 3, view2), -2);
    ASSERT_GT(view.compare(view2), 0);
    ASSERT_LT(view2.compare(view), 0);

    view = ::estd::string_view("Hello");
    ASSERT_EQ(view.compare(0, ::estd::string_view::npos, view2, 0, ::estd::string_view::npos), 0);
    ASSERT_EQ(view2.compare(0, ::estd::string_view::npos, view, 0, ::estd::string_view::npos), 0);
    ASSERT_GT(view.compare(1, ::estd::string_view::npos, view2, 0, ::estd::string_view::npos), 0);
    ASSERT_LT(view2.compare(0, ::estd::string_view::npos, view, 1, ::estd::string_view::npos), 0);

    // Different length
    ::estd::string_view longer  = ::estd::string_view("Hello world");
    ::estd::string_view shorter = ::estd::string_view("He");

    ASSERT_EQ(view.compare(longer), -6);
    ASSERT_EQ(view.compare(shorter), 3);

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.compare(10, 10, view2), ::estd::assert_exception);
}

TEST(String_view, Swap)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = ::estd::string_view("Jellow");

    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ(5U, view.size());

    ASSERT_EQ('J', view2[0]);
    ASSERT_EQ('e', view2[1]);
    ASSERT_EQ(6U, view2.size());

    view.swap(view2);

    ASSERT_EQ('H', view2[0]);
    ASSERT_EQ('e', view2[1]);
    ASSERT_EQ(5U, view2.size());

    ASSERT_EQ('J', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ(6U, view.size());
}

TEST(String_view, Copy)
{
    ::estd::string_view view = ::estd::string_view("Hello");
    constexpr size_t SIZE    = 100;
    char test_pointer[SIZE];
    (void)memset(test_pointer, 0, SIZE);

    ASSERT_EQ(0, test_pointer[0]);
    ASSERT_EQ(0, test_pointer[1]);

    ASSERT_EQ(view.copy(test_pointer, 5), 5);
    ASSERT_EQ('H', test_pointer[0]);
    ASSERT_EQ('e', test_pointer[1]);
    ASSERT_EQ('\0', test_pointer[5]);

    ASSERT_EQ(view.copy(test_pointer, 5, 2), 3);
    ASSERT_EQ('l', test_pointer[0]);
    ASSERT_EQ('l', test_pointer[1]);
    ASSERT_EQ('o', test_pointer[2]);

    ASSERT_EQ(view.copy(nullptr, 4, 3), 0);

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.copy(test_pointer, 10, 10), ::estd::assert_exception);

    ASSERT_EQ(5U, view.size());
}

TEST(String_view, Substr)
{
    ::estd::string_view view = ::estd::string_view("Hello");

    auto substr = view.substr();
    ASSERT_EQ('H', substr[0]);
    ASSERT_EQ('e', substr[1]);
    ASSERT_EQ('\0', substr[5]);
    ASSERT_EQ(5U, substr.size());

    auto substr2 = view.substr(3);
    ASSERT_EQ('l', substr2[0]);
    ASSERT_EQ('o', substr2[1]);
    ASSERT_EQ('\0', substr2[2]);
    ASSERT_EQ(2U, substr2.size());

    auto substr3 = ::estd::string_view("Hello World what is going on?").substr(6, 5);

    ASSERT_EQ('W', substr3[0]);
    ASSERT_EQ('o', substr3[1]);
    ASSERT_EQ('d', substr3[4]);
    ASSERT_EQ(5U, substr3.size());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.substr(10), ::estd::assert_exception);

    ASSERT_EQ(5U, view.size());
}

TEST(String_view, StartsWith)
{
    ::estd::string_view view  = ::estd::string_view("Hello World how is it going?");
    ::estd::string_view view2 = ::estd::string_view("Hello");
    ::estd::string_view view3 = ::estd::string_view("Iello");

    ASSERT_TRUE(view.starts_with(view2));
    ASSERT_FALSE(view.starts_with(view3));

    ASSERT_TRUE(view.starts_with('H'));
    ASSERT_FALSE(view.starts_with('h'));

    ASSERT_TRUE(view.starts_with("Hello"));
    ASSERT_FALSE(view.starts_with("hello"));

    ASSERT_EQ(28U, view.size());
}

TEST(String_view, EndsWith)
{
    ::estd::string_view view  = ::estd::string_view("Hello World how is it going?");
    ::estd::string_view view2 = ::estd::string_view("going?");
    ::estd::string_view view3 = ::estd::string_view("goinG?");

    ASSERT_TRUE(view.ends_with(view2));
    ASSERT_FALSE(view.ends_with(view3));

    ASSERT_TRUE(view.ends_with('?'));
    ASSERT_FALSE(view.ends_with('!'));

    ASSERT_TRUE(view.ends_with("going?"));
    ASSERT_FALSE(view.ends_with("Ng?"));

    ASSERT_EQ(28U, view.size());
}

TEST(String_view, Find)
{
    ::estd::string_view view       = ::estd::string_view("Hello World how is it going?");
    ::estd::string_view view2      = ::estd::string_view("World");
    ::estd::string_view empty_view = ::estd::string_view("");

    ASSERT_EQ(view.find(view2), 6);
    ASSERT_TRUE(view.find(view2, 8) == ::estd::string_view::npos);

    ASSERT_TRUE(empty_view.find('W') == ::estd::string_view::npos);
    ASSERT_EQ(view.find('W'), 6);
    ASSERT_TRUE(view.find('W', 8) == ::estd::string_view::npos);
    ASSERT_TRUE(view.find('X') == ::estd::string_view::npos);
    ASSERT_EQ(view2.find('d'), 4);

    ASSERT_TRUE(empty_view.find("") == 0);
    ASSERT_TRUE(empty_view.find("World") == ::estd::string_view::npos);
    ASSERT_EQ(view.find("World"), 6);
    ASSERT_TRUE(view.find("World", 8) == ::estd::string_view::npos);
    ASSERT_TRUE(view.find("YEEET") == ::estd::string_view::npos);
    ASSERT_TRUE(view.find("") == 0); // feels weird but follows gcc implementation of string_view

    ASSERT_EQ(view.find("World_Hunger", 0, 5), 6);
    ASSERT_TRUE(view.find("World_Hunger") == ::estd::string_view::npos);
    ASSERT_EQ(view.find("World_Hunger", 2, 5), 6);
    ASSERT_TRUE(view.find("World_Hunger", 8, 5) == ::estd::string_view::npos);

    ASSERT_EQ(view.find_first_of("o "), 4);
    ASSERT_EQ(view.find_first_of(" o"), 4);
    ASSERT_EQ(view.find_first_of("o ", 5), 5);

    ASSERT_TRUE(view.find_first_of("") == ::estd::string_view::npos);
    ASSERT_TRUE(::estd::string_view("").find_first_of("") == ::estd::string_view::npos);

    ASSERT_EQ(view.find_first_not_of("o "), 0);
    ASSERT_EQ(view.find_first_not_of(" o", 4), 6);
    ASSERT_EQ(view.find_first_not_of("o ", 7), 8);

    ASSERT_TRUE(view.find_first_not_of("") == 0);
    ASSERT_TRUE(::estd::string_view("").find_first_not_of("") == ::estd::string_view::npos);

    ASSERT_EQ(28U, view.size());

    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    ASSERT_THROW(view.find("10", 100), ::estd::assert_exception);
}

TEST(String_view, skip_while_matches)
{
    ::estd::string_view view = ::estd::string_view("aaabc abbc");
    // returned reference
    ASSERT_EQ(view.skip_while_matches("a"), ::estd::string_view("bc abbc"));
    // original is mutated
    ASSERT_EQ(view, ::estd::string_view("bc abbc"));
    view = ::estd::string_view("aaabc abbc");

    // multiple times same character
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_while_matches("aa"), ::estd::string_view("bc abbc"));

    // ab == ba
    ::estd::string_view view1 = ::estd::string_view("aaabc abbc");
    ::estd::string_view view2 = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view1.skip_while_matches("ab"), ::estd::string_view("c abbc"));
    ASSERT_EQ(view2.skip_while_matches("ba"), ::estd::string_view("c abbc"));
    ASSERT_EQ(view1, view2);

    // match all
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_while_matches("abc "), ::estd::string_view());

    // match none
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_while_matches("x"), ::estd::string_view("aaabc abbc"));

    // "" match none
    ASSERT_EQ(view.skip_while_matches(""), ::estd::string_view("aaabc abbc"));
}

TEST(String_view, skip_until_matches)
{
    ::estd::string_view view = ::estd::string_view("aaabc abbc");

    // returned reference
    ASSERT_EQ(view.skip_until_matches("a"), ::estd::string_view("aaabc abbc"));
    ASSERT_EQ(view.skip_until_matches("b"), ::estd::string_view("bc abbc"));
    // original is mutated
    ASSERT_EQ(view, ::estd::string_view("bc abbc"));

    // ab == ba
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_until_matches("ab"), ::estd::string_view("aaabc abbc"));
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_until_matches("ba"), ::estd::string_view("aaabc abbc"));

    // matches at last char (prevent off-by-one errors)
    view = ::estd::string_view("aaabc abbcd");
    ASSERT_EQ(view.skip_until_matches("d"), ::estd::string_view("d"));

    // no match
    ASSERT_EQ(view.skip_until_matches("x"), ::estd::string_view());

    // "" never matches
    view = ::estd::string_view("aaabc abbc");
    ASSERT_EQ(view.skip_until_matches(""), ::estd::string_view(""));
}

TEST(String_view, OperatorEquals)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = view;

    ASSERT_TRUE(view == view2);
    ASSERT_TRUE(view2 == view);
    view2 = ::estd::string_view("bruh");
    ASSERT_FALSE(view2 == view);

    ASSERT_EQ(5U, view.size());
}

TEST(String_view, OperatorNotEquals)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = view;

    view2 = ::estd::string_view("bruh");
    ASSERT_TRUE(view != view2);
    ASSERT_TRUE(view2 != view);
    view2 = view;
    ASSERT_FALSE(view2 != view);

    ASSERT_EQ(5U, view.size());
}

TEST(String_view, OperatorGreaterThan)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = view;

    view2 = ::estd::string_view("Iello");
    ASSERT_FALSE(view > view2);
    ASSERT_TRUE(view2 > view);

    ASSERT_EQ(5U, view.size());
}

TEST(String_view, OperatorSmallerThan)
{
    ::estd::string_view view  = ::estd::string_view("Hello");
    ::estd::string_view view2 = view;

    view2 = ::estd::string_view("Iello");
    ASSERT_TRUE(view < view2);
    ASSERT_FALSE(view2 < view);

    ASSERT_EQ(5U, view.size());
}
} // namespace
