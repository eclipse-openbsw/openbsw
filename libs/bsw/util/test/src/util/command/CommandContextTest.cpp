// Copyright 2024 Accenture.

#include "util/command/CommandContext.h"

#include "util/format/SharedStringWriter.h"
#include "util/stream/SharedOutputStream.h"
#include "util/stream/StringBufferOutputStream.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

using namespace ::util::command;
using namespace ::util::string;
using namespace ::util::stream;

namespace
{
struct ContinuousUserMock : ISharedOutputStream::IContinuousUser
{
    MOCK_METHOD(void, endContinuousOutput, (IOutputStream & stream));
};
} // namespace

struct CommandContextTest : Test
{
    StrictMock<ContinuousUserMock> user;
};

TEST_F(CommandContextTest, testScanTokens)
{
    {
        CommandContext cut(ConstString(" \r\n\t "));
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  /test  "));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("/test"), cut.scanToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  /test"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("/test"), cut.scanToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_EQ(ConstString(), cut.scanToken());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ConstString(), cut.scanToken());
        ASSERT_EQ(ConstString(""), cut.getSuffix());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  /test 734U"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("/test"), cut.scanToken());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("734U"), cut.scanToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  abc_093  "));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("abc_093"), cut.scanIdentifierToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  abc{  "));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString(""), cut.scanIdentifierToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_EQ(ConstString("{  "), cut.getSuffix());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  1ab  "));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString(""), cut.scanIdentifierToken());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_EQ(ConstString("1ab  "), cut.getSuffix());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" test unknown"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(
            12U,
            cut.scanEnumToken<uint32_t>()
                .check("abc", 1)
                .check("test", 12)
                .check("test", 13)
                .getValue());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("unknown"), cut.getSuffix());
        ASSERT_EQ(
            0U,
            cut.scanEnumToken<uint32_t>()
                .check("abc", 1)
                .check("test", 12)
                .check("test", 13)
                .getValue());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ConstString("unknown"), cut.getSuffix());
        ASSERT_EQ(ICommand::Result::BAD_VALUE, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  0"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(0U, cut.scanIntToken<uint32_t>());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  +12234  "));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(12234U, cut.scanIntToken<uint32_t>());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("  -3473  0x3438 0Xabc 0xF01 023 0x34t"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(-3473, cut.scanIntToken<int32_t>());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("0x3438 0Xabc 0xF01 023 0x34t"), cut.getSuffix());
        ASSERT_EQ(0x3438, cut.scanIntToken<int32_t>());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("0Xabc 0xF01 023 0x34t"), cut.getSuffix());
        ASSERT_EQ(0xabc, cut.scanIntToken<int32_t>());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("0xF01 023 0x34t"), cut.getSuffix());
        ASSERT_EQ(0xf01, cut.scanIntToken<int32_t>());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("023 0x34t"), cut.getSuffix());
        ASSERT_EQ(19, cut.scanIntToken<int32_t>());
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(ConstString("0x34t"), cut.getSuffix());
        ASSERT_EQ(0, cut.scanIntToken<int32_t>());
        ASSERT_EQ(ConstString("t"), cut.getSuffix());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
        ASSERT_EQ(0, cut.scanIntToken<int32_t>());
        ASSERT_EQ(ConstString(""), cut.scanIdentifierToken());
    }
    {
        CommandContext cut(ConstString("  0238"));
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(0, cut.scanIntToken<int32_t>());
        ASSERT_EQ(ConstString("8"), cut.getSuffix());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" 0afe234ae1"));
        uint8_t buffer[5];
        uint8_t const expected[] = {0x0a, 0xfe, 0x23, 0x4a, 0xe1};
        ASSERT_TRUE(cut.hasToken());
        EXPECT_THAT(cut.scanByteBufferToken(buffer), ElementsAreArray(expected));
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString("0afe234ae1b8f", 12));
        uint8_t buffer[6];
        uint8_t const expected[] = {0x0a, 0xfe, 0x23, 0x4a, 0xe1, 0xb8};
        ASSERT_TRUE(cut.hasToken());
        EXPECT_THAT(cut.scanByteBufferToken(buffer), ElementsAreArray(expected));
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" 0afe234a  "));
        uint8_t buffer[5];
        uint8_t const expected[] = {0x0a, 0xfe, 0x23, 0x4a};
        ASSERT_TRUE(cut.hasToken());
        EXPECT_THAT(cut.scanByteBufferToken(buffer), ElementsAreArray(expected));
        ASSERT_FALSE(cut.hasToken());
        ASSERT_TRUE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::OK, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" 0afe234ae1ff"));
        uint8_t buffer[5];
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(0U, cut.scanByteBufferToken(buffer).size());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_VALUE, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" 0afe234 "));
        uint8_t buffer[5];
        ASSERT_TRUE(cut.hasToken());
        ASSERT_EQ(0U, cut.scanByteBufferToken(buffer).size());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::BAD_TOKEN, cut.getResult());
    }
    {
        CommandContext cut(ConstString(" 0afe234 "));
        uint8_t buffer[5];
        ASSERT_TRUE(cut.hasToken());
        cut.check(false, ICommand::Result::ERROR);
        ASSERT_EQ(0U, cut.scanByteBufferToken(buffer).size());
        ASSERT_FALSE(cut.hasToken());
        ASSERT_FALSE(cut.checkEol());
        ASSERT_EQ(ICommand::Result::ERROR, cut.getResult());
    }
}

TEST_F(CommandContextTest, testStringWriter)
{
    {
        CommandContext cut(ConstString(""));
        util::format::SharedStringWriter writer(cut);
        writer.write('a');
        writer.printf("0");
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        CommandContext cut(ConstString(""), &sharedStream);
        util::format::SharedStringWriter writer(cut);
        writer.write('a');
        writer.printf("abc%d", 1);
        ASSERT_EQ(ConstString("aabc1"), ConstString(stream.getString()));
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        CommandContext cut(ConstString(""), &sharedStream);
        cut.startOutput(&user);

        EXPECT_CALL(user, endContinuousOutput(_)).Times(1);
        cut.endOutput(&user);

        cut.releaseContinuousUser(user);
    }
    {
        ::util::stream::declare::StringBufferOutputStream<20> stream;
        SharedOutputStream sharedStream(stream);
        CommandContext cut(ConstString(""), &sharedStream);
        cut.endOutput(nullptr);
    }
}
