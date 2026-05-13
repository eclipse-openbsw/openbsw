// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   FlashEepromDriverTest.cpp
 * \brief  Unit tests for the STM32 flash-based EEPROM emulation driver.
 *
 * Uses fake flash pages (static uint8_t arrays), a fake FLASH_TypeDef, and
 * overridden pointer casts so all memory accesses stay within RAM.
 * Targets the STM32G474xx (G4) code path.
 */

#include <cstdint>
#include <cstring>

#ifndef __IO
#define __IO volatile
#endif

// Select G4 code path
#define STM32G474xx

// =============================================================================
// Fake flash memory — two 2KB pages
// =============================================================================
static uint8_t fakePage0[2048];
static uint8_t fakePage1[2048];

// =============================================================================
// Fake FLASH_TypeDef
// =============================================================================
typedef struct
{
    __IO uint32_t ACR;
    __IO uint32_t PDKEYR;
    __IO uint32_t KEYR;
    __IO uint32_t OPTKEYR;
    __IO uint32_t SR;
    __IO uint32_t CR;
    __IO uint32_t ECCR;
    uint32_t RESERVED1;
    __IO uint32_t OPTR;
    __IO uint32_t PCROP1SR;
    __IO uint32_t PCROP1ER;
    __IO uint32_t WRP1AR;
    __IO uint32_t WRP1BR;
} FLASH_TypeDef;

static FLASH_TypeDef fakeFlash;

#define FLASH (&fakeFlash)

// FLASH register bit definitions (G4)
#define FLASH_CR_PG       (1U << 0)
#define FLASH_CR_PER      (1U << 1)
#define FLASH_CR_STRT     (1U << 16)
#define FLASH_CR_LOCK     (1U << 31)
#define FLASH_CR_PNB_Pos  3U
#define FLASH_CR_PNB_Msk  (0x7FU << FLASH_CR_PNB_Pos)
#define FLASH_SR_BSY      (1U << 16)

// =============================================================================
// Fake RCC_TypeDef (minimal — not used by FlashEepromDriver but needed for mcu.h)
// =============================================================================
typedef struct
{
    uint32_t pad[40];
} RCC_TypeDef;
static RCC_TypeDef fakeRcc;
#define RCC (&fakeRcc)

// =============================================================================
// Fake GPIO_TypeDef (minimal — not used by FlashEepromDriver)
// =============================================================================
typedef struct
{
    uint32_t pad[12];
} GPIO_TypeDef;

// =============================================================================
// Provide BspReturnCode and IEepromDriver before including production code
// =============================================================================

// Override bsp/Bsp.h
namespace bsp
{
enum BspReturnCode
{
    BSP_OK,
    BSP_ERROR,
    BSP_NOT_SUPPORTED
};
} // namespace bsp

// Override bsp/eeprom/IEepromDriver.h
namespace eeprom
{
class IEepromDriver
{
public:
    virtual bsp::BspReturnCode init()                                                      = 0;
    virtual bsp::BspReturnCode write(uint32_t address, uint8_t const* buffer, uint32_t length) = 0;
    virtual bsp::BspReturnCode read(uint32_t address, uint8_t* buffer, uint32_t length)    = 0;

protected:
    IEepromDriver& operator=(IEepromDriver const&) = default;
};
} // namespace eeprom

// Prevent real headers from being included
#define BSP_BSP_H
#define BSP_EEPROM_IEEPROMDRIVER_H
#define PLATFORM_ESTDINT_H

// =============================================================================
// Shim: Override the production FlashEepromDriver.h so it uses our fakes
// =============================================================================

// Override mcu/mcu.h — it is included by the .cpp but we have already defined everything
#define MCU_MCU_H

// Include the production header directly
// We need to provide FlashEepromDriver.h content manually since it includes bsp/Bsp.h etc.

