// Copyright 2024 Accenture.

#include "util/format/Vt100AttributedStringFormatter.h"

#include "util/format/StringWriter.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util;
using namespace ::util::format;

#define ESC "\x1b["

TEST(Vt100AttributedStringFormatterTest, applyZeroLengthCodes)
{
    stream::declare::StringBufferOutputStream<40> stream;
    StringWriter writer(stream);
    Vt100AttributedStringFormatter cut;
    writer.apply(cut.attr(StringAttributes()));
    ASSERT_EQ("", std::string(stream.getString()));
}

TEST(Vt100AttributedStringFormatterTest, applyFormats)
{
    stream::declare::StringBufferOutputStream<40> stream;
    StringWriter writer(stream);
    Vt100AttributedStringFormatter cut;
    writer.apply(cut.reset())
        .write("FF")
        .apply(cut.attr(Color::WHITE, BOLD, Color::RED))
        .write("abcd")
        .apply(cut.attr(StringAttributes(Color::WHITE, BOLD, Color::RED)))
        .write("ABCD")
        .apply(cut.attr(Color::DEFAULT_COLOR, 0, Color::BLACK))
        .write("EE")
        .apply(cut.reset());
    ASSERT_EQ(
        "FF" ESC "97;41;1mabcdABCD" ESC "0m" ESC "40mEE" ESC "0m", std::string(stream.getString()));
}

TEST(Vt100AttributedStringFormatterTest, testWriteWithAllParameters)
{
    stream::declare::StringBufferOutputStream<60> stream;
    StringWriter writer(stream);
    Vt100AttributedStringFormatter cut;
    writer.apply(cut.reset())
        .write("FF")
        .apply(cut.attr(Color::WHITE, BOLD, Color::RED))
        .write("abcd")
        .apply(cut.write("ABCD", Color::DEFAULT_COLOR, 0, Color::BLACK))
        .write("EE")
        .apply(cut.reset());
    ASSERT_EQ(
        "FF" ESC "97;41;1mabcd" ESC "0m" ESC "40mABCD" ESC "0m" ESC "97;41;1mEE" ESC "0m",
        std::string(stream.getString()));
}

TEST(Vt100AttributedStringFormatterTest, testWriteWithStringAndAttributes)
{
    stream::declare::StringBufferOutputStream<60> stream;
    StringWriter writer(stream);
    Vt100AttributedStringFormatter cut;
    writer.apply(cut.reset())
        .write("FF")
        .apply(cut.attr(Color::WHITE, BOLD, Color::RED))
        .write("abcd")
        .apply(cut.write("ABCD", StringAttributes(Color::DEFAULT_COLOR, 0, Color::BLACK)))
        .write("EE")
        .apply(cut.reset());
    ASSERT_EQ(
        "FF" ESC "97;41;1mabcd" ESC "0m" ESC "40mABCD" ESC "0m" ESC "97;41;1mEE" ESC "0m",
        std::string(stream.getString()));
}

TEST(Vt100AttributedStringFormatterTest, testWriteWithSingleParameter)
{
    stream::declare::StringBufferOutputStream<60> stream;
    StringWriter writer(stream);
    Vt100AttributedStringFormatter cut;
    writer.apply(cut.reset())
        .write("FF")
        .apply(cut.attr(Color::WHITE, BOLD, Color::RED))
        .write("abcd")
        .apply(cut.write(
            AttributedString("ABCD", StringAttributes(Color::DEFAULT_COLOR, 0, Color::BLACK))))
        .write("EE")
        .apply(cut.reset());
    ASSERT_EQ(
        "FF" ESC "97;41;1mabcd" ESC "0m" ESC "40mABCD" ESC "0m" ESC "97;41;1mEE" ESC "0m",
        std::string(stream.getString()));
}
