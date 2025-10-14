// Copyright 2024 Accenture.

#include "util/meta/Bitmask.h"

#include <gmock/gmock.h>

using namespace ::testing;
using namespace ::util::meta;

TEST(Bitmask, Bitmask8bit)
{
    ASSERT_EQ(uint8_t(0x00), (Bitmask<uint8_t, 0>::value));
    ASSERT_EQ(uint8_t(0x01), (Bitmask<uint8_t, 1>::value));
    ASSERT_EQ(uint8_t(0x03), (Bitmask<uint8_t, 2>::value));
    ASSERT_EQ(uint8_t(0x07), (Bitmask<uint8_t, 3>::value));
    ASSERT_EQ(uint8_t(0x0F), (Bitmask<uint8_t, 4>::value));
    ASSERT_EQ(uint8_t(0x1F), (Bitmask<uint8_t, 5>::value));
    ASSERT_EQ(uint8_t(0x3F), (Bitmask<uint8_t, 6>::value));
    ASSERT_EQ(uint8_t(0x7F), (Bitmask<uint8_t, 7>::value));
    ASSERT_EQ(uint8_t(0xFF), (Bitmask<uint8_t, 8>::value));
}

TEST(Bitmask, Bitmask16bit)
{
    ASSERT_EQ(uint16_t(0x0000), (Bitmask<uint16_t, 0>::value));
    ASSERT_EQ(uint16_t(0x0001), (Bitmask<uint16_t, 1>::value));
    ASSERT_EQ(uint16_t(0x0003), (Bitmask<uint16_t, 2>::value));
    ASSERT_EQ(uint16_t(0x0007), (Bitmask<uint16_t, 3>::value));
    ASSERT_EQ(uint16_t(0x000F), (Bitmask<uint16_t, 4>::value));
    ASSERT_EQ(uint16_t(0x001F), (Bitmask<uint16_t, 5>::value));
    ASSERT_EQ(uint16_t(0x003F), (Bitmask<uint16_t, 6>::value));
    ASSERT_EQ(uint16_t(0x007F), (Bitmask<uint16_t, 7>::value));
    ASSERT_EQ(uint16_t(0x00FF), (Bitmask<uint16_t, 8>::value));
    ASSERT_EQ(uint16_t(0x01FF), (Bitmask<uint16_t, 9>::value));
    ASSERT_EQ(uint16_t(0x03FF), (Bitmask<uint16_t, 10>::value));
    ASSERT_EQ(uint16_t(0x07FF), (Bitmask<uint16_t, 11>::value));
    ASSERT_EQ(uint16_t(0x0FFF), (Bitmask<uint16_t, 12>::value));
    ASSERT_EQ(uint16_t(0x1FFF), (Bitmask<uint16_t, 13>::value));
    ASSERT_EQ(uint16_t(0x3FFF), (Bitmask<uint16_t, 14>::value));
    ASSERT_EQ(uint16_t(0x7FFF), (Bitmask<uint16_t, 15>::value));
    ASSERT_EQ(uint16_t(0xFFFF), (Bitmask<uint16_t, 16>::value));
}
