// Copyright 2024 Accenture.

#ifndef GUARD_0A746553_D1DF_418B_884E_465EE18AFC24
#define GUARD_0A746553_D1DF_418B_884E_465EE18AFC24

#include "platform/estdint.h"
#include "util/types/Enum.h"

namespace bsp
{
struct I2cReturnCodes
{
    enum type
    {
        RES_OK,
        RES_NACK,
        RES_ARBITRATION_LOST,
        RES_BUSY,
        RES_BUS_BUSY,
        RES_ERROR
    };
};

using I2cReturnCode = ::util::types::Enum<I2cReturnCodes>;

namespace i2c
{
using AddressType = size_t;

} // namespace i2c
} // namespace bsp

#endif // GUARD_0A746553_D1DF_418B_884E_465EE18AFC24
