// Copyright 2024 Accenture.

#include "util/stream/ByteBufferOutputStream.h"

#include <etl/memory.h>
#include <etl/span.h>
#include <etl/string_view.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace ::testing;
using namespace ::util::stream;

TEST(ByteBufferOutputStreamTest, isEofReportsTrueForByteBufferSinkWhichIsBasedOnABufferWithSizeZero)
{
    ::etl::span<uint8_t> emptyBuffer;
    ByteBufferOutputStream stream(emptyBuffer);

    EXPECT_THAT(stream.isEof(), Eq(true));
}

TEST(ByteBufferOutputStreamTest, testGetBufferAndPosition)
{
    uint8_t const ARRAY_SIZE = 5U;
    uint8_t buffer[ARRAY_SIZE];

    ByteBufferOutputStream stream(buffer);
    ASSERT_EQ(buffer, stream.getBuffer().data());
    ASSERT_EQ(0U, stream.getBuffer().size());
    ByteBufferOutputStream const& constStream = stream;
    ASSERT_EQ(buffer, constStream.getBuffer().data());
    ASSERT_EQ(0U, constStream.getBuffer().size());

    ASSERT_EQ(0U, stream.getPosition());
    stream.write(1);
    ASSERT_EQ(1U, stream.getPosition());
    stream.write(2);
    ASSERT_EQ(2U, stream.getPosition());
    stream.write(3);
    ASSERT_EQ(3U, stream.getPosition());
    ::etl::span<uint8_t const> readBuffer = stream.getBuffer();
    ASSERT_EQ(3U, readBuffer.size());
    for (size_t idx = 0; idx < readBuffer.size(); ++idx)
    {
        ASSERT_EQ(idx + 1, readBuffer[idx]);
    }
    readBuffer = constStream.getBuffer();
    ASSERT_EQ(3U, readBuffer.size());
    for (size_t idx = 0; idx < readBuffer.size(); ++idx)
    {
        ASSERT_EQ(idx + 1, readBuffer[idx]);
    }
    stream.write(4);
    ASSERT_EQ(4U, stream.getPosition());
    stream.write(5);
    ASSERT_EQ(5U, stream.getPosition());
    stream.write(6);
    ASSERT_EQ(6U, stream.getPosition());
    readBuffer = stream.getBuffer();
    ASSERT_EQ(5U, readBuffer.size());
    for (size_t idx = 0; idx < readBuffer.size(); ++idx)
    {
        ASSERT_EQ(idx + 1, readBuffer[idx]);
    }
    readBuffer = constStream.getBuffer();
    ASSERT_EQ(5U, readBuffer.size());
    for (size_t idx = 0; idx < readBuffer.size(); ++idx)
    {
        ASSERT_EQ(idx + 1, readBuffer[idx]);
    }
}

TEST(ByteBufferOutputStreamTest, isEofReportsFalseAsLongThereIsAtLeastOneByteOfSpaceLeftInTheSink)
{
    uint8_t const ARRAY_SIZE = 10U;
    uint8_t buffer[ARRAY_SIZE];

    ByteBufferOutputStream stream(buffer);

    for (uint8_t index = 0U; index < ARRAY_SIZE; ++index)
    {
        EXPECT_THAT(stream.isEof(), Eq(false));
        stream.write(0xFF);
    }
    EXPECT_THAT(stream.isEof(), Eq(true));
}

TEST(ByteBufferOutputStreamTest, writeWorksCorrectly)
{
    uint8_t const ARRAY_SIZE   = 10U;
    uint8_t buffer[ARRAY_SIZE] = {0};
    uint8_t const bytePattern  = 0x77U;

    ByteBufferOutputStream stream(buffer);

    while (!stream.isEof())
    {
        stream.write(bytePattern);
    }

    EXPECT_THAT(buffer, Each(Eq(bytePattern)));
}

TEST(ByteBufferOutputStreamTest, overflowIsReportedIfWriteAfterEof)
{
    uint8_t const ARRAY_SIZE   = 10U;
    uint8_t buffer[ARRAY_SIZE] = {0};
    uint8_t const bytePattern  = 0x77U;

    ByteBufferOutputStream stream(buffer);

    // run test scenario
    while (!stream.isEof())
    {
        stream.write(bytePattern);
        ASSERT_FALSE(stream.isOverflow());
    }
    stream.write(bytePattern);
    ASSERT_TRUE(stream.isOverflow());
}

TEST(ByteBufferOutputStreamTest, writeBufferWorksCorrectly)
{
    uint8_t array[10];
    ::etl::mem_set(array, sizeof(array), static_cast<uint8_t>(0xAF));
    ByteBufferOutputStream stream(::etl::span<uint8_t>(array).first(6));

    stream.write('T');
    stream.write_string_view(::etl::string_view("abc"));
    stream.write_string_view(::etl::string_view("bcd"));
    stream.write_string_view(::etl::string_view("abc"));
    ASSERT_EQ(10U, stream.getPosition());

    ASSERT_TRUE(stream.isEof());

    EXPECT_THAT(array, ElementsAre('T', 'a', 'b', 'c', 'b', 'c', 0xaf, 0xaf, 0xaf, 0xaf));
}

TEST(ByteBufferOutputStreamTest, skipWorksSkippingAByte)
{
    uint8_t const ARRAY_SIZE = 3U;
    uint8_t buffer[ARRAY_SIZE];
    ::etl::mem_set(buffer, sizeof(buffer), static_cast<uint8_t>(3));
    ByteBufferOutputStream stream(buffer);

    stream.skip(1);
    stream.write(1);
    ASSERT_FALSE(stream.isEof());
    stream.write(2);
    ASSERT_TRUE(stream.isEof());
    EXPECT_THAT(buffer, ElementsAre(3, 1, 2));
}

TEST(ByteBufferOutputStreamTest, skipWorksSkippingExactEofBytes)
{
    uint8_t const ARRAY_SIZE = 3U;
    uint8_t buffer[ARRAY_SIZE];
    ::etl::mem_set(buffer, sizeof(buffer), static_cast<uint8_t>(3));
    ByteBufferOutputStream stream(buffer);

    stream.write(1);
    ASSERT_FALSE(stream.isEof());
    stream.skip(2);
    ASSERT_TRUE(stream.isEof());
    EXPECT_THAT(buffer, ElementsAre(1, 3, 3));
}

TEST(ByteBufferOutputStreamTest, skipWorksSkippingTooManyBytes)
{
    uint8_t const ARRAY_SIZE = 3U;
    uint8_t buffer[ARRAY_SIZE];
    ::etl::mem_set(buffer, sizeof(buffer), static_cast<uint8_t>(3));
    ByteBufferOutputStream stream(buffer);

    stream.write(1);
    ASSERT_FALSE(stream.isEof());
    stream.skip(10);
    ASSERT_TRUE(stream.isEof());
    EXPECT_THAT(buffer, ElementsAre(1, 3, 3));
}
