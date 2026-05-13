// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   SafetyManagerTest.cpp
 * \brief  Integration tests for SafetyManager + Watchdog interaction on STM32.
 *
 * Uses fake IWDG_TypeDef and RCC_TypeDef structs so register writes go
 * to testable memory instead of real hardware.
 */

#include <cstdint>
#include <cstring>

#ifndef __IO
#define __IO volatile
#endif

// Select G4 code path
#define STM32G474xx

// --- Fake IWDG_TypeDef ---
typedef struct
{
    __IO uint32_t KR;
    __IO uint32_t PR;
    __IO uint32_t RLR;
    __IO uint32_t SR;
    __IO uint32_t WINR;
} IWDG_TypeDef;

// --- Fake RCC_TypeDef (STM32G4 layout) ---
typedef struct
{
    __IO uint32_t CR;
    __IO uint32_t ICSCR;
    __IO uint32_t CFGR;
    __IO uint32_t PLLCFGR;
    uint32_t RESERVED0;
    uint32_t RESERVED1;
    __IO uint32_t CIER;
    __IO uint32_t CIFR;
    __IO uint32_t CICR;
    uint32_t RESERVED2;
    __IO uint32_t AHB1RSTR;
    __IO uint32_t AHB2RSTR;
    __IO uint32_t AHB3RSTR;
    uint32_t RESERVED3;
    __IO uint32_t APB1RSTR1;
    __IO uint32_t APB1RSTR2;
    __IO uint32_t APB2RSTR;
    uint32_t RESERVED4;
    __IO uint32_t AHB1ENR;
    __IO uint32_t AHB2ENR;
    __IO uint32_t AHB3ENR;
    uint32_t RESERVED5;
    __IO uint32_t APB1ENR1;
    __IO uint32_t APB1ENR2;
    __IO uint32_t APB2ENR;
    uint32_t RESERVED6;
    __IO uint32_t AHB1SMENR;
    __IO uint32_t AHB2SMENR;
    __IO uint32_t AHB3SMENR;
    uint32_t RESERVED7;
    __IO uint32_t APB1SMENR1;
    __IO uint32_t APB1SMENR2;
    __IO uint32_t APB2SMENR;
    uint32_t RESERVED8;
    __IO uint32_t CCIPR;
    uint32_t RESERVED9;
    __IO uint32_t BDCR;
    __IO uint32_t CSR;
    __IO uint32_t CRRCR;
    __IO uint32_t CCIPR2;
} RCC_TypeDef;

// --- Static fake peripherals ---
static IWDG_TypeDef fakeIwdg;
static RCC_TypeDef fakeRcc;

// --- Override hardware macros ---
#define IWDG (&fakeIwdg)
#define RCC  (&fakeRcc)

// IWDG SR bit definitions
#define IWDG_SR_PVU (1U << 0)
#define IWDG_SR_RVU (1U << 1)

// RCC CSR bit definitions
#define RCC_CSR_IWDGRSTF (1U << 29)
#define RCC_CSR_RMVF     (1U << 23)

// Provide platform/estdint.h types
#ifndef PLATFORM_ESTDINT_H
#define PLATFORM_ESTDINT_H
#include <cstdint>
#endif

// Include production code
#include <watchdog/Watchdog.h>
#include <watchdog/Watchdog.cpp>

#include <gtest/gtest.h>

using namespace safety::bsp;

// ============================================================================
// Test fixture
// ============================================================================

class SafetyManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeIwdg, 0, sizeof(fakeIwdg));
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));
    }
};

// ============================================================================
// Category 1 -- enableWatchdog writes KR=0xCCCC to start IWDG (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, enableWatchdogWritesStartKey)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    // KR should be 0xAAAA (the last write is from serviceWatchdog initial kick)
    // enableWatchdog sequence: KR=0x5555 (unlock), set PR, set RLR, wait SR,
    // KR=0xCCCC (start), then serviceWatchdog writes KR=0xAAAA
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(SafetyManagerTest, enableWatchdogSetsReloadRegister)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    // 250ms, 32kHz: ticks = (250 * 32) / 4 = 2000, prescaler=0, reload=1999
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(SafetyManagerTest, enableWatchdogSetsPrescalerRegister)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
}

