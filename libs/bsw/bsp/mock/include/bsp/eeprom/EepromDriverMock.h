// Copyright 2024 Accenture.

#pragma once

#include "bsp/eeprom/IEepromDriver.h"

#include <gmock/gmock.h>

namespace eeprom
{
class EepromDriverMock : public IEepromDriver
{
public:
    MOCK_METHOD(::bsp::BspReturnCode, init, (), (override));
    MOCK_METHOD(::bsp::BspReturnCode, write, (uint32_t, uint8_t const*, uint32_t), (override));
    MOCK_METHOD(::bsp::BspReturnCode, read, (uint32_t, uint8_t*, uint32_t), (override));
};

} // namespace eeprom
