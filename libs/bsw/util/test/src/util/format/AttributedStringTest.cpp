// Copyright 2024 Accenture.

#include "util/format/AttributedString.h"

#include <gtest/gtest.h>

using namespace ::util::format;

TEST(AttributedString, checkConstructors)
{
    char const* pText          = "text";
    PlainStringAttributes attr = {Color::BLACK, BLINK | REVERSE, Color::RED};
    {
        AttributedString cut(pText, attr);
        ASSERT_EQ(pText, cut.getString());
        ASSERT_EQ(StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED), cut.getAttributes());
    }
    {
        PlainAttributedString info = {pText, attr};
        AttributedString cut(info);
        ASSERT_EQ(pText, cut.getString());
        ASSERT_EQ(StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED), cut.getAttributes());
    }
}

TEST(StringAttributes, testGetters)
{
    StringAttributes cut(Color::BLACK, BLINK | REVERSE, Color::RED);
    ASSERT_EQ(Color::BLACK, cut.getForegroundColor());
    ASSERT_EQ(BLINK | REVERSE, cut.getFormat());
    ASSERT_EQ(Color::RED, cut.getBackgroundColor());
    ASSERT_EQ(Color::BLACK, cut.getAttributes()._foregroundColor);
    ASSERT_EQ(BLINK | REVERSE, cut.getAttributes()._format);
    ASSERT_EQ(Color::RED, cut.getAttributes()._backgroundColor);
}

TEST(StringAttributes, checkIsAttributed)
{
    ASSERT_FALSE(StringAttributes(Color::DEFAULT_COLOR, 0, Color::DEFAULT_COLOR).isAttributed());
    ASSERT_TRUE(StringAttributes(Color::BLACK, 0, Color::DEFAULT_COLOR).isAttributed());
    ASSERT_TRUE(StringAttributes(Color::DEFAULT_COLOR, BLINK | REVERSE, Color::DEFAULT_COLOR)
                    .isAttributed());
    ASSERT_TRUE(StringAttributes(Color::DEFAULT_COLOR, 0, Color::RED).isAttributed());
}

TEST(StringAttributes, checkComparisonOperators)
{
    ASSERT_TRUE(StringAttributes() == StringAttributes());
    ASSERT_FALSE(StringAttributes() != StringAttributes());
    ASSERT_TRUE(
        StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED)
        == StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_FALSE(
        StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED)
        != StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_FALSE(
        StringAttributes(Color::GREEN, BLINK | REVERSE, Color::RED)
        == StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_TRUE(
        StringAttributes(Color::GREEN, BLINK | REVERSE, Color::RED)
        != StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_FALSE(
        StringAttributes(Color::BLACK, BOLD, Color::RED)
        == StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_TRUE(
        StringAttributes(Color::BLACK, BOLD, Color::RED)
        != StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_FALSE(
        StringAttributes(Color::BLACK, BLINK | REVERSE, Color::YELLOW)
        == StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
    ASSERT_TRUE(
        StringAttributes(Color::BLACK, BLINK | REVERSE, Color::YELLOW)
        != StringAttributes(Color::BLACK, BLINK | REVERSE, Color::RED));
}
