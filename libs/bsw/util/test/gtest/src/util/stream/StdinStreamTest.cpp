// Copyright 2024 Accenture.

#include "util/stream/StdinStream.h"

#include "util/StdIoMock.h"

#include <etl/string_view.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
namespace stream = ::util::stream;

struct StdinStreamTest : Test
{
    util::test::StdIoMock stdIo;
};

TEST_F(StdinStreamTest, testAllCharactersAreRead)
{
    uint8_t bytes[10] = {0};
    bytes[7]          = 0xaf;

    ::etl::string_view view("abcdefg");
    stdIo.in
        = ::etl::span<uint8_t const>(reinterpret_cast<uint8_t const*>(view.begin()), view.size());

    ::etl::span<uint8_t> buffer(bytes);
    stream::StdinStream cut;

    EXPECT_FALSE(cut.isEof());

    ASSERT_EQ(7U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer.first(7), ElementsAre('a', 'b', 'c', 'd', 'e', 'f', 'g'));
    EXPECT_EQ(0xaf, bytes[7]);
    EXPECT_FALSE(cut.isEof());
}

TEST_F(StdinStreamTest, testBufferTooSmallForAllChars)
{
    uint8_t bytes[4] = {0};

    ::etl::string_view view("abcdefg");
    stdIo.in
        = ::etl::span<uint8_t const>(reinterpret_cast<uint8_t const*>(view.begin()), view.size());

    ::etl::span<uint8_t> buffer(bytes);
    stream::StdinStream cut;

    EXPECT_FALSE(cut.isEof());

    ASSERT_EQ(4U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer, ElementsAre('a', 'b', 'c', 'd'));

    ASSERT_EQ(3U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer.first(3), ElementsAre('e', 'f', 'g'));

    ASSERT_EQ(0U, cut.readBuffer(buffer));
    EXPECT_FALSE(cut.isEof());
}
