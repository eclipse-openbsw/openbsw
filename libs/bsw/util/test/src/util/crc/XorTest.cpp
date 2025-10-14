// Copyright 2024 Accenture.

#include "util/crc/Xor.h"

#include <gmock/gmock.h>

namespace
{
using namespace ::util::crc;
using namespace ::util;
using namespace ::testing;

TEST(Xor, uint8_t__with_0)
{
    // setup
    uint8_t value          = 10U;
    uint8_t expectedResult = 10U;

    // execute
    uint8_t result = Xor<uint8_t>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

TEST(Xor, uint8_t__with_0xFF)
{
    // setup
    uint8_t value          = 0xF0U;
    uint8_t expectedResult = value ^ 0xFF;

    // execute
    uint8_t result = Xor<uint8_t, 0xFFU>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

TEST(Xor, uint16_t__with_0)
{
    // setup
    uint16_t value          = 0xF0F0U;
    uint16_t expectedResult = 0xF0F0U;

    // execute
    uint16_t result = Xor<uint16_t>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

TEST(Xor, uint16_t__with_0xFFFF)
{
    // setup
    uint16_t value          = 0xF0F0U;
    uint16_t expectedResult = value ^ 0xFFFFU;

    // execute
    uint16_t result = Xor<uint16_t, 0xFFFFU>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

TEST(Xor, uint32_t__with_0)
{
    // setup
    uint32_t value          = 0xF0F0F0F0U;
    uint32_t expectedResult = value;

    // execute
    uint32_t result = Xor<uint32_t>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

TEST(Xor, uint32_t__with_0xFFFFFFFF)
{
    // setup
    uint32_t value          = 0xF0F0F0F0U;
    uint32_t expectedResult = value ^ 0xFFFFFFFFU;

    // execute
    uint32_t result = Xor<uint32_t, 0xFFFFFFFFU>::apply(value);

    // verify
    ASSERT_EQ(result, expectedResult);
}

} // anonymous namespace
