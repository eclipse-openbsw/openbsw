// Copyright 2024 Accenture.

#ifndef GUARD_184814C9_D20C_4F7A_8076_D399E54F2C26
#define GUARD_184814C9_D20C_4F7A_8076_D399E54F2C26

#include "util/crc/Crc.h"
#include "util/crc/LookupTable.h"

namespace util
{
namespace crc
{
struct Crc8
{
    using Ccitt    = CrcRegister<uint8_t, 0x07, 0>;
    using Rohc     = CrcRegister<uint8_t, 0x07, 0xFF, true, true, 0>;
    using Saej1850 = CrcRegister<uint8_t, 0x1D, 0>;
    using Crc8H2F  = CrcRegister<uint8_t, 0x2F, 0xFF, false, false, 0xFF>;
    using Maxim    = CrcRegister<uint8_t, 0x31, 0, true, true, 0>;
    using Crc8F_3  = CrcRegister<uint8_t, 0xCF, 0>;
};

} /* namespace crc */
} /* namespace util */

#endif /* GUARD_184814C9_D20C_4F7A_8076_D399E54F2C26 */
