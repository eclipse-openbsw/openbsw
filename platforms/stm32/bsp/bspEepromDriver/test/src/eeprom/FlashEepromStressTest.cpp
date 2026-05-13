// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   FlashEepromStressTest.cpp
 * \brief  Stress and boundary-condition tests for flash-based EEPROM emulation.
 *
 * Uses the same fake flash pages and FLASH_TypeDef pattern as
 * FlashEepromDriverTest.cpp.
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
// Fake RCC_TypeDef (minimal)
// =============================================================================
typedef struct
{
    uint32_t pad[40];
} RCC_TypeDef;
static RCC_TypeDef fakeRcc;
#define RCC (&fakeRcc)

// =============================================================================
// Fake GPIO_TypeDef (minimal)
// =============================================================================
typedef struct
{
    uint32_t pad[12];
} GPIO_TypeDef;

// =============================================================================
// Provide BspReturnCode and IEepromDriver before including production code
// =============================================================================

namespace bsp
{
enum BspReturnCode
{
    BSP_OK,
    BSP_ERROR,
    BSP_NOT_SUPPORTED
};
} // namespace bsp

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
#define MCU_MCU_H

// FlashEepromDriver class definition
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

#define EEPROM_FLASHEEPROMDRIVER_H

// =============================================================================
// Production implementation (inline with fakes active)
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

class FlashEepromStressTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(fakePage0, 0xFF, PAGE_SIZE);
        std::memset(fakePage1, 0xFF, PAGE_SIZE);
        std::memset(&fakeFlash, 0, sizeof(fakeFlash));
        fakeFlash.CR = FLASH_CR_LOCK;
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
// Write every single byte address individually (32 tests at key offsets)
// =============================================================================