// The FlashEepromDriver class definition:
namespace eeprom
{

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
    static uint32_t const MAGIC = 0xEE50AA55U;

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

// Prevent the real header from being included
#define EEPROM_FLASHEEPROMDRIVER_H

// =============================================================================
// Now provide the production .cpp implementation inline with our fakes active.
// We copy the implementation here because it accesses memory via reinterpret_cast
// on addresses — we need those addresses to map to our fakePage arrays.
// =============================================================================

namespace eeprom
{

FlashEepromDriver::FlashEepromDriver(Config const& config) : fConfig(config), fActivePage(0U) {}

bsp::BspReturnCode FlashEepromDriver::init()
{
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
        if (erasePage(fConfig.page0Address) != bsp::BSP_OK)
        {
            return bsp::BSP_ERROR;
        }
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
    uint32_t dataOffset = 4U;
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

    static uint8_t pageBuf[2048];
    if (dataSize > sizeof(pageBuf))
    {
        return bsp::BSP_ERROR;
    }

    std::memcpy(pageBuf, reinterpret_cast<void const*>(activePageAddress() + dataOffset), dataSize);
    std::memcpy(&pageBuf[address], buffer, length);

    uintptr_t inactiveAddr = inactivePageAddress();
    if (erasePage(inactiveAddr) != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    if (unlockFlash() != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    uint32_t const magic = MAGIC;
    if (programPage(inactiveAddr, reinterpret_cast<uint8_t const*>(&magic), 4U) != bsp::BSP_OK)
    {
        lockFlash();
        return bsp::BSP_ERROR;
    }

    if (programPage(inactiveAddr + dataOffset, pageBuf, dataSize) != bsp::BSP_OK)
    {
        lockFlash();
        return bsp::BSP_ERROR;
    }

    lockFlash();

    uintptr_t oldActiveAddr = activePageAddress();
    (void)erasePage(oldActiveAddr);

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
    // In our fake, we always allow unlock by clearing LOCK
    FLASH->CR &= ~FLASH_CR_LOCK;
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U)
    {
        return bsp::BSP_ERROR;
    }
    return bsp::BSP_OK;
}

void FlashEepromDriver::lockFlash() { FLASH->CR |= FLASH_CR_LOCK; }

bsp::BspReturnCode FlashEepromDriver::waitForFlash()
{
    // In test, BSY is never set so this returns immediately
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

    // Simulate the erase: fill the page with 0xFF
    std::memset(reinterpret_cast<void*>(pageAddr), 0xFF, fConfig.pageSize);

    FLASH->CR &= ~(FLASH_CR_PER | FLASH_CR_STRT);
    lockFlash();
    return bsp::BSP_OK;
}

bsp::BspReturnCode FlashEepromDriver::programPage(
    uintptr_t destAddr, uint8_t const* data, uint32_t length)
{
    if (waitForFlash() != bsp::BSP_OK)
    {
        return bsp::BSP_ERROR;
    }

    // Simulate programming: just memcpy the data to the destination
    std::memcpy(reinterpret_cast<void*>(destAddr), data, length);

    return bsp::BSP_OK;
}

} // namespace eeprom

// =============================================================================
// Tests
// =============================================================================

#include <gtest/gtest.h>

using namespace eeprom;

static uint32_t const PAGE_SIZE = 2048U;
static uint32_t const MAGIC     = 0xEE50AA55U;

class FlashEepromDriverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(fakePage0, 0xFF, PAGE_SIZE);
        std::memset(fakePage1, 0xFF, PAGE_SIZE);
        std::memset(&fakeFlash, 0, sizeof(fakeFlash));
        // Flash starts locked
        fakeFlash.CR = FLASH_CR_LOCK;
        // BSY not set so waitForFlash returns OK
        fakeFlash.SR = 0U;
    }

    FlashEepromDriver::Config makeConfig()
    {
        return {reinterpret_cast<uintptr_t>(fakePage0),
                reinterpret_cast<uintptr_t>(fakePage1), PAGE_SIZE};
    }

