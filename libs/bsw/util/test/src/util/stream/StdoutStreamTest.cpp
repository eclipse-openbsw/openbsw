/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/stream/StdoutStream.h"

#include "util/StdIoMock.h"

#include <etl/string_view.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;
using namespace ::util::stream;

struct StdoutStreamTest : Test
{
    util::test::StdIoMock stdIo;
};

TEST_F(StdoutStreamTest, testSingleCharactersAreWritten)
{
    StdoutStream cut;
    cut.write('a');
    cut.write('\r');
    cut.write('b');
    cut.write('\n');
    cut.write('c');
    EXPECT_THAT(stdIo.out, ElementsAre('a', '\r', 'b', '\n', 'c'));
}

TEST_F(StdoutStreamTest, testMultipleCharactersAreWritten)
{
    StdoutStream cut;
    cut.write_string_view("a\rb\n");
    cut.write_string_view("c");

    EXPECT_THAT(stdIo.out, ElementsAre('a', '\r', 'b', '\n', 'c'));
}

TEST_F(StdoutStreamTest, testEofIsNeverSet)
{
    StdoutStream cut;
    for (uint32_t i = 0; i < 100; ++i)
    {
        ASSERT_FALSE(cut.isEof());
        cut.write(i);
    }
}
