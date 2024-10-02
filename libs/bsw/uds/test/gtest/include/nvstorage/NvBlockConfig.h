// Copyright 2024 Accenture.

#ifndef GUARD_D5ABA4AC_EF95_4A54_A278_C256DC351B1A
#define GUARD_D5ABA4AC_EF95_4A54_A278_C256DC351B1A

#include <nvstorage/NvStorageTypes.h>

namespace nvstorage
{
class NvBlockConfig
{
public:
    static NvBlockIdType const BootConfig           = 0U;
    static NvBlockIdType const ActiveResponseClient = 1U;

    static void setRamBlockModified(NvBlockIdType const blockId, bool const modified) {}

    static bool setArrayIndex(NvBlockIdType const, uint8_t) { return true; }

    static bool getArrayIndex(NvBlockIdType const) { return true; }

    static void setMutability(NvBlockIdType const, bool const) {}

    static bool isMutable(NvBlockIdType const) { return true; }
};

} /* namespace nvstorage */

#endif // GUARD_D5ABA4AC_EF95_4A54_A278_C256DC351B1A
