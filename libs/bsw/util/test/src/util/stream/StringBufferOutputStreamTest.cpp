/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

#include <etl/type_traits.h>
#include <memory>

namespace stream = ::util::stream;

namespace
{
/**
 * Verify the existing output-stream interface destroys concrete streams and flushes their buffer.
 */
TEST(StringBufferOutputStreamTest, PublicInterfaceDestruction)
{
    static_assert(::etl::has_virtual_destructor<stream::IOutputStream>::value);
    static_assert(!::etl::is_copy_constructible<stream::StringBufferOutputStream>::value);
    static_assert(!::etl::is_move_constructible<stream::StringBufferOutputStream>::value);

    char buffer[10]{};
    {
        std::unique_ptr<stream::IOutputStream> output
            = std::make_unique<stream::StringBufferOutputStream>(::etl::span<char>(buffer));
        output->write_string_view(::etl::string_view("abc"));
    }
    EXPECT_STREQ("abc", buffer);
}

/**
 * Verify string views preserve embedded nulls and high bytes, including an empty follow-up write.
 */
TEST(StringBufferOutputStreamTest, StringViewPreservesEmbeddedNullAndHighBytes)
{
    char const input[]{'a', '\0', static_cast<char>(0xffU), 'z'};
    char buffer[10]{};
    stream::StringBufferOutputStream output{::etl::span<char>(buffer)};
    output.write_string_view(::etl::string_view(input, sizeof(input)));
    EXPECT_EQ(0, memcmp(input, buffer, sizeof(input)));
    output.write_string_view(::etl::string_view());
    EXPECT_EQ(0, memcmp(input, buffer, sizeof(input)));
}
} // namespace

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

TEST(StringBufferOutputStream, testGetStringWithEmptyBufferLeavesStorageUntouched)
{
    char buffer[1];
    buffer[0] = 0x17;

    {
        stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(0), "\n", "..");
        ASSERT_EQ(buffer, cut.getString());
    }

    ASSERT_EQ(0x17, buffer[0]);
}

TEST(StringBufferOutputStream, testEndOfStringIsTruncatedToFit)
{
    char buffer[5];
    memset(buffer, 0x17, 5);

    stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(4), "ABCDE");

    ASSERT_EQ(0, strcmp("ABC", cut.getString()));
    ASSERT_EQ(0x17, buffer[4]);
}

TEST(StringBufferOutputStream, testEllipsisIsTruncatedToFit)
{
    char buffer[7];
    memset(buffer, 0x17, 7);

    stream::StringBufferOutputStream cut(buffer, "\n", "......");
    cut.write_string_view(::etl::string_view("abcdefgh"));

    ASSERT_EQ(0, strcmp(".....\n", cut.getString()));
}

TEST(StringBufferOutputStream, testEllipsisIsOmittedIfNoSpaceIsAvailable)
{
    char buffer[2];
    memset(buffer, 0x17, 2);

    stream::StringBufferOutputStream cut(buffer, "\n", "..");
    cut.write_string_view(::etl::string_view("ab"));

    ASSERT_EQ(0, strcmp("\n", cut.getString()));
}

TEST(StringBufferOutputStream, testSuffixCanTriggerTruncationWithoutWriteOverflow)
{
    char buffer[6];
    memset(buffer, 0x17, 6);

    stream::StringBufferOutputStream cut(buffer, "\n", "..");
    cut.write_string_view(::etl::string_view("abcde"));

    ASSERT_EQ(0, strcmp("ab..\n", cut.getString()));
}

TEST(StringBufferOutputStream, testDestructorFinalizesBuffer)
{
    char buffer[10];
    memset(buffer, 0x17, 10);

    {
        stream::StringBufferOutputStream cut(::etl::span<char>(buffer).first(9), "\n", "..");
        cut.write_string_view(::etl::string_view("abcdef1234"));
    }

    ASSERT_EQ(0, strcmp("abcde..\n", buffer));
    ASSERT_EQ(0x17, buffer[9]);
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
