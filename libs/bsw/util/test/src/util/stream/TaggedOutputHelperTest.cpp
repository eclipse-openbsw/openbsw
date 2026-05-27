/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/TaggedOutputHelper.h"

#include "util/stream/StringBufferOutputStream.h"

#include <etl/string_view.h>

#include <gtest/gtest.h>

namespace stream = ::util::stream;

TEST(TaggedOutputHelper, testPrefixAndSuffixes)
{
    {
        util::stream::declare::StringBufferOutputStream<80> stream;
        stream::TaggedOutputHelper cut(nullptr, nullptr);
        cut.writeBytes(stream, ::etl::string_view("a\nb"));
        ASSERT_EQ("ab", std::string(stream.getString()));
    }
    {
        util::stream::declare::StringBufferOutputStream<80> stream;
        stream::TaggedOutputHelper cut("P", nullptr);
        cut.writeBytes(stream, ::etl::string_view("a\nb"));
        ASSERT_EQ("PaPb", std::string(stream.getString()));
    }
    {
        util::stream::declare::StringBufferOutputStream<80> stream;
        stream::TaggedOutputHelper cut(nullptr, "S");
        cut.writeBytes(stream, ::etl::string_view("a\nb"));
        ASSERT_EQ("aSb", std::string(stream.getString()));
    }
}

TEST(TaggedOutputHelper, testReset)
{
    util::stream::declare::StringBufferOutputStream<80> stream;
    stream::TaggedOutputHelper cut("P", "S");
    cut.writeBytes(stream, ::etl::string_view("a\nb"));
    cut.reset();
    cut.writeBytes(stream, ::etl::string_view("c"));
    ASSERT_EQ("PaSPbPc", std::string(stream.getString()));
}
