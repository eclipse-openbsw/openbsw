// Copyright 2024 Accenture.

#include "util/crc/Crc.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

TEST(CrcTests, CalculateCRCWithReferenceValueComparison)
{
    uint8_t data[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    using Crc8      = CrcCalculator<uint8_t, 0x07>;
    uint8_t result  = Crc8::update(data, sizeof(data), 0);
    result          = Crc8::digest(result);

    ASSERT_EQ(result, 0xF4);
}

TEST(CrcTests, CalculateCRCWithNoData)
{
    uint8_t* data  = nullptr;
    using Crc8     = CrcCalculator<uint8_t, 0x07>;
    uint8_t result = Crc8::update(data, 0, 0);
    EXPECT_EQ(result, 0);
    result = Crc8::digest(result);

    ASSERT_EQ(result, 0x0);
}

TEST(CrcTests, CalculateCRCWithInvalidData)
{
    ::estd::AssertHandlerScope scope(::estd::AssertExceptionHandler);
    uint8_t* data = nullptr;
    using Crc8    = CrcCalculator<uint8_t, 0x07>;
    ASSERT_THROW(Crc8::update(data, 1, 0), ::estd::assert_exception);
}

} // anonymous namespace