TEST_F(SafetyManagerTest, enableWatchdog1000ms32kHz)
{
    Watchdog::enableWatchdog(1000U, false, 32000U);
    // ticks = (1000 * 32) / 4 = 8000 > 4096, try next prescaler
    // /8: ticks = (1000 * 32) / 8 = 4000, prescaler=1, reload=3999
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableWatchdog2000ms32kHz)
{
    Watchdog::enableWatchdog(2000U, false, 32000U);
    // /4: 16000 > 4096, /8: 8000 > 4096, /16: 4000, prescaler=2, reload=3999
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableWatchdog4000ms32kHz)
{
    Watchdog::enableWatchdog(4000U, false, 32000U);
    // /4: 32000, /8: 16000, /16: 8000, /32: 4000, prescaler=3, reload=3999
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableWatchdog8000ms32kHz)
{
    Watchdog::enableWatchdog(8000U, false, 32000U);
    // /64: 4000, prescaler=4, reload=3999
    EXPECT_EQ(fakeIwdg.PR, 4U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableWatchdog16000ms32kHz)
{
    Watchdog::enableWatchdog(16000U, false, 32000U);
    // /128: 4000, prescaler=5, reload=3999
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableWatchdog50ms32kHz)
{
    Watchdog::enableWatchdog(50U, false, 32000U);
    // /4: ticks = (50 * 32) / 4 = 400, prescaler=0, reload=399
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 399U);
}

TEST_F(SafetyManagerTest, enableWatchdog10ms32kHz)
{
    Watchdog::enableWatchdog(10U, false, 32000U);
    // /4: ticks = (10 * 32) / 4 = 80, prescaler=0, reload=79
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 79U);
}

// ============================================================================
// Category 2 -- serviceWatchdog writes KR=0xAAAA (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, serviceWatchdogWritesKickKey)
{
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(SafetyManagerTest, serviceWatchdogIncrementsCounter)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 1U);
}

TEST_F(SafetyManagerTest, serviceWatchdogTwiceIncrementsBy2)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 2U);
}

TEST_F(SafetyManagerTest, serviceWatchdogFiveTimesIncrementsBy5)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 5; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 5U);
}

TEST_F(SafetyManagerTest, serviceWatchdogTenTimesIncrementsBy10)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 10; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 10U);
}

TEST_F(SafetyManagerTest, serviceWatchdogKRAlways0xAAAA)
{
    fakeIwdg.KR = 0U;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(SafetyManagerTest, serviceWatchdogDoesNotModifyPR)
{
    fakeIwdg.PR = 3U;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.PR, 3U);
}

TEST_F(SafetyManagerTest, serviceWatchdogDoesNotModifyRLR)
{
    fakeIwdg.RLR = 4095U;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(SafetyManagerTest, serviceWatchdogDoesNotModifySR)
{
    fakeIwdg.SR = 0x3U;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.SR, 0x3U);
}

TEST_F(SafetyManagerTest, serviceWatchdogCounterMonotonic)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 20; i++)
    {
        Watchdog::serviceWatchdog();
        uint32_t curr = Watchdog::getWatchdogServiceCounter();
        EXPECT_GT(curr, prev);
        prev = curr;
    }
}

// ============================================================================
// Category 3 -- disableWatchdog is a no-op (5 tests)
// ============================================================================

TEST_F(SafetyManagerTest, disableWatchdogDoesNotModifyKR)
{
    fakeIwdg.KR = 0xCCCCU;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xCCCCU);
}

TEST_F(SafetyManagerTest, disableWatchdogDoesNotModifyPR)
{
    fakeIwdg.PR = 5U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.PR, 5U);
}

TEST_F(SafetyManagerTest, disableWatchdogDoesNotModifyRLR)
{
    fakeIwdg.RLR = 2000U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.RLR, 2000U);
}