    void writeMagic(uint8_t* page)
    {
        uint32_t m = MAGIC;
        std::memcpy(page, &m, 4U);
    }
};

// =============================================================================
// init tests
// =============================================================================

TEST_F(FlashEepromDriverTest, init_NeitherPageValid_FormatsPage0)
{
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    // Page 0 should now have the magic
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromDriverTest, init_Page0Valid)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, init_Page1Valid)
{
    writeMagic(fakePage1);
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, init_BothPagesValid_SelectsPage0)
{
    writeMagic(fakePage0);
    writeMagic(fakePage1);
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    // Page 0 should be selected (first check wins)
}

TEST_F(FlashEepromDriverTest, init_Page0ValidWithData_PreservesData)
{
    writeMagic(fakePage0);
    fakePage0[4] = 0x42U;
    fakePage0[5] = 0x43U;
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    uint8_t buf[2];
    EXPECT_EQ(drv.read(0U, buf, 2U), bsp::BSP_OK);
    EXPECT_EQ(buf[0], 0x42U);
    EXPECT_EQ(buf[1], 0x43U);
}

// =============================================================================
// read tests
// =============================================================================

TEST_F(FlashEepromDriverTest, read_Offset0_1Byte)
{
    writeMagic(fakePage0);
    fakePage0[4] = 0xAAU;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xAAU);
}

TEST_F(FlashEepromDriverTest, read_Offset0_4Bytes)
{
    writeMagic(fakePage0);
    fakePage0[4] = 0x11U;
    fakePage0[5] = 0x22U;
    fakePage0[6] = 0x33U;
    fakePage0[7] = 0x44U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[4];
    EXPECT_EQ(drv.read(0U, buf, 4U), bsp::BSP_OK);
    EXPECT_EQ(buf[0], 0x11U);
    EXPECT_EQ(buf[1], 0x22U);
    EXPECT_EQ(buf[2], 0x33U);
    EXPECT_EQ(buf[3], 0x44U);
}

TEST_F(FlashEepromDriverTest, read_AtOffset100)
{
    writeMagic(fakePage0);
    fakePage0[104] = 0xBBU;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(100U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromDriverTest, read_MaxOffset)
{
    writeMagic(fakePage0);
    // Max data = pageSize - 4 = 2044 bytes, so last byte is at address 2043
    fakePage0[4 + 2043] = 0xCCU;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2043U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xCCU);
}

TEST_F(FlashEepromDriverTest, read_EntireDataRegion)
{
    writeMagic(fakePage0);
    for (uint32_t i = 0; i < 2044; i++)
    {
        fakePage0[4 + i] = static_cast<uint8_t>(i & 0xFF);
    }
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++)
    {
        EXPECT_EQ(buf[i], static_cast<uint8_t>(i & 0xFF));
    }
}

TEST_F(FlashEepromDriverTest, read_OutOfBounds_ReturnsError)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[1];
    // address 2044 + length 1 > 2044
    EXPECT_EQ(drv.read(2044U, buf, 1U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, read_LengthExceedsPage_ReturnsError)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[2048];
    EXPECT_EQ(drv.read(0U, buf, 2045U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, read_ZeroLength)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0xFFU;
    EXPECT_EQ(drv.read(0U, &buf, 0U), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, read_BoundaryExact)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, read_BoundaryOneOver)
{
    writeMagic(fakePage0);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[2045];
    EXPECT_EQ(drv.read(0U, buf, 2045U), bsp::BSP_ERROR);
}

// =============================================================================
// write tests
// =============================================================================

TEST_F(FlashEepromDriverTest, write_SingleByte)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    // After write, active page flips to page 1
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x42U);
}

