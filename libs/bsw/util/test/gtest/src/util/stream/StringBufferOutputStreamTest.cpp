// Copyright 2024 Accenture.

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

namespace stream = ::util::stream;

TEST(StringBufferOutputStream, testAppendIsSafe)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9));
    cut.write_string_view(::etl::string_view("abc"));
    cut.write_string_view(::etl::string_view("def"));
    cut.write_string_view(::etl::string_view("1234"));
    ASSERT_EQ(0, strcmp("abcdef12", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testAppendOnRawBufferIsSafe)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9));
    cut.write_string_view(::etl::string_view("abc"));
    cut.write_string_view(::etl::string_view("def"));
    cut.write_string_view(::etl::string_view("1234"));
    ASSERT_EQ(0, strcmp("abcdef12", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testEofIsWorkingWithWrite)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(7));
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    ASSERT_FALSE(cut.isEof());
    cut.write_string_view(::etl::string_view("bcde"));
    ASSERT_FALSE(cut.isEof());
    cut.write('E');
    ASSERT_TRUE(cut.isEof());
    cut.write('E');
    ASSERT_TRUE(cut.isEof());
    cut.write('E');
    ASSERT_TRUE(cut.isEof());
    ASSERT_EQ(0, strcmp("abcdeE", cut.getString()));
}

TEST(StringBufferOutputStream, testEofIsWorkingWithWriteBuffer)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(7));
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    ASSERT_FALSE(cut.isEof());
    cut.write_string_view(::etl::string_view("bcd"));
    ASSERT_FALSE(cut.isEof());
    cut.write_string_view(::etl::string_view("1234"));
    ASSERT_TRUE(cut.isEof());
    ASSERT_EQ(0, strcmp("abcd12", cut.getString()));
}

TEST(StringBufferOutputStream, testEolIsAppended)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n");
    cut.write_string_view(::etl::string_view("abcdef1234"));
    ASSERT_EQ(0, strcmp("abcdef1\n", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testEolAndEllipsisLetBufferOverflow)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n");
    cut.write_string_view(::etl::string_view("abcdef12"));
    ASSERT_EQ(0, strcmp("abcdef1\n", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testEolAndEllipsisIsAppended)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n", "..");
    cut.write_string_view(::etl::string_view("abcdef1234"));
    ASSERT_EQ(0, strcmp("abcde..\n", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testEolAndEllipsisIsAppendedOnRawBuffer)
{
    char buffer[10];
    memset(buffer, 0x17, 10);
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n", "..");
    cut.write_string_view(::etl::string_view("abcdef1234"));
    ASSERT_EQ(0, strcmp("abcde..\n", cut.getString()));
    ASSERT_EQ(0x17, buffer[9]);
}

TEST(StringBufferOutputStream, testReset)
{
    char buffer[10];
    stream::StringBufferOutputStream cut(buffer);
    cut.write_string_view(::etl::string_view("abcdef1234"));
    ASSERT_EQ(0, strcmp("abcdef123", cut.getString()));
    cut.reset();
    cut.write_string_view(::etl::string_view("ABCDEFG"));
    ASSERT_EQ(0, strcmp("ABCDEFG", cut.getString()));
}

TEST(StringBufferOutputStream, testGetBufferIfNotFilledCompletely)
{
    char buffer[10];
    stream::StringBufferOutputStream cut(buffer);
    cut.write_string_view(::etl::string_view("abcd"));
    ASSERT_EQ(buffer, cut.getBuffer().data());
    ASSERT_EQ(5U, cut.getBuffer().size());
}

TEST(StringBufferOutputStream, testGetBufferIfFull)
{
    char buffer[10];
    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n", "..");
    cut.write_string_view(::etl::string_view("abcd1234"));
    ASSERT_EQ(buffer, cut.getBuffer().data());
    ASSERT_EQ(9U, cut.getBuffer().size());
}

TEST(StringBufferOutputStream, testMixedUsage)
{
    stream::declare::StringBufferOutputStream<20> cut("E");
    cut.write_string_view(::etl::string_view("abcdef1234:"));
    cut.write('1');
    cut.write('0');
    cut.write_string_view(::etl::string_view("test"));
    cut.write_string_view(::etl::string_view(""));
    ASSERT_EQ("abcdef1234:10testE", std::string(cut.getString()));
}
