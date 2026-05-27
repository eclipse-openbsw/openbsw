/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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
