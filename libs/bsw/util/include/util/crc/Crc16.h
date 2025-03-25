// Copyright 2024 Accenture.

#pragma once

#include "util/crc/Crc.h"

namespace util
{
namespace crc
{
struct Crc16
{
    using Ccitt = CrcRegister<uint16_t, 0x1021, 0xFFFF>;
};

} /* namespace crc */
} /* namespace util */