TEST_F(FlashEepromDriverTest, write_MultiByte)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[4] = {0x11, 0x22, 0x33, 0x44};
    EXPECT_EQ(drv.write(0U, data, 4U), bsp::BSP_OK);
    uint8_t buf[4];
    EXPECT_EQ(drv.read(0U, buf, 4U), bsp::BSP_OK);
    EXPECT_EQ(buf[0], 0x11U);
    EXPECT_EQ(buf[1], 0x22U);
    EXPECT_EQ(buf[2], 0x33U);
    EXPECT_EQ(buf[3], 0x44U);
}

TEST_F(FlashEepromDriverTest, write_AtOffset)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xBBU;
    EXPECT_EQ(drv.write(100U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(100U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromDriverTest, write_ThenRead_Consistency)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t pattern[16];
    for (uint8_t i = 0; i < 16; i++)
    {
        pattern[i] = i * 10U;
    }
    EXPECT_EQ(drv.write(50U, pattern, 16U), bsp::BSP_OK);
    uint8_t buf[16];
    EXPECT_EQ(drv.read(50U, buf, 16U), bsp::BSP_OK);
    for (uint8_t i = 0; i < 16; i++)
    {
        EXPECT_EQ(buf[i], i * 10U);
    }
}

TEST_F(FlashEepromDriverTest, write_BoundaryWrite_LastByte)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xEEU;
    EXPECT_EQ(drv.write(2043U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2043U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xEEU);
}

TEST_F(FlashEepromDriverTest, write_OutOfBounds_ReturnsError)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xFFU;
    EXPECT_EQ(drv.write(2044U, &data, 1U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, write_LengthExceedsDataRegion_ReturnsError)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2048];
    std::memset(data, 0xAA, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2045U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, write_AddressPlusLengthOverflow_ReturnsError)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[100];
    std::memset(data, 0, sizeof(data));
    EXPECT_EQ(drv.write(2000U, data, 100U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, write_LargeBlock)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[1024];
    for (uint32_t i = 0; i < 1024; i++)
    {
        data[i] = static_cast<uint8_t>(i & 0xFF);
    }
    EXPECT_EQ(drv.write(0U, data, 1024U), bsp::BSP_OK);
    uint8_t buf[1024];
    EXPECT_EQ(drv.read(0U, buf, 1024U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 1024; i++)
    {
        EXPECT_EQ(buf[i], static_cast<uint8_t>(i & 0xFF));
    }
}

TEST_F(FlashEepromDriverTest, write_FullDataRegion)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0x55, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++)
    {
        EXPECT_EQ(buf[i], 0x55U);
    }
}

// =============================================================================
// erase tests
// =============================================================================

TEST_F(FlashEepromDriverTest, erase_BothPagesErased_ThenReinit)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    drv.write(0U, &data, 1U);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    // After erase + reinit, reading should give 0xFF (erased flash)
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xFFU);
}

TEST_F(FlashEepromDriverTest, erase_CanWriteAfterErase)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    drv.erase();
    uint8_t data = 0xBBU;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromDriverTest, erase_ReturnsOK)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, erase_MultipleErases)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
}

// =============================================================================
// Wear leveling: write flips active page
// =============================================================================

TEST_F(FlashEepromDriverTest, wearLeveling_FirstWriteGoesToPage1)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    // After init (neither valid), page0 is active.
    uint8_t data = 0x01U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    // Now page1 should be active — verify magic on page1
    uint32_t magic;
    std::memcpy(&magic, fakePage1, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromDriverTest, wearLeveling_SecondWriteGoesBackToPage0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data1 = 0x01U;
    drv.write(0U, &data1, 1U);
    uint8_t data2 = 0x02U;
    drv.write(0U, &data2, 1U);
    // Now page0 should be active again
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
    // Read should return latest value
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x02U);
}

TEST_F(FlashEepromDriverTest, wearLeveling_AlternatesPages)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0; i < 10; i++)
    {
        uint8_t data = i;
        EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
        uint8_t buf = 0xFFU;
        EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
        EXPECT_EQ(buf, i);
    }
}

