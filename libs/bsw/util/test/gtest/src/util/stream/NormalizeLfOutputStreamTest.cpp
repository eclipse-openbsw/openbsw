// Copyright 2024 Accenture.

#include "util/stream/NormalizeLfOutputStream.h"

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util::stream;

TEST(NormalizeLfOutputStream, testLfIsReplacedWithCustomString)
{
    declare::StringBufferOutputStream<40> stream;
    NormalizeLfOutputStream cut(stream, "[CRLF]");
    cut.write('a');
    cut.write('\n');
    cut.write(::estd::make_str("abc\ndef"));
    cut.write(::estd::make_str("AB\nDEF"));
    ASSERT_EQ("a[CRLF]abc[CRLF]defAB[CRLF]DEF", std::string(stream.getString()));
}

TEST(NormalizeLfOutputStream, testLfIsReplacedWithDefaultString)
{
    declare::StringBufferOutputStream<40> stream;
    NormalizeLfOutputStream cut(stream);
    cut.write('a');
    cut.write('\n');
    cut.write(::estd::make_str("abc\ndef"));
    cut.write(::estd::make_str("AB\nDEF"));
    ASSERT_EQ("a\r\nabc\r\ndefAB\r\nDEF", std::string(stream.getString()));
}

TEST(NormalizeLfOutputStream, testEofIsReportedCorrectly)
{
    declare::StringBufferOutputStream<10> stream;
    NormalizeLfOutputStream cut(stream, "[CRLF]");
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    cut.write('b');
    cut.write('\n');
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    ASSERT_TRUE(cut.isEof());
}
