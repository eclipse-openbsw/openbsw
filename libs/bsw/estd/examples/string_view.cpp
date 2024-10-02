// Copyright 2024 Accenture.

#include "estd/string_view.h"

#include <gtest/gtest.h>

TEST(StringViewExample, assign)
{
    // [EXAMPLE_STRING_VIEW_ASSIGN_START]
    estd::string_view view;
    estd::string_view view2("Hello");

    view = view2;
    ASSERT_EQ('H', view[0]);
    ASSERT_EQ('e', view[1]);
    ASSERT_EQ(5U, view.size());
    // [EXAMPLE_STRING_VIEW_ASSIGN_END]
}

TEST(StringViewExample, swap)
{
    // [EXAMPLE_STRING_VIEW_SWAP_START]
    estd::string_view view  = estd::string_view("Hello");
    estd::string_view view2 = estd::string_view("Jellow");

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
    // [EXAMPLE_STRING_VIEW_SWAP_END]
}

TEST(StringViewExample, tokenize)
{
    // [EXAMPLE_STRING_VIEW_TOKENIZE_START]
    estd::string_view view   = estd::string_view("Hello World how is it going?");
    estd::string_view delims = estd::string_view(" ");
    estd::string_view expected_tokens[]
        = {estd::string_view("Hello"),
           estd::string_view("World"),
           estd::string_view("how"),
           estd::string_view("is"),
           estd::string_view("it"),
           estd::string_view("going?")};

    auto tokens = view.tokenize(delims);
    auto it     = tokens.begin();

    ASSERT_EQ(*it, expected_tokens[0]);
    ++it;
    ASSERT_EQ(*it, expected_tokens[1]);
    ++it;
    ASSERT_EQ(*it, expected_tokens[2]);
    ++it;
    ASSERT_EQ(*it, expected_tokens[3]);
    ++it;
    ASSERT_EQ(*it, expected_tokens[4]);
    ++it;
    ASSERT_EQ(*it, expected_tokens[5]);
    ++it;
    ASSERT_TRUE(it == tokens.end());
    // [EXAMPLE_STRING_VIEW_TOKENIZE_END]
}
