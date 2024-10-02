// Copyright 2024 Accenture.

#include "estd/string_view.h"

#include <gtest/gtest.h>

namespace
{
using namespace ::testing;

TEST(Tokenized_string_view, Tokenize)
{
    ::estd::string_view view   = ::estd::string_view("Hello World how is it going?");
    ::estd::string_view delims = ::estd::string_view(" ");
    ::estd::string_view expected_tokens[]
        = {::estd::string_view("Hello"),
           ::estd::string_view("World"),
           ::estd::string_view("how"),
           ::estd::string_view("is"),
           ::estd::string_view("it"),
           ::estd::string_view("going?")};

    {
        auto tokens   = view.tokenize(delims);
        uint8_t index = 0;

        for (auto token : tokens)
        {
            ASSERT_TRUE(token == expected_tokens[index++]);
        }
        ASSERT_EQ(index, 6);
    }
    {
        auto tokens   = view.tokenize(" ");
        uint8_t index = 0;

        for (auto token : tokens)
        {
            ASSERT_TRUE(token == expected_tokens[index++]);
        }
    }
    {
        auto tokens   = ::estd::string_view("Hello World how is it going?").tokenize(delims);
        uint8_t index = 0;

        for (auto token : tokens)
        {
            ASSERT_TRUE(token == expected_tokens[index++]);
        }
    }
    {
        auto tokens   = ::estd::string_view("Hello World how is it going?").tokenize(" ");
        uint8_t index = 0;

        for (auto token : tokens)
        {
            ASSERT_TRUE(token == expected_tokens[index++]);
        }
    }
}

TEST(Tokenized_string_view, Peek)
{
    ::estd::string_view view   = ::estd::string_view("Hello World how is it going?");
    ::estd::string_view delims = ::estd::string_view(" ");
    ::estd::string_view expected_tokens[]
        = {::estd::string_view("Hello"),
           ::estd::string_view("World"),
           ::estd::string_view("how"),
           ::estd::string_view("is"),
           ::estd::string_view("it"),
           ::estd::string_view("going?")};

    auto tokens   = view.tokenize(delims);
    uint8_t index = 0;

    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        auto peek = it;
        ++peek;
        ASSERT_EQ(*it, expected_tokens[index++]);
        if (index < (sizeof(expected_tokens) / sizeof(expected_tokens[0])))
        {
            ASSERT_EQ(*peek, expected_tokens[index]);
        }
        else
        {
            ASSERT_TRUE((*peek).empty());
        }
    }
    ASSERT_EQ(index, 6);
}

TEST(Tokenized_string_view, TokenizeSimple)
{
    ::estd::string_view view = ::estd::string_view(" Hello Planet #  Earth!");
    auto tokens              = view.tokenize("#! ");

    auto i = tokens.begin();
    std::cout << std::endl << std::endl;
    ASSERT_EQ(0U, (*i).compare("Hello"));
    ++i;
    ASSERT_EQ(0U, (*i).compare("Planet"));
    ++i;
    ASSERT_EQ(0U, (*i).compare("Earth"));
    ++i;
    ASSERT_TRUE(i == tokens.end());
}

TEST(Tokenized_string_view, TokenizeRepeatingDelims)
{
    ::estd::string_view view = ::estd::string_view("Hello!Planet#Earth!");
    auto tokens              = view.tokenize("! ! !");

    auto i = tokens.begin();
    ASSERT_EQ(0U, (*i).compare("Hello"));
    ++i;
    ASSERT_EQ(0U, (*i).compare("Planet#Earth"));
    ++i;
    ASSERT_TRUE(i == tokens.end());
}

TEST(Tokenized_string_view, TokenizeDelimsNotInString)
{
    ::estd::string_view view = ::estd::string_view("Hello Planet Earth!");
    auto tokens              = view.tokenize("@#");

    auto i = tokens.begin();
    ASSERT_EQ(0U, (*i).compare("Hello Planet Earth!"));
    ++i;
    ASSERT_TRUE(i == tokens.end());
}

TEST(Tokenized_string_view, TokenizeDelimsInAndNotInString)
{
    ::estd::string_view view = ::estd::string_view("Hello Planet Earth!");
    auto tokens              = view.tokenize("!@ ");

    auto i = tokens.begin();
    ASSERT_EQ(0U, (*i).compare("Hello"));
    ++i;
    ASSERT_EQ(0U, (*i).compare("Planet"));
    ++i;
    ASSERT_EQ(0U, (*i).compare("Earth"));
    ++i;
    ASSERT_TRUE(i == tokens.end());
}

TEST(Tokenized_string_view, TokenizeEmptyDelims)
{
    ::estd::string_view view = ::estd::string_view("Hello Planet Earth!");
    auto tokens              = view.tokenize("");

    auto it = tokens.begin();
    ASSERT_EQ(0U, (*it).compare("Hello Planet Earth!"));
    ++it;
    ASSERT_TRUE(it == tokens.end());
}

TEST(Tokenized_string_view, TokenizeEmptyString)
{
    ::estd::string_view view = ::estd::string_view("");
    auto tokens              = view.tokenize("#@!");
    ASSERT_FALSE(tokens.begin() != tokens.end());
}

TEST(Tokenized_string_view, TokenizeEmptyStringEmptyDelims)
{
    ::estd::string_view view = ::estd::string_view("");
    auto tokens              = view.tokenize("");
    ASSERT_FALSE(tokens.begin() != tokens.end());
}

} // namespace
