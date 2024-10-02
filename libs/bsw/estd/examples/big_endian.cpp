// Copyright 2024 Accenture.

#include "estd/big_endian.h"

#include <gmock/gmock.h>
using namespace ::testing;

TEST(BigEndianExample, read_u16)
{
    // [EXAMPLE_READ_U16_START]
    uint8_t const buffer[] = {0xAB, 0x12};
    EXPECT_EQ(0xAB12U, ::estd::read_be<uint16_t>(&buffer[0]));
    // [EXAMPLE_READ_U16_END]
}

TEST(BigEndianExample, write_u32)
{
    // [EXAMPLE_WRITE_U32_START]
    uint8_t buffer[4] = {0};
    ::estd::write_be<uint32_t>(&buffer[0], 0x12345678);
    EXPECT_THAT(buffer, ElementsAre(0x12, 0x34, 0x56, 0x78));
    // [EXAMPLE_WRITE_U32_END]
}