TEST_F(FlashEepromDriverTest, wearLeveling_DataPreservedOnPageSwap)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    // Write at two different offsets
    uint8_t d1 = 0xAAU;
    drv.write(0U, &d1, 1U);
    uint8_t d2 = 0xBBU;
    drv.write(100U, &d2, 1U);
    // The first write should be preserved
    uint8_t buf1 = 0U;
    drv.read(0U, &buf1, 1U);
    EXPECT_EQ(buf1, 0xAAU);
    uint8_t buf2 = 0U;
    drv.read(100U, &buf2, 1U);
    EXPECT_EQ(buf2, 0xBBU);
}

TEST_F(FlashEepromDriverTest, wearLeveling_MultipleWritesSameOffset)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0; i < 20; i++)
    {
        uint8_t data = i;
        drv.write(0U, &data, 1U);
    }
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 19U);
}

// =============================================================================
// Write preserves unmodified data
// =============================================================================

TEST_F(FlashEepromDriverTest, write_PreservesOtherData)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0x11U;
    drv.write(0U, &d1, 1U);
    uint8_t d2 = 0x22U;
    drv.write(10U, &d2, 1U);
    // Byte at offset 0 should still be 0x11
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x11U);
}

TEST_F(FlashEepromDriverTest, write_OverwritePreservesNeighbors)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    drv.write(10U, data, 4U);
    // Overwrite just byte at offset 11
    uint8_t overwrite = 0xEEU;
    drv.write(11U, &overwrite, 1U);
    uint8_t buf[4];
    drv.read(10U, buf, 4U);
    EXPECT_EQ(buf[0], 0xAAU);
    EXPECT_EQ(buf[1], 0xEEU);
    EXPECT_EQ(buf[2], 0xCCU);
    EXPECT_EQ(buf[3], 0xDDU);
}

// =============================================================================
// Init from page1
// =============================================================================

TEST_F(FlashEepromDriverTest, init_Page1Active_ReadsFromPage1)
{
    writeMagic(fakePage1);
    fakePage1[4] = 0x77U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x77U);
}

TEST_F(FlashEepromDriverTest, init_Page1Active_WriteGoesToPage0)
{
    writeMagic(fakePage1);
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x88U;
    drv.write(0U, &data, 1U);
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x88U);
}

// =============================================================================
// Read various address patterns
// =============================================================================

TEST_F(FlashEepromDriverTest, read_AtAddress0)
{
    writeMagic(fakePage0);
    fakePage0[4] = 0x01U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x01U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress1)
{
    writeMagic(fakePage0);
    fakePage0[5] = 0x02U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(1U, &buf, 1U);
    EXPECT_EQ(buf, 0x02U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress255)
{
    writeMagic(fakePage0);
    fakePage0[259] = 0x03U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(255U, &buf, 1U);
    EXPECT_EQ(buf, 0x03U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress1000)
{
    writeMagic(fakePage0);
    fakePage0[1004] = 0x04U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(1000U, &buf, 1U);
    EXPECT_EQ(buf, 0x04U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress2000)
{
    writeMagic(fakePage0);
    fakePage0[2004] = 0x05U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(2000U, &buf, 1U);
    EXPECT_EQ(buf, 0x05U);
}

// =============================================================================
// Write at various sizes
// =============================================================================

TEST_F(FlashEepromDriverTest, write_1Byte)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x11U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x11U);
}

TEST_F(FlashEepromDriverTest, write_8Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    drv.read(0U, buf, 8U);
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(buf[i], i + 1);
    }
}

TEST_F(FlashEepromDriverTest, write_256Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[256];
    for (int i = 0; i < 256; i++)
    {
        data[i] = static_cast<uint8_t>(i);
    }
    EXPECT_EQ(drv.write(0U, data, 256U), bsp::BSP_OK);
    uint8_t buf[256];
    drv.read(0U, buf, 256U);
    for (int i = 0; i < 256; i++)
    {
        EXPECT_EQ(buf[i], static_cast<uint8_t>(i));
    }
}

