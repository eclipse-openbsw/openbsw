// Copyright 2025 Accenture.

#pragma once

#include <bsp/eeprom/IEepromDriver.h>
#include <etl/array.h>
#include <mcu/mcu.h>

namespace eeprom
{

struct EepromConfiguration
{
    uint32_t const fBaseAddress;
    uint32_t const fTotalSize;
    uint32_t const fProtectedAreaSize;
    bool const fLoadOnReset;
    uint8_t const fDataSetSize;
    uint8_t const fPartitionCode;
};

class EepromDriver : public eeprom::IEepromDriver
{
public:
    explicit EepromDriver(EepromConfiguration const& configuration);

    ~EepromDriver()                              = default;
    EepromDriver(EepromDriver const&)            = delete;
    EepromDriver& operator=(EepromDriver const&) = delete;

    bsp::BspReturnCode init() override;
    bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) override;
    bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length) override;
    bool isInitialized() const;

    EepromConfiguration const& getConfiguration() const { return _configuration; }

private:
    bool isOperationAllowed(uint32_t address, uint8_t const* buffer, uint32_t length) const;
    void clearCommand();
    bsp::BspReturnCode launchCommand();

    EepromConfiguration const& _configuration;
    ::etl::array<uint8_t, FTFC_FCCOB_COUNT> _cmd;
};

} // namespace eeprom