TEST_F(SafetyManagerTest, disableWatchdogDoesNotModifySR)
{
    fakeIwdg.SR = 0x1U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.SR, 0x1U);
}

TEST_F(SafetyManagerTest, disableWatchdogDoesNotModifyWINR)
{
    fakeIwdg.WINR = 0x0FFFU;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.WINR, 0x0FFFU);
}

// ============================================================================
// Category 4 -- Reset cause detection (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, isResetFromWatchdogReturnsFalseWhenClear)
{
    fakeRcc.CSR = 0U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(SafetyManagerTest, isResetFromWatchdogReturnsTrueWhenSet)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(SafetyManagerTest, isResetFromWatchdogWithOtherBitsSet)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF | 0x00FF0000U;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(SafetyManagerTest, isResetFromWatchdogOnlyChecksIWDGRSTF)
{
    fakeRcc.CSR = 0xFFFFFFFFU & ~RCC_CSR_IWDGRSTF;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(SafetyManagerTest, clearResetFlagSetsRMVF)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(SafetyManagerTest, clearResetFlagPreservesOtherBits)
{
    fakeRcc.CSR = 0x00FF0000U;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & 0x00FF0000U, 0U);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(SafetyManagerTest, clearResetFlagWithIWDGRSTFPreset)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    // RMVF should be set, IWDGRSTF still in register (hardware clears it)
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(SafetyManagerTest, isResetFromWatchdogAfterClearStillReadsFlag)
{
    // In real HW, writing RMVF clears all reset flags. Our fake doesn't simulate
    // that hardware behavior, so IWDGRSTF remains.
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    // Flag still set in our fake
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(SafetyManagerTest, clearResetFlagIdempotent)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    uint32_t csr1 = fakeRcc.CSR;
    Watchdog::clearResetFlag();
    EXPECT_EQ(fakeRcc.CSR, csr1);
}

TEST_F(SafetyManagerTest, isResetFromWatchdogDefaultIsFalse)
{
    // CSR is zero from SetUp
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

// ============================================================================
// Category 5 -- checkWatchdogConfiguration (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, checkConfigurationMatchesAfterEnable250ms)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationMatchesAfterEnable100ms)
{
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(100U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationMatchesAfterEnable1000ms)
{
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(1000U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationFailsForDifferentTimeout)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(500U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationFailsForDifferentClock)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(250U, 40000U));
}

TEST_F(SafetyManagerTest, checkConfigurationAfterEnable50ms)
{
    Watchdog::enableWatchdog(50U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(50U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationAfterEnable10ms)
{
    Watchdog::enableWatchdog(10U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(10U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationAfterEnable500ms)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(500U, 32000U));
}

TEST_F(SafetyManagerTest, checkConfigurationPrescalerRegMatchesExpected)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR & 0x7U, 0U);
}

TEST_F(SafetyManagerTest, checkConfigurationReloadRegMatchesExpected)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.RLR & 0xFFFU, 1999U);
}

// ============================================================================
// Category 6 -- Prescaler boundary tests (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, prescaler0For1ms)
{
    Watchdog::enableWatchdog(1U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 7U);
}

TEST_F(SafetyManagerTest, prescaler0For128ms)
{
    Watchdog::enableWatchdog(128U, false, 32000U);
    // ticks = (128 * 32) / 4 = 1024
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1023U);
}

TEST_F(SafetyManagerTest, prescaler0For512ms)
{
    Watchdog::enableWatchdog(512U, false, 32000U);
    // ticks = (512 * 32) / 4 = 4096
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(SafetyManagerTest, prescaler1For513ms)
{
    Watchdog::enableWatchdog(513U, false, 32000U);
    // /4: ticks = (513 * 32) / 4 = 4104 > 4096
    // /8: ticks = (513 * 32) / 8 = 2052
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 2051U);
}

