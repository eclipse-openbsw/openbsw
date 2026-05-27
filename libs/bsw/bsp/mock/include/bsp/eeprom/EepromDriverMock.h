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
