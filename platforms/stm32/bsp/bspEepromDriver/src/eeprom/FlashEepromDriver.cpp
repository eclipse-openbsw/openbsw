// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#include <eeprom/FlashEepromDriver.h>

#include <cstring>
#include <mcu/mcu.h>

namespace eeprom
{

FlashEepromDriver::FlashEepromDriver(Config const& config) : fConfig(config), fActivePage(0U) {}

bsp::BspReturnCode FlashEepromDriver::init()
{
    // Determine which page is active by checking magic number
    if (isPageValid(fConfig.page0Address))
    {
        fActivePage = 0U;
    }
    else if (isPageValid(fConfig.page1Address))
    {
        fActivePage = 1U;
    }
    else
    {
        // Neither page valid — format page 0
        if (erasePage(fConfig.page0Address) != bsp::BSP_OK)
        {
            return bsp::BSP_ERROR;
        }

        // Write magic to page 0
        if (unlockFlash() != bsp::BSP_OK)
        {
            return bsp::BSP_ERROR;
        }
        uint32_t const magic = MAGIC;
        if (programPage(fConfig.page0Address, reinterpret_cast<uint8_t const*>(&magic), 4U)
            != bsp::BSP_OK)
        {
            lockFlash();
            return bsp::BSP_ERROR;
        }
        lockFlash();
        fActivePage = 0U;
    }

    return bsp::BSP_OK;
}

bsp::BspReturnCode FlashEepromDriver::read(uint32_t address, uint8_t* buffer, uint32_t length)
{
    uint32_t dataOffset = 4U; // Skip magic word
    if ((address + length) > (fConfig.pageSize - dataOffset))
    {
        return bsp::BSP_ERROR;
    }

    uintptr_t srcAddr = activePageAddress() + dataOffset + address;
    std::memcpy(buffer, reinterpret_cast<void const*>(srcAddr), length);
    return bsp::BSP_OK;
}

bsp::BspReturnCode FlashEepromDriver::write(
    uint32_t address, uint8_t const* buffer, uint32_t length)
{
    uint32_t dataOffset = 4U;
    uint32_t dataSize   = fConfig.pageSize - dataOffset;

    if ((address + length) > dataSize)
    {
        return bsp::BSP_ERROR;
    }

    // Static buffer for page copy — avoids stack allocation in the safety task.
    // Sized for G474RE (2KB pages).  F413ZH uses 128KB sectors which are too
    // large for this ping-pong scheme; use sector-level storage instead.
    static uint8_t pageBuf[2048];
    if (dataSize > sizeof(pageBuf))
    {
        return bsp::BSP_ERROR;
    }

    // Copy current data
    std::memcpy(pageBuf, reinterpret_cast<void const*>(activePageAddress() + dataOffset), dataSize);

    // Apply the write
    std::memcpy(&pageBuf[address], buffer, length);

    // Program inactive page
    uintptr_t inactiveAddr = inactivePageAddress();
    if (erasePage(inactiveAddr) != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    if (unlockFlash() != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    // Write magic
    uint32_t const magic = MAGIC;
    if (programPage(inactiveAddr, reinterpret_cast<uint8_t const*>(&magic), 4U) != bsp::BSP_OK)
    {
        lockFlash();
        return bsp::BSP_ERROR;
    }

    // Write data
    if (programPage(inactiveAddr + dataOffset, pageBuf, dataSize) != bsp::BSP_OK)
    {
        lockFlash();
        return bsp::BSP_ERROR;
    }

    lockFlash();

    // Erase old active page
    uintptr_t oldActiveAddr = activePageAddress();
    (void)erasePage(oldActiveAddr);

    // Swap active page
    fActivePage = (fActivePage == 0U) ? 1U : 0U;

    return bsp::BSP_OK;
}

bsp::BspReturnCode FlashEepromDriver::erase()
{
    if (erasePage(fConfig.page0Address) != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }
    if (erasePage(fConfig.page1Address) != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }
    return init();
}

uintptr_t FlashEepromDriver::activePageAddress() const
{
    return (fActivePage == 0U) ? fConfig.page0Address : fConfig.page1Address;
}

uintptr_t FlashEepromDriver::inactivePageAddress() const
{
    return (fActivePage == 0U) ? fConfig.page1Address : fConfig.page0Address;
}

bool FlashEepromDriver::isPageValid(uintptr_t pageAddr) const
{
    uint32_t const magic = *reinterpret_cast<uint32_t const volatile*>(pageAddr);
    return magic == MAGIC;
}

bsp::BspReturnCode FlashEepromDriver::unlockFlash()
{
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        FLASH->KEYR = 0x45670123U;
        FLASH->KEYR = 0xCDEF89ABU;
    }
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return bsp::BSP_ERROR;
    }
    return bsp::BSP_OK;
}

void FlashEepromDriver::lockFlash() { FLASH->CR |= FLASH_CR_LOCK; }

bsp::BspReturnCode FlashEepromDriver::waitForFlash()
{
    uint32_t timeout = 0xFFFFFFU;
    while ((FLASH->SR & FLASH_SR_BSY) != 0U)
    {
        if (--timeout == 0U)
        {
            return bsp::BSP_ERROR;
        }
    }
    return bsp::BSP_OK;
}

bsp::BspReturnCode FlashEepromDriver::erasePage(uintptr_t pageAddr)
{
    if (unlockFlash() != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    if (waitForFlash() != bsp::BSP_OK)
    {
        lockFlash();
        return bsp::BSP_ERROR;
    }

#if defined(STM32G474xx)
    // STM32G4: page erase — calculate page number from address
    uint32_t pageNum = (pageAddr - 0x08000000U) / fConfig.pageSize;
    FLASH->CR        = (FLASH->CR & ~(FLASH_CR_PNB_Msk)) | (pageNum << FLASH_CR_PNB_Pos)
                | FLASH_CR_PER;
    FLASH->CR |= FLASH_CR_STRT;
#elif defined(STM32F413xx)
    // STM32F4: sector erase — determine sector from address
    // Simplified: assume pageAddr maps directly to a sector number
    // For sectors 12-15 (dual bank), SNB field encodes sector + bank
    uint32_t sector = 0U;
    if (pageAddr >= 0x08100000U)
    {
        sector = ((pageAddr - 0x08100000U) / 0x4000U) + 16U; // Bank 2
    }
    else
    {
        sector = (pageAddr - 0x08000000U) / 0x20000U; // 128KB sectors (simplified)
    }
    FLASH->CR = (FLASH->CR & ~(FLASH_CR_SNB_Msk)) | (sector << FLASH_CR_SNB_Pos)
                | FLASH_CR_SER;
    FLASH->CR |= FLASH_CR_STRT;
#endif

    bsp::BspReturnCode result = waitForFlash();

    // Clear operation bits
#if defined(STM32G474xx)
    FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_STRT);
#elif defined(STM32F413xx)
    FLASH->CR &= ~(FLASH_CR_SER | FLASH_CR_STRT);
#endif

    lockFlash();
    return result;
}

bsp::BspReturnCode FlashEepromDriver::programPage(
    uintptr_t destAddr, uint8_t const* data, uint32_t length)
{
    if (waitForFlash() != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

#if defined(STM32G474xx)
    // STM32G4: double-word (64-bit) programming
    FLASH->CR |= FLASH_CR_PG;

    uint32_t i = 0U;
    while (i < length)
    {
        // Write 64 bits at a time
        uint32_t word0 = 0xFFFFFFFFU;
        uint32_t word1 = 0xFFFFFFFFU;

        uint32_t remaining = length - i;
        if (remaining > 0U)
        {
            std::memcpy(&word0, &data[i], (remaining >= 4U) ? 4U : remaining);
        }
        if (remaining > 4U)
        {
            uint32_t r2 = remaining - 4U;
            std::memcpy(&word1, &data[i + 4U], (r2 >= 4U) ? 4U : r2);
        }

        *reinterpret_cast<uint32_t volatile*>(destAddr + i)      = word0;
        *reinterpret_cast<uint32_t volatile*>(destAddr + i + 4U) = word1;

        if (waitForFlash() != bsp::BSP_OK)
        {
            FLASH->CR &= ~FLASH_CR_PG;
            return bsp::BSP_ERROR;
        }

        i += 8U;
    }

    FLASH->CR &= ~FLASH_CR_PG;
#elif defined(STM32F413xx)
    // STM32F4: word (32-bit) programming
    FLASH->CR |= FLASH_CR_PG;
    // Set PSIZE to word (32-bit)
    FLASH->CR = (FLASH->CR & ~FLASH_CR_PSIZE) | FLASH_CR_PSIZE_1;

    uint32_t i = 0U;
    while (i < length)
    {
        uint32_t word = 0xFFFFFFFFU;
        uint32_t remaining = length - i;
        std::memcpy(&word, &data[i], (remaining >= 4U) ? 4U : remaining);

        *reinterpret_cast<uint32_t volatile*>(destAddr + i) = word;

        if (waitForFlash() != bsp::BSP_OK)
        {
            FLASH->CR &= ~FLASH_CR_PG;
            return bsp::BSP_ERROR;
        }

        i += 4U;
    }

    FLASH->CR &= ~FLASH_CR_PG;
#endif

    return bsp::BSP_OK;
}

} // namespace eeprom
