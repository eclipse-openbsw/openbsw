// Copyright 2024 Accenture.

#include "util/logger/LevelInfo.h"

#include "util/format/AttributedString.h"

#include <gtest/gtest.h>

#include <string>

using namespace ::util::logger;
using namespace ::util::format;

TEST(LoggerLevelInfoTest, testLevelInfo)
{
    {
        LevelInfo cut;
        ASSERT_FALSE(cut.isValid());
    }
    LevelInfo::PlainInfo plainInfo
        = {{"abc",
            {::util::format::Color::YELLOW, ::util::format::BOLD, ::util::format::Color::BLACK}},
           LEVEL_DEBUG};
    {
        LevelInfo cut(&plainInfo);
        ASSERT_TRUE(cut.isValid());
        ASSERT_EQ(LEVEL_DEBUG, cut.getLevel());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        LevelInfo src(&plainInfo);
        LevelInfo cut = src;
        ASSERT_EQ(LEVEL_DEBUG, cut.getLevel());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        LevelInfo src(&plainInfo);
        LevelInfo cut;
        cut = src;
        ASSERT_EQ(LEVEL_DEBUG, cut.getLevel());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        LevelInfo cut(&plainInfo);
        cut = *(&cut);
        ASSERT_EQ(LEVEL_DEBUG, cut.getLevel());
        ASSERT_EQ("abc", std::string(cut.getName().getString()));
        ASSERT_EQ(
            StringAttributes(Color::YELLOW, BOLD, Color::BLACK), cut.getName().getAttributes());
    }
    {
        LevelInfo cut(&plainInfo);
        ASSERT_TRUE(cut.isValid());
        ASSERT_STREQ(plainInfo._nameInfo._string, cut.getPlainInfoString());
    }
}

TEST(LoggerLevelInfoTest, LevelInfoInvalidPlainInfoCase)
{
    LevelInfo cut(nullptr);
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    {
        ASSERT_FALSE(cut.isValid());
    }
    {
        EXPECT_THROW(cut.getName(), ::estd::assert_exception);
    }
    {
        EXPECT_THROW(cut.getLevel(), ::estd::assert_exception);
    }
    {
        EXPECT_THROW(cut.getPlainInfoString(), ::estd::assert_exception);
    }
}
