// Copyright 2024 Accenture.

#include "util/format/StringWriter.h"

#include "util/stream/StringBufferOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util;

namespace
{
struct StringWriterTest : ::testing::Test
{
    void apply(format::StringWriter& writer) const { writer.printf("ext0"); }

    // NOLINTNEXTLINE(cert-dcl50-cpp): va_list usage only for printing functionalities.
    void callVprintf(format::StringWriter& writer, char const* format, ...)
    {
        va_list va;
        va_start(va, format);
        writer.vprintf(format, va);
        va_end(va);
    }
};
} // namespace

TEST_F(StringWriterTest, testMixedUsage)
{
    stream::declare::StringBufferOutputStream<40> stream;
    format::StringWriter cut(stream);
    cut.write('t')
        .write("abcdef1234:")
        .printf("%d", 10)
        .printf(nullptr)
        .write("test")
        .write(nullptr)
        .write("test", 3)
        .write(::util::string::ConstString("ABCD"))
        .endl();
    callVprintf(cut, "%d", 10);
    ASSERT_EQ(0, strcmp("tabcdef1234:10testtesABCD\n10", stream.getString()));
}

TEST_F(StringWriterTest, testExtensions)
{
    stream::declare::StringBufferOutputStream<40> stream;
    format::StringWriter cut(stream);
    cut.apply(*this);
    ASSERT_EQ(0, strcmp("ext0", stream.getString()));
}
