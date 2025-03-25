// Copyright 2024 Accenture.

#pragma once

#include "util/crc/Crc.h"

namespace util
{
namespace crc
{
struct Crc32
{
    using Ethernet = CrcRegister<uint32_t, 0x4C11DB7U, 0xFFFFFFFFU, true, true, 0xFFFFFFFFU>;
    using ARE2EP4  = CrcRegister<uint32_t, 0xF4ACFB13U, 0xFFFFFFFFU, true, true, 0xFFFFFFFFU>;
};

} /* namespace crc */
} /* namespace util */