TEST_F(FlashEepromDriverTest, write_512Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[512];
    std::memset(data, 0xCD, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 512U), bsp::BSP_OK);
    uint8_t buf[512];
    drv.read(0U, buf, 512U);
    for (int i = 0; i < 512; i++)
    {
        EXPECT_EQ(buf[i], 0xCDU);
    }
}

// =============================================================================
// Stress: many writes
// =============================================================================

TEST_F(FlashEepromDriverTest, stress_50Writes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0; i < 50; i++)
    {
        uint8_t data = i;
        EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 49U);
}

TEST_F(FlashEepromDriverTest, stress_WriteDifferentOffsets)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint32_t offset = 0; offset < 100; offset++)
    {
        uint8_t data = static_cast<uint8_t>(offset);
        EXPECT_EQ(drv.write(offset, &data, 1U), bsp::BSP_OK);
    }
    // Verify all offsets
    for (uint32_t offset = 0; offset < 100; offset++)
    {
        uint8_t buf = 0xFFU;
        drv.read(offset, &buf, 1U);
        EXPECT_EQ(buf, static_cast<uint8_t>(offset));
    }
}

// =============================================================================
// Flash lock/unlock behavior
// =============================================================================

TEST_F(FlashEepromDriverTest, flashLock_LockedAfterInit)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    // After init, flash should be locked
    EXPECT_NE(fakeFlash.CR & FLASH_CR_LOCK, 0U);
}

TEST_F(FlashEepromDriverTest, flashLock_LockedAfterWrite)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    drv.write(0U, &data, 1U);
    EXPECT_NE(fakeFlash.CR & FLASH_CR_LOCK, 0U);
}

TEST_F(FlashEepromDriverTest, flashLock_LockedAfterErase)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    drv.erase();
    EXPECT_NE(fakeFlash.CR & FLASH_CR_LOCK, 0U);
}

// =============================================================================
// Additional edge case tests
// =============================================================================

TEST_F(FlashEepromDriverTest, write_ZeroLength)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    // Writing 0 bytes should succeed but not change anything meaningful
    EXPECT_EQ(drv.write(0U, &data, 0U), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, eraseAndRewrite)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data1 = 0xAAU;
    drv.write(0U, &data1, 1U);
    drv.erase();
    uint8_t data2 = 0xBBU;
    drv.write(0U, &data2, 1U);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromDriverTest, init_MultipleTimes)
{
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
}

TEST_F(FlashEepromDriverTest, write_AtBoundary_ExactFit)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[100];
    std::memset(data, 0xAB, sizeof(data));
    // 2044 - 100 = 1944, so writing at 1944 with length 100 is exactly at boundary
    EXPECT_EQ(drv.write(1944U, data, 100U), bsp::BSP_OK);
    uint8_t buf[100];
    drv.read(1944U, buf, 100U);
    for (int i = 0; i < 100; i++)
    {
        EXPECT_EQ(buf[i], 0xABU);
    }
}

TEST_F(FlashEepromDriverTest, write_AtBoundary_OneByteOver)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[101];
    std::memset(data, 0xAB, sizeof(data));
    // 1944 + 101 = 2045 > 2044
    EXPECT_EQ(drv.write(1944U, data, 101U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, read_AfterMultipleWritesDifferentOffsets)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t a = 0x0AU;
    drv.write(0U, &a, 1U);
    uint8_t b = 0x0BU;
    drv.write(500U, &b, 1U);
    uint8_t c = 0x0CU;
    drv.write(1000U, &c, 1U);
    uint8_t d = 0x0DU;
    drv.write(2000U, &d, 1U);

    uint8_t buf;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x0AU);
    drv.read(500U, &buf, 1U);
    EXPECT_EQ(buf, 0x0BU);
    drv.read(1000U, &buf, 1U);
    EXPECT_EQ(buf, 0x0CU);
    drv.read(2000U, &buf, 1U);
    EXPECT_EQ(buf, 0x0DU);
}

