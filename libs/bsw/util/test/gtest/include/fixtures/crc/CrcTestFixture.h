// Copyright 2024 Accenture.

#pragma once

#include <gmock/gmock.h>

#include <cstdint>
#include <cstring>

namespace test
{
namespace fixtures
{
struct CrcTestFixture : ::testing::Test
{
    CrcTestFixture()
    {
        _one_byte[0]      = 0x31U;
        _one_zero_byte[0] = 0x00U;
        memset(_multiple_zero_bytes, 0, sizeof(_multiple_zero_bytes));

        uint8_t const multipleBytes[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
        memcpy(_multiple_bytes, multipleBytes, sizeof(multipleBytes));
    }

    uint8_t _one_byte[1];
    uint8_t _one_zero_byte[1];
    uint8_t _multiple_bytes[9];
    uint8_t _multiple_zero_bytes[9];
};

} /* namespace fixtures */
} /* namespace test */

