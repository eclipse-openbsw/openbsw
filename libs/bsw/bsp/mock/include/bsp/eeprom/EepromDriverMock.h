// Copyright 2024 Accenture.

#pragma once

#include "bsp/eeprom/IEepromDriver.h"

#include <gmock/gmock.h>

namespace eeprom
{
class EepromDriverMock : public IEepromDriver
{
public:
    MOCK_METHOD0(init, bsp::BspReturnCode());
    MOCK_METHOD3(write, bsp::BspReturnCode(uint32_t, uint8_t const*, uint32_t));
    MOCK_METHOD3(read, bsp::BspReturnCode(uint32_t, uint8_t*, uint32_t));
};

} // namespace eeprom
