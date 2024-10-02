// Copyright 2024 Accenture.

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
