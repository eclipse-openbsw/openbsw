// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   FlashEepromDriver.h
 * \brief  Flash-based EEPROM emulation for STM32 platforms.
 *
 * Implements IEepromDriver using internal flash as EEPROM storage.
 * Uses two flash pages in a ping-pong scheme for wear leveling:
 *   - Active page: holds current data with a valid magic number
 *   - Inactive page: erased, ready for next swap
 *
 * On write, data is copied from active page with the modified region,
 * then the old page is erased and the new page becomes active.
 *
 * STM32F413ZH: last 2 sectors (128KB each) — sectors 14 & 15
 * STM32G474RE: last 2 pages (2KB each) — pages 254 & 255
 */

#pragma once

#include <bsp/Bsp.h>
#include <bsp/eeprom/IEepromDriver.h>
#include <platform/estdint.h>

namespace eeprom
{

class FlashEepromDriver : public IEepromDriver
{
public:
    struct Config
    {
        uintptr_t page0Address; ///< Start address of flash page 0
        uintptr_t page1Address; ///< Start address of flash page 1
        uint32_t pageSize;      ///< Size of each flash page in bytes
    };

    explicit FlashEepromDriver(Config const& config);

    bsp::BspReturnCode init() override;
    bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) override;
    bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length) override;

    /**
     * \brief Erase both flash pages and reinitialize.
     */
    bsp::BspReturnCode erase();

private:
    static uint32_t const MAGIC = 0xEE50AA55U; ///< Page validity marker

    Config const fConfig;
    uint8_t fActivePage; ///< 0 or 1

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
