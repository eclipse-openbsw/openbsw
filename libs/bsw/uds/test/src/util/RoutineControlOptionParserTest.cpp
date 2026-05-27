/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "util/RoutineControlOptionParser.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::uds;

TEST(RoutineControlOptionParserTest, RoutineControlOptionParserCoverage)
{
    uint8_t testSize = 0;
    EXPECT_EQ(RoutineControlOptionParser::getLogicalBlockNumberLength(testSize), 0);
    EXPECT_EQ(RoutineControlOptionParser::getMemoryAddressLength(testSize), 0);
    EXPECT_EQ(RoutineControlOptionParser::getMemorySizeLength(testSize), 0);
    uint8_t testBuffer[4U] = {0, 0, 0, 0};
    for (uint32_t i = 1U; i < 6U; i++)
    {
        EXPECT_EQ(RoutineControlOptionParser::parseParameter(testBuffer, i), 0U);
    }
}
} // anonymous namespace
