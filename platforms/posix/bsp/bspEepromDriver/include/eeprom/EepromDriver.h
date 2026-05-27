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

#include "bsp/EepromConfiguration.h"
#include "bsp/eeprom/IEepromDriver.h"

#include <string>

namespace eeprom
{
class EepromDriver : public IEepromDriver
{
public:
    EepromDriver();
    ~EepromDriver();

    EepromDriver(EepromDriver const&)            = delete;
    EepromDriver& operator=(EepromDriver const&) = delete;
    EepromDriver(EepromDriver&&)                 = delete;
    EepromDriver& operator=(EepromDriver&&)      = delete;

    bsp::BspReturnCode init() override;

    bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) override;

    bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length) override;

private:
    std::string const eepromFilePath = EEPROM_FILEPATH;
    int eepromFd;
};
} // namespace eeprom
