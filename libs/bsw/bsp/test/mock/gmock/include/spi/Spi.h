// Copyright 2024 Accenture.

#ifndef GUARD_FA5D81D5_E128_4561_A62B_FF819A376541
#define GUARD_FA5D81D5_E128_4561_A62B_FF819A376541

#include "bsp/Bsp.h"

#include <platform/estdint.h>

#include <gmock/gmock.h>

namespace bios
{
class Spi
{
public:
    MOCK_METHOD6(
        transfer,
        bsp::BspReturnCode(uint8_t, uint8_t const*, uint16_t, uint8_t*, uint16_t, uint8_t*));
    MOCK_METHOD1(Get, int(int));
};

} // namespace bios

#endif /* GUARD_FA5D81D5_E128_4561_A62B_FF819A376541 */
