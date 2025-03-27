// Copyright 2024 Accenture.

#pragma once

#include "util/eeprom/IEepromHelper.h"

#include <estd/memory.h>

#include <gmock/gmock.h>

namespace eeprom
{
struct EepromHelperMock : IEepromHelper
{
    MOCK_METHOD4(read, bool(uint32_t blockId, size_t offset, uint8_t* buffer, size_t length));
    MOCK_METHOD4(
        write, bool(uint32_t blockId, size_t offset, uint8_t const* buffer, size_t length));
};

ACTION_P(CopyFromBuffer, buf)
{
    ::estd::memory::copy(::estd::slice<uint8_t>::from_pointer(arg2, arg3), buf);
}
} // namespace eeprom