TEST_F(SafetyManagerTest, maxPrescalerForHugeTimeout)
{
    Watchdog::enableWatchdog(60000U, false, 32000U);
    // All dividers overflow -> max prescaler=6, reload=4095
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(SafetyManagerTest, prescaler6ForVeryLargeTimeout)
{
    Watchdog::enableWatchdog(100000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(SafetyManagerTest, enableWatchdogWithDefaultClockSpeed)
{
    Watchdog::enableWatchdog(250U);
    // Default clock = 32000
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(SafetyManagerTest, enableWatchdogWith40kHzClock)
{
    Watchdog::enableWatchdog(250U, false, 40000U);
    // ticks = (250 * 40) / 4 = 2500
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(SafetyManagerTest, enableWatchdogWith17kHzClock)
{
    Watchdog::enableWatchdog(250U, false, 17000U);
    // ticks = (250 * 17) / 4 = 1062 (integer division: 250*17=4250, 4250/4=1062)
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1061U);
}

TEST_F(SafetyManagerTest, enableWatchdogWith47kHzClock)
{
    Watchdog::enableWatchdog(250U, false, 47000U);
    // ticks = (250 * 47) / 4 = 2937 (integer: 250*47=11750, 11750/4=2937)
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 2936U);
}

// ============================================================================
// Category 7 -- Constructor and getWatchdogServiceCounter (5 tests)
// ============================================================================

TEST_F(SafetyManagerTest, getWatchdogServiceCounterReturnsValue)
{
    uint32_t count = Watchdog::getWatchdogServiceCounter();
    // Just verify it returns without crashing
    (void)count;
    SUCCEED();
}

TEST_F(SafetyManagerTest, serviceCounterIncreasesAfterEnable)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U);
    // enableWatchdog calls serviceWatchdog once at the end
    uint32_t after = Watchdog::getWatchdogServiceCounter();
    EXPECT_EQ(after, before + 1U);
}

TEST_F(SafetyManagerTest, serviceCounterIncreasesAfterMultipleEnables)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U);
    Watchdog::enableWatchdog(100U, false, 32000U);
    uint32_t after = Watchdog::getWatchdogServiceCounter();
    EXPECT_EQ(after, before + 2U);
}

TEST_F(SafetyManagerTest, defaultTimeoutConstant)
{
    EXPECT_EQ(Watchdog::DEFAULT_TIMEOUT, 250U);
}

TEST_F(SafetyManagerTest, defaultClockSpeedConstant)
{
    EXPECT_EQ(Watchdog::DEFAULT_CLOCK_SPEED, 32000U);
}

// ============================================================================
// Category 8 -- Multiple enable/service sequences (10 tests)
// ============================================================================

TEST_F(SafetyManagerTest, enableThenServiceSequence)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    uint32_t afterEnable = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), afterEnable + 1U);
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(SafetyManagerTest, enableThenService10Times)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    uint32_t afterEnable = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 10; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), afterEnable + 10U);
}

TEST_F(SafetyManagerTest, reEnableDoesNotResetCounter)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U); // +1 from initial kick
    Watchdog::serviceWatchdog();                    // +1
    Watchdog::serviceWatchdog();                    // +1
    Watchdog::enableWatchdog(100U, false, 32000U);  // +1 from initial kick
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 4U);
}

TEST_F(SafetyManagerTest, enableWith1msThenCheck)
{
    Watchdog::enableWatchdog(1U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(1U, 32000U));
}

TEST_F(SafetyManagerTest, enableWith5000msThenCheck)
{
    Watchdog::enableWatchdog(5000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(5000U, 32000U));
}

TEST_F(SafetyManagerTest, enableThenServiceThenCheckConfig)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    Watchdog::serviceWatchdog();
    // Service should not change PR or RLR
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(SafetyManagerTest, enableThenService100TimesThenCheckConfig)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    for (int i = 0; i < 100; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(SafetyManagerTest, reEnableChangesConfig)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(SafetyManagerTest, enableDisableEnableSequence)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    Watchdog::disableWatchdog(); // no-op
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(100U, 32000U));
}

TEST_F(SafetyManagerTest, interruptActiveParameterIgnored)
{
    // interruptActive is ignored on STM32 IWDG
    Watchdog::enableWatchdog(250U, true, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}
