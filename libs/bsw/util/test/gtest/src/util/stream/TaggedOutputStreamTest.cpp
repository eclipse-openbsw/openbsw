// Copyright 2024 Accenture.

#include "util/stream/TaggedOutputStream.h"

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util::stream;

TEST(TaggedOutputStream, testPrefixAndSuffixesAreInserted)
{
    {
        declare::StringBufferOutputStream<80> stream;
        {
            TaggedOutputStream cut(stream, "[START]", "[CRLF]");
            cut.write('a');
            cut.write('\n');
            cut.write(::estd::make_str("abc\ndef"));
            cut.write(::estd::make_str("AB\nDEF"));
            ASSERT_EQ(
                "[START]a[CRLF][START]abc[CRLF][START]defAB[CRLF][START]DEF",
                std::string(stream.getString()));
        }
        ASSERT_EQ(
            "[START]a[CRLF][START]abc[CRLF][START]defAB[CRLF][START]DEF[CRLF]",
            std::string(stream.getString()));
    }
    {
        declare::StringBufferOutputStream<80> stream;
        {
            TaggedOutputStream cut(stream, "[START]", "[CRLF]");
            cut.write('\n');
        }
        ASSERT_EQ("[START][CRLF]", std::string(stream.getString()));
    }
}

TEST(TaggedOutputStream, testEofIsReportedCorrectly)
{
    declare::StringBufferOutputStream<10> stream;
    TaggedOutputStream cut(stream, "", "[CRLF]");
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    cut.write('b');
    cut.write('\n');
    ASSERT_FALSE(cut.isEof());
    cut.write('a');
    ASSERT_TRUE(cut.isEof());
}
