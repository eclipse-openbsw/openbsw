// Copyright 2024 Accenture.

#include "util/crc/Reflect.h"

#include <gmock/gmock.h>

using namespace ::util::crc;
using namespace ::testing;

TEST(Reflect, false_uint8_t)
{
    uint8_t const value          = 10U;
    uint8_t const expectedResult = 10U;

    EXPECT_EQ(expectedResult, Reflect<false>::apply(value));
}

TEST(Reflect, true_uint8_t_nibbles_are_swaped)
{
    uint8_t const value          = 0xF0U;
    uint8_t const expectedResult = 0x0FU;

    ASSERT_EQ(expectedResult, Reflect<true>::apply(value));
}

TEST(Reflect, false_uint16_t)
{
    uint16_t const value          = 10U;
    uint16_t const expectedResult = 10U;

    EXPECT_EQ(expectedResult, Reflect<false>::apply(value));
}

TEST(Reflect, true_uint16_t_bytes_are_swaped)
{
    uint16_t const value          = 0xFF00U;
    uint16_t const expectedResult = 0x00FFU;

    ASSERT_EQ(expectedResult, Reflect<true>::apply(value));
}

TEST(Reflect, false_uint32_t)
{
    uint32_t const value          = 10U;
    uint32_t const expectedResult = 10U;

    EXPECT_EQ(expectedResult, Reflect<false>::apply(value));
}

TEST(Reflect, true_uint32_t_bytes_are_mirrowed)
{
    uint32_t const value          = 0xFF00FFFFU;
    uint32_t const expectedResult = 0xFFFF00FFU;

    EXPECT_EQ(expectedResult, Reflect<true>::apply(value));
}