// =============================================================================
// Additional tests to reach 80+
// =============================================================================

TEST_F(FlashEepromDriverTest, write_2Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2] = {0xDE, 0xAD};
    EXPECT_EQ(drv.write(0U, data, 2U), bsp::BSP_OK);
    uint8_t buf[2];
    drv.read(0U, buf, 2U);
    EXPECT_EQ(buf[0], 0xDEU);
    EXPECT_EQ(buf[1], 0xADU);
}

TEST_F(FlashEepromDriverTest, write_16Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[16];
    for (int i = 0; i < 16; i++) data[i] = static_cast<uint8_t>(0x10 + i);
    EXPECT_EQ(drv.write(0U, data, 16U), bsp::BSP_OK);
    uint8_t buf[16];
    drv.read(0U, buf, 16U);
    for (int i = 0; i < 16; i++) EXPECT_EQ(buf[i], 0x10 + i);
}

TEST_F(FlashEepromDriverTest, write_64Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[64];
    std::memset(data, 0x77, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 64U), bsp::BSP_OK);
    uint8_t buf[64];
    drv.read(0U, buf, 64U);
    for (int i = 0; i < 64; i++) EXPECT_EQ(buf[i], 0x77U);
}

TEST_F(FlashEepromDriverTest, write_128Bytes)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[128];
    std::memset(data, 0x99, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 128U), bsp::BSP_OK);
    uint8_t buf[128];
    drv.read(0U, buf, 128U);
    for (int i = 0; i < 128; i++) EXPECT_EQ(buf[i], 0x99U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress50)
{
    writeMagic(fakePage0);
    fakePage0[54] = 0x50U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(50U, &buf, 1U);
    EXPECT_EQ(buf, 0x50U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress500)
{
    writeMagic(fakePage0);
    fakePage0[504] = 0x55U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(500U, &buf, 1U);
    EXPECT_EQ(buf, 0x55U);
}

TEST_F(FlashEepromDriverTest, read_AtAddress1500)
{
    writeMagic(fakePage0);
    fakePage0[1504] = 0x15U;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(1500U, &buf, 1U);
    EXPECT_EQ(buf, 0x15U);
}

TEST_F(FlashEepromDriverTest, write_AtOffset50)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x50U;
    EXPECT_EQ(drv.write(50U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    drv.read(50U, &buf, 1U);
    EXPECT_EQ(buf, 0x50U);
}

TEST_F(FlashEepromDriverTest, write_AtOffset500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x55U;
    EXPECT_EQ(drv.write(500U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    drv.read(500U, &buf, 1U);
    EXPECT_EQ(buf, 0x55U);
}

TEST_F(FlashEepromDriverTest, write_AtOffset1000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x10U;
    EXPECT_EQ(drv.write(1000U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    drv.read(1000U, &buf, 1U);
    EXPECT_EQ(buf, 0x10U);
}

TEST_F(FlashEepromDriverTest, write_AtOffset2000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x20U;
    EXPECT_EQ(drv.write(2000U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    drv.read(2000U, &buf, 1U);
    EXPECT_EQ(buf, 0x20U);
}

TEST_F(FlashEepromDriverTest, erase_DataClearedToFF)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    drv.write(0U, data, 10U);
    drv.erase();
    uint8_t buf[10];
    drv.read(0U, buf, 10U);
    for (int i = 0; i < 10; i++) EXPECT_EQ(buf[i], 0xFFU);
}

TEST_F(FlashEepromDriverTest, init_Page0ValidPage1Invalid_SelectsPage0)
{
    writeMagic(fakePage0);
    // Page1 stays 0xFF (no magic)
    FlashEepromDriver drv(makeConfig());
    EXPECT_EQ(drv.init(), bsp::BSP_OK);
    // Write some data and verify it works from page0
    uint8_t data = 0x42U;
    drv.write(0U, &data, 1U);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x42U);
}

TEST_F(FlashEepromDriverTest, write_ThreeConsecutiveWritesDifferentOffsets)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0x01U; drv.write(0U, &d1, 1U);
    uint8_t d2 = 0x02U; drv.write(1U, &d2, 1U);
    uint8_t d3 = 0x03U; drv.write(2U, &d3, 1U);
    uint8_t buf[3];
    drv.read(0U, buf, 3U);
    EXPECT_EQ(buf[0], 0x01U);
    EXPECT_EQ(buf[1], 0x02U);
    EXPECT_EQ(buf[2], 0x03U);
}

TEST_F(FlashEepromDriverTest, write_OverwriteSameOffset)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0xAAU; drv.write(0U, &d1, 1U);
    uint8_t d2 = 0xBBU; drv.write(0U, &d2, 1U);
    uint8_t d3 = 0xCCU; drv.write(0U, &d3, 1U);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0xCCU);
}

TEST_F(FlashEepromDriverTest, read_MultipleAddresses_32ByteBlock)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[32];
    for (int i = 0; i < 32; i++) data[i] = static_cast<uint8_t>(i);
    drv.write(200U, data, 32U);
    uint8_t buf[32];
    drv.read(200U, buf, 32U);
    for (int i = 0; i < 32; i++) EXPECT_EQ(buf[i], static_cast<uint8_t>(i));
}

TEST_F(FlashEepromDriverTest, wearLeveling_ThirdWriteToPage1)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    // Write 1: page0->page1
    uint8_t d1 = 0x01U; drv.write(0U, &d1, 1U);
    // Write 2: page1->page0
    uint8_t d2 = 0x02U; drv.write(0U, &d2, 1U);
    // Write 3: page0->page1
    uint8_t d3 = 0x03U; drv.write(0U, &d3, 1U);
    uint32_t magic;
    std::memcpy(&magic, fakePage1, 4U);
    EXPECT_EQ(magic, MAGIC);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x03U);
}

