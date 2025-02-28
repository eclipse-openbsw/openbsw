// Copyright 2024 Accenture.

#include "util/stream/StdinStream.h"

#include "util/StdIoMock.h"

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

    stdIo.in = ::estd::make_str("abcdefg");

    ::estd::slice<uint8_t> buffer(bytes);
    stream::StdinStream cut;

    EXPECT_FALSE(cut.isEof());

    ASSERT_EQ(7U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer.subslice(7), ElementsAre('a', 'b', 'c', 'd', 'e', 'f', 'g'));
    EXPECT_EQ(0xaf, bytes[7]);
    EXPECT_FALSE(cut.isEof());
}

TEST_F(StdinStreamTest, testBufferTooSmallForAllChars)
{
    uint8_t bytes[4] = {0};

    stdIo.in = ::estd::make_str("abcdefg");

    ::estd::slice<uint8_t> buffer(bytes);
    stream::StdinStream cut;

    EXPECT_FALSE(cut.isEof());

    ASSERT_EQ(4U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer, ElementsAre('a', 'b', 'c', 'd'));

    ASSERT_EQ(3U, cut.readBuffer(buffer));
    EXPECT_THAT(buffer.subslice(3), ElementsAre('e', 'f', 'g'));

    ASSERT_EQ(0U, cut.readBuffer(buffer));
    EXPECT_FALSE(cut.isEof());
}
