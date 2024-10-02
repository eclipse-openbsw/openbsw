// Copyright 2024 Accenture.

#ifndef GUARD_65A82A89_608A_485B_8FA5_D3A838DA8BE9
#define GUARD_65A82A89_608A_485B_8FA5_D3A838DA8BE9

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

#endif /* GUARD_65A82A89_608A_485B_8FA5_D3A838DA8BE9 */
