// Copyright 2024 Accenture.

#pragma once

#include "util/eeprom/IEepromHelper.h"

#include <estd/memory.h>

#include <gmock/gmock.h>

namespace eeprom
{
struct EepromHelperMock : IEepromHelper
{
    MOCK_METHOD(
        bool, read, (uint32_t blockId, size_t offset, uint8_t* buffer, size_t length), (override));
    MOCK_METHOD(
        bool,
        write,
        (uint32_t blockId, size_t offset, uint8_t const* buffer, size_t length),
        (override));
};

ACTION_P(CopyFromBuffer, buf)
{
    ::estd::memory::copy(::estd::slice<uint8_t>::from_pointer(arg2, arg3), buf);
}
} // namespace eeprom
