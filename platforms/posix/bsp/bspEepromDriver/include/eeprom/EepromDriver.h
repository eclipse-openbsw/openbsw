// Copyright 2024 Accenture.

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
    static constexpr size_t EEPROM_SIZE = 4096; // 4KB
    std::string const eepromFilePath    = EEPROM_FILEPATH;
    int eepromFd;
};
} // namespace eeprom