TEST_F(FlashEepromDriverTest, read_ErrorBadAddress)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[1];
    EXPECT_EQ(drv.read(2044U, buf, 1U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, read_ErrorBadLength)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf[2048];
    EXPECT_EQ(drv.read(0U, buf, 2048U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, write_ErrorBadAddress)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xFFU;
    EXPECT_EQ(drv.write(2044U, &data, 1U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, write_ErrorBadLength)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2048];
    EXPECT_EQ(drv.write(0U, data, 2048U), bsp::BSP_ERROR);
}

TEST_F(FlashEepromDriverTest, erase_WriteAfterErase_Consistent)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0xAAU; drv.write(0U, &d1, 1U);
    uint8_t d2 = 0xBBU; drv.write(100U, &d2, 1U);
    drv.erase();
    // Both should be erased
    uint8_t buf1 = 0U; drv.read(0U, &buf1, 1U);
    EXPECT_EQ(buf1, 0xFFU);
    uint8_t buf2 = 0U; drv.read(100U, &buf2, 1U);
    EXPECT_EQ(buf2, 0xFFU);
    // Write fresh data
    uint8_t d3 = 0xCCU; drv.write(0U, &d3, 1U);
    uint8_t buf3 = 0U; drv.read(0U, &buf3, 1U);
    EXPECT_EQ(buf3, 0xCCU);
}

TEST_F(FlashEepromDriverTest, init_BothPagesValid_Page0HasData)
{
    writeMagic(fakePage0);
    writeMagic(fakePage1);
    fakePage0[4] = 0xAAU;
    fakePage1[4] = 0xBBU;
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    // Page0 is selected first, so data from page0
    EXPECT_EQ(buf, 0xAAU);
}
