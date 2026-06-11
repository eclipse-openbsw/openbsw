/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <bsp/Bsp.h>
#include <bsp/eeprom/IEepromDriver.h>
#include <platform/estdint.h>

namespace eeprom
{

// EEPROM emulation in internal flash using two pages in a ping-pong scheme:
// the active page holds current data behind a magic marker, the inactive
// page is erased and reprogrammed on each write, then the roles swap.
class FlashEepromDriver : public IEepromDriver
{
public:
    struct Config
    {
        uintptr_t page0Address;
        uintptr_t page1Address;
        uint32_t pageSize;
    };

    explicit FlashEepromDriver(Config const& config);

    bsp::BspReturnCode init() override;
    bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) override;
    bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length) override;

    bsp::BspReturnCode erase();

private:
    static uint32_t const MAGIC = 0xEE50AA55U; // Page validity marker

    Config const fConfig;
    uint8_t fActivePage;

    uintptr_t activePageAddress() const;
    uintptr_t inactivePageAddress() const;

    bool isPageValid(uintptr_t pageAddr) const;
    bsp::BspReturnCode erasePage(uintptr_t pageAddr);
    bsp::BspReturnCode programPage(uintptr_t destAddr, uint8_t const* data, uint32_t length);

    bsp::BspReturnCode unlockFlash();
    void lockFlash();
    bsp::BspReturnCode waitForFlash();
};

} // namespace eeprom
