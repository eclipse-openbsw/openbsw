// Copyright 2024 Accenture.

#include "logger/EntrySerializer.h"

#include <util/format/StringWriter.h>
#include <util/stream/StringBufferOutputStream.h>

#include <gtest/gtest.h>

using namespace ::logger;

using namespace ::util::logger;

struct EntrySerializerTest
: ::testing::Test
, private IEntrySerializerCallback<uint32_t>
{
    std::string const& serializeAndDeserialize(
        uint32_t bufferSize,
        uint32_t timestamp,
        uint8_t componentIdx,
        uint8_t level,
        char const* formatString,
        ...)
    {
        EntrySerializer<> serializer(
            SectionPredicate(_constStrings, _constStrings + sizeof(_constStrings)));
        memset(_buffer, 0xaf, sizeof(_buffer));
        va_list ap;
        va_start(ap, formatString);
        ::estd::slice<uint8_t> entryBuffer
            = ::estd::make_slice(_buffer).offset(1).subslice(bufferSize);
        _usedBufferSize = serializer.serialize(
            entryBuffer, timestamp, componentIdx, static_cast<Level>(level), formatString, ap);
        _usedBufferSize = _usedBufferSize < bufferSize ? _usedBufferSize : bufferSize;
        va_end(ap);
        _entry.clear();
        EXPECT_EQ(0xaf, _buffer[0]);
        EXPECT_EQ(0xaf, _buffer[bufferSize + 1]);
        serializer.deserialize(entryBuffer.subslice(_usedBufferSize), *this);
        return _entry;
    }

    char const* addConstString(char const* string)
    {
        char* destString = _constStrings + _nextConstStringOffset;
        strcpy(destString, string);
        _nextConstStringOffset += strlen(destString) + 1;
        return destString;
    }

    void onEntry(
        uint32_t timestamp,
        uint8_t componentIndex,
        Level level,
        char const* str,
        util::format::IPrintfArgumentReader& argReader) override
    {
        util::stream::declare::StringBufferOutputStream<300> outputStream;
        util::format::StringWriter writer(outputStream);
        writer.printf("%d:%d:%d:", timestamp, componentIndex, level).vprintf(str, argReader);
        _entry = outputStream.getString();
    }

    uint8_t _buffer[302]{};
    uint32_t _usedBufferSize = 0U;
    std::string _entry;
    uint32_t _nextConstStringOffset = 0;
    char _constStrings[300]{};
};

TEST_F(EntrySerializerTest, testLogWithSmallBuffers)
{
    ASSERT_EQ("", serializeAndDeserialize(0, 123, 1, 2, "test"));
    ASSERT_EQ("", serializeAndDeserialize(1, 123, 1, 2, "test"));
}

TEST_F(EntrySerializerTest, testLogWithSimpleNonConstEntries)
{
    ASSERT_EQ("123:1:2:simpleLog", serializeAndDeserialize(300, 123, 1, 2, "simpleLog"));
    ASSERT_EQ(
        "123:1:2:simpl<?>", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleLog"));
    ASSERT_EQ(
        "123:1:2:simp<?>", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleLog"));
    ASSERT_EQ(
        "123:1:2:sim<?>", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleLog"));
    ASSERT_EQ(
        "123:1:2:si<?>", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleLog"));
    ASSERT_EQ("123:1:2:s<?>", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleLog"));
}

TEST_F(EntrySerializerTest, testLogWithSimpleConstEntries)
{
    char const* pConstString = addConstString("simpleLog");
    ASSERT_EQ("123:1:2:simpleLog", serializeAndDeserialize(300, 123, 1, 2, pConstString));
    ASSERT_EQ("", serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, pConstString));
}

TEST_F(EntrySerializerTest, testLogWithSimpleNonConstStringArg)
{
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value)",
        serializeAndDeserialize(300, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-v<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(ar<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(a<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", "arg-value"));
}

TEST_F(EntrySerializerTest, testLogWithSimpleConstStringArg)
{
    char const* pConstString = addConstString("arg-value");
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value)",
        serializeAndDeserialize(300, 123, 1, 2, "simpleArgLog(%s)", pConstString));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(<?>)",
        serializeAndDeserialize(_usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s)", pConstString));
}

TEST_F(EntrySerializerTest, testLogWithSimpleSizedStringArg)
{
    ::util::string::ConstString sizedString("arg-value334", 9U);
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value)",
        serializeAndDeserialize(300, 123, 1, 2, "simpleArgLog(%S)", sizedString.plain_str()));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(<?>)",
        serializeAndDeserialize(
            _usedBufferSize - 6, 123, 1, 2, "simpleArgLog(%S)", sizedString.plain_str()));
}

TEST_F(EntrySerializerTest, testLogWithEmptySizedStringArg)
{
    ::util::string::ConstString sizedString(nullptr, 0U);
    ASSERT_EQ(
        "123:1:2:simpleArgLog()",
        serializeAndDeserialize(300, 123, 1, 2, "simpleArgLog(%S)", sizedString.plain_str()));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(<?>)",
        serializeAndDeserialize(
            _usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%S)", sizedString.plain_str()));
}

TEST_F(EntrySerializerTest, testLogWithArguments)
{
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value, 123, f)",
        serializeAndDeserialize(300, 123, 1, 2, "simpleArgLog(%s, %d, %c)", "arg-value", 123, 'f'));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value, 123, <?>)",
        serializeAndDeserialize(
            _usedBufferSize - 1, 123, 1, 2, "simpleArgLog(%s, %d, %c)", "arg-value", 123, 'f'));
    ASSERT_EQ(
        "123:1:2:simpleArgLog(arg-value, <?>, <?>)",
        serializeAndDeserialize(
            _usedBufferSize - sizeof(uint32_t),
            123,
            1,
            2,
            "simpleArgLog(%s, %d, %c)",
            "arg-value",
            123,
            'f'));
}

TEST_F(EntrySerializerTest, testPrintfDatatypes)
{
    ASSERT_EQ(
        "124:2:3:17 18 19", serializeAndDeserialize(300, 124, 2, 3, "%d %hd %ld", 17, 18, 19LL));
    ASSERT_EQ(
        "124:2:3:17 18 19", serializeAndDeserialize(300, 124, 2, 3, "%u %hu %lu", 17, 18, 19ULL));
    ASSERT_EQ(
        "124:2:3:f String ConstString <NULL>",
        serializeAndDeserialize(
            300,
            124,
            2,
            3,
            "%c %s %s %s",
            'f',
            "String",
            addConstString("ConstString"),
            (char const*)nullptr));
    ASSERT_EQ(
        "124:2:3:12345678",
        serializeAndDeserialize(300, 124, 2, 3, "%p", reinterpret_cast<char const*>(0x12345678)));
    ASSERT_EQ("124:2:3:", serializeAndDeserialize(300, 124, 2, 3, "%n", 0));
    ASSERT_EQ("124:2:3:    0017", serializeAndDeserialize(300, 124, 2, 3, "%*.*d", 8, 4, 17));
}
