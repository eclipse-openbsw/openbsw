// Copyright 2024 Accenture.

#include "util/meta/BinaryValue.h"

#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::util::meta;

TEST(BinaryValue, metaTemplateWorksCorrectlyForBasicBinaryStrings)
{
    uint32_t binaryValue   = BinaryValue<1>::value;
    uint32_t expectedValue = 1U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<10>::value;
    expectedValue = 2U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<11>::value;
    expectedValue = 3U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<100>::value;
    expectedValue = 4U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<101>::value;
    expectedValue = 5U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<110>::value;
    expectedValue = 6U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<111>::value;
    expectedValue = 7U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<1000>::value;
    expectedValue = 8U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<10000>::value;
    expectedValue = 16U;
    ASSERT_THAT(binaryValue, Eq(expectedValue));

    binaryValue   = BinaryValue<100000>::value;
    expectedValue = 32;
    ASSERT_THAT(binaryValue, Eq(expectedValue));
}
