// Copyright 2024 Accenture.

#ifndef GUARD_0D636399_8EF6_4E40_A107_2B33B943CFA0
#define GUARD_0D636399_8EF6_4E40_A107_2B33B943CFA0

#include "util/eeprom/IEepromHelper.h"

#include <etl/memory.h>

#include <gmock/gmock.h>

namespace eeprom
{
struct EepromHelperMock : IEepromHelper
{
    MOCK_METHOD4(read, bool(uint32_t blockId, size_t offset, uint8_t* buffer, size_t length));
    MOCK_METHOD4(
        write, bool(uint32_t blockId, size_t offset, uint8_t const* buffer, size_t length));
};

ACTION_P(CopyFromBuffer, buf) { ::etl::mem_copy(buf, arg3, arg2); }
} // namespace eeprom

#endif // GUARD_0D636399_8EF6_4E40_A107_2B33B943CFA0
