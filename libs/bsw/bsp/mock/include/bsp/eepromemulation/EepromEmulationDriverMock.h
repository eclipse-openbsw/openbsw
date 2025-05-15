// Copyright 2024 Accenture.

#pragma once

#include "bsp/eepromemulation/IEepromEmulationDriver.h"

#include <gmock/gmock.h>

namespace eepromemulation
{
class EepromEmulationDriverMock : public IEepromEmulationDriver
{
public:
    MOCK_METHOD(EepromEmulationReturnCode, init, (bool), (override));
    MOCK_METHOD(EepromEmulationReturnCode, read, (uint16_t, uint8_t*, uint16_t&), (override));
    MOCK_METHOD(EepromEmulationReturnCode, write, (uint16_t, uint8_t const*, uint16_t), (override));
    MOCK_METHOD(
        EepromEmulationReturnCode,
        write2,
        (uint16_t, uint16_t, uint8_t const*, uint8_t const*, uint16_t, uint16_t),
        (override));
};

} // namespace eepromemulation
