// Copyright 2024 Accenture.

#ifndef GUARD_5E750695_2000_4771_ACD9_E13E57316D9D
#define GUARD_5E750695_2000_4771_ACD9_E13E57316D9D

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

#endif /* GUARD_5E750695_2000_4771_ACD9_E13E57316D9D */