TEST_F(FlashEepromStressTest, writeByteAt_Offset0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xABU;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xABU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset1)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xCDU;
    EXPECT_EQ(drv.write(1U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xCDU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xEFU;
    EXPECT_EQ(drv.write(2U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xEFU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset3)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x12U;
    EXPECT_EQ(drv.write(3U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(3U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x12U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset4)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x34U;
    EXPECT_EQ(drv.write(4U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(4U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x34U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset7)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x56U;
    EXPECT_EQ(drv.write(7U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(7U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x56U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset8)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x78U;
    EXPECT_EQ(drv.write(8U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(8U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x78U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset15)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x9AU;
    EXPECT_EQ(drv.write(15U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(15U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x9AU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset16)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xBCU;
    EXPECT_EQ(drv.write(16U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(16U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xBCU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset31)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xDEU;
    EXPECT_EQ(drv.write(31U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(31U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xDEU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset32)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xF0U;
    EXPECT_EQ(drv.write(32U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(32U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xF0U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset63)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x11U;
    EXPECT_EQ(drv.write(63U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(63U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x11U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset64)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x22U;
    EXPECT_EQ(drv.write(64U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(64U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x22U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset100)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x33U;
    EXPECT_EQ(drv.write(100U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(100U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x33U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset127)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x44U;
    EXPECT_EQ(drv.write(127U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(127U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x44U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset128)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x55U;
    EXPECT_EQ(drv.write(128U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(128U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x55U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset255)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x66U;
    EXPECT_EQ(drv.write(255U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(255U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x66U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset256)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x77U;
    EXPECT_EQ(drv.write(256U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(256U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x77U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x88U;
    EXPECT_EQ(drv.write(500U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(500U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x88U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset511)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x99U;
    EXPECT_EQ(drv.write(511U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(511U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x99U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset512)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xAAU;
    EXPECT_EQ(drv.write(512U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(512U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xAAU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset1000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xBBU;
    EXPECT_EQ(drv.write(1000U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1000U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset1023)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xCCU;
    EXPECT_EQ(drv.write(1023U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1023U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xCCU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset1024)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xDDU;
    EXPECT_EQ(drv.write(1024U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1024U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xDDU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset1500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xEEU;
    EXPECT_EQ(drv.write(1500U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1500U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xEEU);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x01U;
    EXPECT_EQ(drv.write(2000U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2000U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x01U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2040)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x02U;
    EXPECT_EQ(drv.write(2040U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2040U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x02U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2041)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x03U;
    EXPECT_EQ(drv.write(2041U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2041U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x03U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2042)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x04U;
    EXPECT_EQ(drv.write(2042U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2042U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x04U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset2043_LastByte)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x05U;
    EXPECT_EQ(drv.write(2043U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2043U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x05U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset10)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x06U;
    EXPECT_EQ(drv.write(10U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(10U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x06U);
}

TEST_F(FlashEepromStressTest, writeByteAt_Offset200)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x07U;
    EXPECT_EQ(drv.write(200U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(200U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x07U);
}

// =============================================================================
// Read-after-write consistency for all data patterns (5 patterns x 5 = 25 tests reduced to 5x1)
// =============================================================================

TEST_F(FlashEepromStressTest, rawPattern_0x00)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8];
    std::memset(data, 0x00, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    EXPECT_EQ(drv.read(0U, buf, 8U), bsp::BSP_OK);
    for (int i = 0; i < 8; i++) { EXPECT_EQ(buf[i], 0x00U); }
}

TEST_F(FlashEepromStressTest, rawPattern_0xFF)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8];
    std::memset(data, 0xFF, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    EXPECT_EQ(drv.read(0U, buf, 8U), bsp::BSP_OK);
    for (int i = 0; i < 8; i++) { EXPECT_EQ(buf[i], 0xFFU); }
}

TEST_F(FlashEepromStressTest, rawPattern_0xAA)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8];
    std::memset(data, 0xAA, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    EXPECT_EQ(drv.read(0U, buf, 8U), bsp::BSP_OK);
    for (int i = 0; i < 8; i++) { EXPECT_EQ(buf[i], 0xAAU); }
}

TEST_F(FlashEepromStressTest, rawPattern_0x55)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8];
    std::memset(data, 0x55, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    EXPECT_EQ(drv.read(0U, buf, 8U), bsp::BSP_OK);
    for (int i = 0; i < 8; i++) { EXPECT_EQ(buf[i], 0x55U); }
}

TEST_F(FlashEepromStressTest, rawPattern_0xA5)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[8];
    std::memset(data, 0xA5, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 8U), bsp::BSP_OK);
    uint8_t buf[8];
    EXPECT_EQ(drv.read(0U, buf, 8U), bsp::BSP_OK);
    for (int i = 0; i < 8; i++) { EXPECT_EQ(buf[i], 0xA5U); }
}

// =============================================================================
// Multiple sequential writes (10 writes to same address, verify last value)
// 5 tests
// =============================================================================

TEST_F(FlashEepromStressTest, sequentialWrites_10x_Addr0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = i;
        EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 9U);
}

TEST_F(FlashEepromStressTest, sequentialWrites_10x_Addr100)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0xA0U + i);
        EXPECT_EQ(drv.write(100U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(100U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xA9U);
}

TEST_F(FlashEepromStressTest, sequentialWrites_10x_Addr500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0x50U + i);
        EXPECT_EQ(drv.write(500U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(500U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x59U);
}

TEST_F(FlashEepromStressTest, sequentialWrites_10x_Addr1024)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0x10U + i);
        EXPECT_EQ(drv.write(1024U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1024U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x19U);
}

TEST_F(FlashEepromStressTest, sequentialWrites_10x_Addr2043)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0xF0U + i);
        EXPECT_EQ(drv.write(2043U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(2043U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xF9U);
}

// =============================================================================
// Alternating writes between two different addresses (5 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, alternatingWrites_Addr0And100)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 5U; i++)
    {
        uint8_t d0 = i;
        uint8_t d1 = static_cast<uint8_t>(0x80U + i);
        EXPECT_EQ(drv.write(0U, &d0, 1U), bsp::BSP_OK);
        EXPECT_EQ(drv.write(100U, &d1, 1U), bsp::BSP_OK);
    }
    uint8_t b0 = 0U, b1 = 0U;
    EXPECT_EQ(drv.read(0U, &b0, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(100U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(b0, 4U);
    EXPECT_EQ(b1, 0x84U);
}

TEST_F(FlashEepromStressTest, alternatingWrites_Addr500And1000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 5U; i++)
    {
        uint8_t d0 = static_cast<uint8_t>(0x20U + i);
        uint8_t d1 = static_cast<uint8_t>(0x30U + i);
        EXPECT_EQ(drv.write(500U, &d0, 1U), bsp::BSP_OK);
        EXPECT_EQ(drv.write(1000U, &d1, 1U), bsp::BSP_OK);
    }
    uint8_t b0 = 0U, b1 = 0U;
    EXPECT_EQ(drv.read(500U, &b0, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(1000U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(b0, 0x24U);
    EXPECT_EQ(b1, 0x34U);
}

TEST_F(FlashEepromStressTest, alternatingWrites_Addr0And2043)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 3U; i++)
    {
        uint8_t d0 = static_cast<uint8_t>(0x40U + i);
        uint8_t d1 = static_cast<uint8_t>(0x60U + i);
        EXPECT_EQ(drv.write(0U, &d0, 1U), bsp::BSP_OK);
        EXPECT_EQ(drv.write(2043U, &d1, 1U), bsp::BSP_OK);
    }
    uint8_t b0 = 0U, b1 = 0U;
    EXPECT_EQ(drv.read(0U, &b0, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(2043U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(b0, 0x42U);
    EXPECT_EQ(b1, 0x62U);
}

TEST_F(FlashEepromStressTest, alternatingWrites_Addr10And20)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 4U; i++)
    {
        uint8_t d0 = static_cast<uint8_t>(i * 2U);
        uint8_t d1 = static_cast<uint8_t>(i * 3U);
        EXPECT_EQ(drv.write(10U, &d0, 1U), bsp::BSP_OK);
        EXPECT_EQ(drv.write(20U, &d1, 1U), bsp::BSP_OK);
    }
    uint8_t b0 = 0U, b1 = 0U;
    EXPECT_EQ(drv.read(10U, &b0, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(20U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(b0, 6U);
    EXPECT_EQ(b1, 9U);
}

TEST_F(FlashEepromStressTest, alternatingWrites_Addr1021And1022)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 3U; i++)
    {
        uint8_t d0 = static_cast<uint8_t>(0xA0U + i);
        uint8_t d1 = static_cast<uint8_t>(0xB0U + i);
        EXPECT_EQ(drv.write(1021U, &d0, 1U), bsp::BSP_OK);
        EXPECT_EQ(drv.write(1022U, &d1, 1U), bsp::BSP_OK);
    }
    uint8_t b0 = 0U, b1 = 0U;
    EXPECT_EQ(drv.read(1021U, &b0, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(1022U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(b0, 0xA2U);
    EXPECT_EQ(b1, 0xB2U);
}

// =============================================================================
// Fill entire page data area with pattern, read back (5 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, fillPage_0x00)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0x00, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], 0x00U); }
}

TEST_F(FlashEepromStressTest, fillPage_0xFF)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0xFF, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], 0xFFU); }
}

TEST_F(FlashEepromStressTest, fillPage_0xAA)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0xAA, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], 0xAAU); }
}

TEST_F(FlashEepromStressTest, fillPage_0x55)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0x55, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], 0x55U); }
}

TEST_F(FlashEepromStressTest, fillPage_CountingPattern)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    for (uint32_t i = 0; i < 2044; i++) { data[i] = static_cast<uint8_t>(i & 0xFFU); }
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], static_cast<uint8_t>(i & 0xFFU)); }
}

// =============================================================================
// Write straddling multiple regions (5 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, straddle_Write256BytesAtOffset1000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[256];
    for (uint32_t i = 0; i < 256; i++) { data[i] = static_cast<uint8_t>(i); }
    EXPECT_EQ(drv.write(1000U, data, 256U), bsp::BSP_OK);
    uint8_t buf[256];
    EXPECT_EQ(drv.read(1000U, buf, 256U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 256; i++) { EXPECT_EQ(buf[i], static_cast<uint8_t>(i)); }
}

TEST_F(FlashEepromStressTest, straddle_Write512BytesAtOffset1500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[512];
    std::memset(data, 0xBB, sizeof(data));
    EXPECT_EQ(drv.write(1500U, data, 512U), bsp::BSP_OK);
    uint8_t buf[512];
    EXPECT_EQ(drv.read(1500U, buf, 512U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 512; i++) { EXPECT_EQ(buf[i], 0xBBU); }
}

TEST_F(FlashEepromStressTest, straddle_Write128BytesAtOffset1920)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[124]; // 1920+124=2044 exactly the limit
    std::memset(data, 0xCC, sizeof(data));
    EXPECT_EQ(drv.write(1920U, data, 124U), bsp::BSP_OK);
    uint8_t buf[124];
    EXPECT_EQ(drv.read(1920U, buf, 124U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 124; i++) { EXPECT_EQ(buf[i], 0xCCU); }
}

TEST_F(FlashEepromStressTest, straddle_Write1024BytesAtOffset512)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[1024];
    for (uint32_t i = 0; i < 1024; i++) { data[i] = static_cast<uint8_t>(i ^ 0xAA); }
    EXPECT_EQ(drv.write(512U, data, 1024U), bsp::BSP_OK);
    uint8_t buf[1024];
    EXPECT_EQ(drv.read(512U, buf, 1024U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 1024; i++) { EXPECT_EQ(buf[i], static_cast<uint8_t>(i ^ 0xAA)); }
}

TEST_F(FlashEepromStressTest, straddle_WriteEntireRegionFrom0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    for (uint32_t i = 0; i < 2044; i++) { data[i] = static_cast<uint8_t>((i * 7U) & 0xFFU); }
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++)
    {
        EXPECT_EQ(buf[i], static_cast<uint8_t>((i * 7U) & 0xFFU));
    }
}

// =============================================================================
// Page swap verification: write, check active page flipped (5 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, pageSwap_FirstWriteFlipsToPage1)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    uint32_t magic;
    std::memcpy(&magic, fakePage1, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromStressTest, pageSwap_SecondWriteFlipsBackToPage0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data1 = 0x01U;
    drv.write(0U, &data1, 1U);
    uint8_t data2 = 0x02U;
    drv.write(0U, &data2, 1U);
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromStressTest, pageSwap_ThirdWriteGoesToPage1Again)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x01U;
    drv.write(0U, &data, 1U);
    data = 0x02U;
    drv.write(0U, &data, 1U);
    data = 0x03U;
    drv.write(0U, &data, 1U);
    uint32_t magic;
    std::memcpy(&magic, fakePage1, 4U);
    EXPECT_EQ(magic, MAGIC);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x03U);
}

TEST_F(FlashEepromStressTest, pageSwap_DataPreservedAcrossSwap)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d0 = 0xAAU;
    drv.write(10U, &d0, 1U);
    uint8_t d1 = 0xBBU;
    drv.write(20U, &d1, 1U);
    // After two swaps, addr 10 should still have 0xAA
    uint8_t buf = 0U;
    drv.read(10U, &buf, 1U);
    EXPECT_EQ(buf, 0xAAU);
    drv.read(20U, &buf, 1U);
    EXPECT_EQ(buf, 0xBBU);
}

TEST_F(FlashEepromStressTest, pageSwap_FourWritesAlternate)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    // init => page0 active
    uint8_t d = 0x01U;
    drv.write(0U, &d, 1U); // => page1 active
    d = 0x02U;
    drv.write(0U, &d, 1U); // => page0 active
    d = 0x03U;
    drv.write(0U, &d, 1U); // => page1 active
    d = 0x04U;
    drv.write(0U, &d, 1U); // => page0 active
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
    uint8_t buf = 0U;
    drv.read(0U, &buf, 1U);
    EXPECT_EQ(buf, 0x04U);
}

// =============================================================================
// Multi-swap: 10 consecutive writes, active page alternates correctly (3 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, multiSwap_10Writes_FinalValue)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 10U; i++)
    {
        uint8_t data = i;
        EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 9U);
    // 10 writes from page0 init: even count => page0 active
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromStressTest, multiSwap_7Writes_OddSwap)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 7U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0x10U + i);
        EXPECT_EQ(drv.write(50U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(50U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x16U);
    // 7 writes (odd) => page1 active
    uint32_t magic;
    std::memcpy(&magic, fakePage1, 4U);
    EXPECT_EQ(magic, MAGIC);
}

TEST_F(FlashEepromStressTest, multiSwap_6Writes_EvenSwap)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    for (uint8_t i = 0U; i < 6U; i++)
    {
        uint8_t data = static_cast<uint8_t>(0xC0U + i);
        EXPECT_EQ(drv.write(200U, &data, 1U), bsp::BSP_OK);
    }
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(200U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xC5U);
    // 6 writes (even) => page0 active
    uint32_t magic;
    std::memcpy(&magic, fakePage0, 4U);
    EXPECT_EQ(magic, MAGIC);
}

// =============================================================================
// Read-after-write at various offsets with multi-byte patterns (5 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, rawMultiByte_0x00_AtOffset50)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[16];
    std::memset(data, 0x00, sizeof(data));
    EXPECT_EQ(drv.write(50U, data, 16U), bsp::BSP_OK);
    uint8_t buf[16];
    EXPECT_EQ(drv.read(50U, buf, 16U), bsp::BSP_OK);
    for (int i = 0; i < 16; i++) { EXPECT_EQ(buf[i], 0x00U); }
}

TEST_F(FlashEepromStressTest, rawMultiByte_0xFF_AtOffset300)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[16];
    std::memset(data, 0xFF, sizeof(data));
    EXPECT_EQ(drv.write(300U, data, 16U), bsp::BSP_OK);
    uint8_t buf[16];
    EXPECT_EQ(drv.read(300U, buf, 16U), bsp::BSP_OK);
    for (int i = 0; i < 16; i++) { EXPECT_EQ(buf[i], 0xFFU); }
}

TEST_F(FlashEepromStressTest, rawMultiByte_0xAA_AtOffset700)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[32];
    std::memset(data, 0xAA, sizeof(data));
    EXPECT_EQ(drv.write(700U, data, 32U), bsp::BSP_OK);
    uint8_t buf[32];
    EXPECT_EQ(drv.read(700U, buf, 32U), bsp::BSP_OK);
    for (int i = 0; i < 32; i++) { EXPECT_EQ(buf[i], 0xAAU); }
}

TEST_F(FlashEepromStressTest, rawMultiByte_0x55_AtOffset1500)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[64];
    std::memset(data, 0x55, sizeof(data));
    EXPECT_EQ(drv.write(1500U, data, 64U), bsp::BSP_OK);
    uint8_t buf[64];
    EXPECT_EQ(drv.read(1500U, buf, 64U), bsp::BSP_OK);
    for (int i = 0; i < 64; i++) { EXPECT_EQ(buf[i], 0x55U); }
}

TEST_F(FlashEepromStressTest, rawMultiByte_0xA5_AtOffset2000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[32];
    std::memset(data, 0xA5, sizeof(data));
    EXPECT_EQ(drv.write(2000U, data, 32U), bsp::BSP_OK);
    uint8_t buf[32];
    EXPECT_EQ(drv.read(2000U, buf, 32U), bsp::BSP_OK);
    for (int i = 0; i < 32; i++) { EXPECT_EQ(buf[i], 0xA5U); }
}

// =============================================================================
// Write then erase then write again (3 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, eraseAndRewrite_Offset0)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xFFU);
    data = 0x99U;
    EXPECT_EQ(drv.write(0U, &data, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(0U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x99U);
}

TEST_F(FlashEepromStressTest, eraseAndRewrite_Offset1000)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xBBU;
    EXPECT_EQ(drv.write(1000U, &data, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    data = 0xCCU;
    EXPECT_EQ(drv.write(1000U, &data, 1U), bsp::BSP_OK);
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(1000U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0xCCU);
}

TEST_F(FlashEepromStressTest, eraseAndRewrite_FullPage)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data[2044];
    std::memset(data, 0x77, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    EXPECT_EQ(drv.erase(), bsp::BSP_OK);
    std::memset(data, 0x88, sizeof(data));
    EXPECT_EQ(drv.write(0U, data, 2044U), bsp::BSP_OK);
    uint8_t buf[2044];
    EXPECT_EQ(drv.read(0U, buf, 2044U), bsp::BSP_OK);
    for (uint32_t i = 0; i < 2044; i++) { EXPECT_EQ(buf[i], 0x88U); }
}

// =============================================================================
// Multiple addresses written, all preserved after additional writes (4 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, multiAddress_TwoAddresses_BothPreserved)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0x11U;
    uint8_t d2 = 0x22U;
    EXPECT_EQ(drv.write(0U, &d1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.write(100U, &d2, 1U), bsp::BSP_OK);
    uint8_t b1 = 0U, b2 = 0U;
    EXPECT_EQ(drv.read(0U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(100U, &b2, 1U), bsp::BSP_OK);
    EXPECT_EQ(b1, 0x11U);
    EXPECT_EQ(b2, 0x22U);
}

TEST_F(FlashEepromStressTest, multiAddress_ThreeAddresses_AllPreserved)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0xAAU, d2 = 0xBBU, d3 = 0xCCU;
    EXPECT_EQ(drv.write(0U, &d1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.write(500U, &d2, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.write(1000U, &d3, 1U), bsp::BSP_OK);
    uint8_t b1 = 0U, b2 = 0U, b3 = 0U;
    EXPECT_EQ(drv.read(0U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(500U, &b2, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(1000U, &b3, 1U), bsp::BSP_OK);
    EXPECT_EQ(b1, 0xAAU);
    EXPECT_EQ(b2, 0xBBU);
    EXPECT_EQ(b3, 0xCCU);
}

TEST_F(FlashEepromStressTest, multiAddress_OverwriteOnePreservesOther)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t d1 = 0x11U, d2 = 0x22U;
    EXPECT_EQ(drv.write(0U, &d1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.write(100U, &d2, 1U), bsp::BSP_OK);
    uint8_t d3 = 0x33U;
    EXPECT_EQ(drv.write(0U, &d3, 1U), bsp::BSP_OK);
    uint8_t b1 = 0U, b2 = 0U;
    EXPECT_EQ(drv.read(0U, &b1, 1U), bsp::BSP_OK);
    EXPECT_EQ(drv.read(100U, &b2, 1U), bsp::BSP_OK);
    EXPECT_EQ(b1, 0x33U);
    EXPECT_EQ(b2, 0x22U);
}

TEST_F(FlashEepromStressTest, multiAddress_FourAddresses_AllPreserved)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t vals[4] = {0x10U, 0x20U, 0x30U, 0x40U};
    uint32_t addrs[4] = {0U, 256U, 512U, 1024U};
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(drv.write(addrs[i], &vals[i], 1U), bsp::BSP_OK);
    }
    for (int i = 0; i < 4; i++)
    {
        uint8_t buf = 0U;
        EXPECT_EQ(drv.read(addrs[i], &buf, 1U), bsp::BSP_OK);
        EXPECT_EQ(buf, vals[i]);
    }
}

// =============================================================================
// Zero-length operations (3 tests)
// =============================================================================

TEST_F(FlashEepromStressTest, zeroLength_WriteReturnsOK)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0xFFU;
    EXPECT_EQ(drv.write(0U, &data, 0U), bsp::BSP_OK);
}

TEST_F(FlashEepromStressTest, zeroLength_ReadReturnsOK)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t buf = 0xFFU;
    EXPECT_EQ(drv.read(0U, &buf, 0U), bsp::BSP_OK);
}

TEST_F(FlashEepromStressTest, zeroLength_WriteDoesNotCorruptData)
{
    FlashEepromDriver drv(makeConfig());
    drv.init();
    uint8_t data = 0x42U;
    EXPECT_EQ(drv.write(10U, &data, 1U), bsp::BSP_OK);
    uint8_t dummy = 0x00U;
    drv.write(10U, &dummy, 0U); // zero-length write at same addr
    uint8_t buf = 0U;
    EXPECT_EQ(drv.read(10U, &buf, 1U), bsp::BSP_OK);
    EXPECT_EQ(buf, 0x42U);
}
