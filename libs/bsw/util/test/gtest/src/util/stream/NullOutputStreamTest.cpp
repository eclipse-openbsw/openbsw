// Copyright 2024 Accenture.

#include "util/stream/NullOutputStream.h"

#include <gtest/gtest.h>

using namespace ::util::stream;

TEST(NullOutputStream, testNothingIsDone)
{
    NullOutputStream cut;
    ASSERT_TRUE(cut.isEof());
    cut.write('a');
    cut.write_string_view(::etl::string_view("abc\ndef"));
}
