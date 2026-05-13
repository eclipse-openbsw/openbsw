// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   WatchdogStressTest.cpp
 * \brief  Stress and boundary-condition tests for the STM32 IWDG driver.
 *
 * Uses the same fake IWDG_TypeDef and RCC_TypeDef pattern as WatchdogTest.cpp.
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

class WatchdogStressTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeIwdg, 0, sizeof(fakeIwdg));
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));
    }
};

// =============================================================================
// Service counter after N kicks (N=1,2,5,10,50,100) = 6 tests
// =============================================================================

TEST_F(WatchdogStressTest, serviceCounter_After1Kick)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 1U);
}

TEST_F(WatchdogStressTest, serviceCounter_After2Kicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 2U);
}

TEST_F(WatchdogStressTest, serviceCounter_After5Kicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 5; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 5U);
}

TEST_F(WatchdogStressTest, serviceCounter_After10Kicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 10; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 10U);
}

TEST_F(WatchdogStressTest, serviceCounter_After50Kicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 50; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 50U);
}

TEST_F(WatchdogStressTest, serviceCounter_After100Kicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 100; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 100U);
}

// =============================================================================
// enableWatchdog with every standard timeout, verify PR and RLR (13 tests)
// =============================================================================

TEST_F(WatchdogStressTest, enable_10ms)
{
    // 10ms@32kHz: ticks=(10*32)/4=80 => PR=0, RLR=79
    Watchdog::enableWatchdog(10U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 79U);
}

TEST_F(WatchdogStressTest, enable_25ms)
{
    // 25ms@32kHz: ticks=(25*32)/4=200 => PR=0, RLR=199
    Watchdog::enableWatchdog(25U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 199U);
}

TEST_F(WatchdogStressTest, enable_50ms)
{
    // 50ms@32kHz: ticks=(50*32)/4=400 => PR=0, RLR=399
    Watchdog::enableWatchdog(50U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 399U);
}

TEST_F(WatchdogStressTest, enable_100ms)
{
    // 100ms@32kHz: ticks=800 => PR=0, RLR=799
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 799U);
}

TEST_F(WatchdogStressTest, enable_200ms)
{
    // 200ms@32kHz: ticks=1600 => PR=0, RLR=1599
    Watchdog::enableWatchdog(200U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1599U);
}

TEST_F(WatchdogStressTest, enable_250ms)
{
    // 250ms@32kHz: ticks=2000 => PR=0, RLR=1999
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogStressTest, enable_500ms)
{
    // 500ms@32kHz: ticks=4000 => PR=0, RLR=3999
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_1000ms)
{
    // 1000ms@32kHz: ticks@/4=8000>4096; /8=4000 => PR=1, RLR=3999
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_2000ms)
{
    // /16=4000 => PR=2, RLR=3999
    Watchdog::enableWatchdog(2000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_4000ms)
{
    // /32=4000 => PR=3, RLR=3999
    Watchdog::enableWatchdog(4000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_8000ms)
{
    // /64=4000 => PR=4, RLR=3999
    Watchdog::enableWatchdog(8000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 4U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_16000ms)
{
    // /128=4000 => PR=5, RLR=3999
    Watchdog::enableWatchdog(16000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, enable_32000ms)
{
    // /256=4000 => PR=6, RLR=3999
    Watchdog::enableWatchdog(32000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

// =============================================================================
// checkWatchdogConfiguration after each enable = 13 tests
// =============================================================================

TEST_F(WatchdogStressTest, checkAfterEnable_10ms)
{
    Watchdog::enableWatchdog(10U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(10U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_25ms)
{
    Watchdog::enableWatchdog(25U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(25U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_50ms)
{
    Watchdog::enableWatchdog(50U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(50U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_100ms)
{
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(100U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_200ms)
{
    Watchdog::enableWatchdog(200U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(200U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_250ms)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_500ms)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(500U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_1000ms)
{
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(1000U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_2000ms)
{
    Watchdog::enableWatchdog(2000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(2000U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_4000ms)
{
    Watchdog::enableWatchdog(4000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(4000U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_8000ms)
{
    Watchdog::enableWatchdog(8000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(8000U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_16000ms)
{
    Watchdog::enableWatchdog(16000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(16000U, 32000U));
}

TEST_F(WatchdogStressTest, checkAfterEnable_32000ms)
{
    Watchdog::enableWatchdog(32000U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(32000U, 32000U));
}

// =============================================================================
// Service then check: kick doesn't change PR/RLR = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, serviceThenCheck_250ms)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    uint32_t prBefore = fakeIwdg.PR;
    uint32_t rlrBefore = fakeIwdg.RLR;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.PR, prBefore);
    EXPECT_EQ(fakeIwdg.RLR, rlrBefore);
}

TEST_F(WatchdogStressTest, serviceThenCheck_500ms)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    uint32_t prBefore = fakeIwdg.PR;
    uint32_t rlrBefore = fakeIwdg.RLR;
    for (int i = 0; i < 5; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(fakeIwdg.PR, prBefore);
    EXPECT_EQ(fakeIwdg.RLR, rlrBefore);
}

TEST_F(WatchdogStressTest, serviceThenCheck_1000ms)
{
    Watchdog::enableWatchdog(1000U, false, 32000U);
    uint32_t prBefore = fakeIwdg.PR;
    uint32_t rlrBefore = fakeIwdg.RLR;
    for (int i = 0; i < 10; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(fakeIwdg.PR, prBefore);
    EXPECT_EQ(fakeIwdg.RLR, rlrBefore);
}

TEST_F(WatchdogStressTest, serviceThenCheck_4000ms)
{
    Watchdog::enableWatchdog(4000U, false, 32000U);
    uint32_t prBefore = fakeIwdg.PR;
    uint32_t rlrBefore = fakeIwdg.RLR;
    for (int i = 0; i < 20; i++) { Watchdog::serviceWatchdog(); }
    EXPECT_EQ(fakeIwdg.PR, prBefore);
    EXPECT_EQ(fakeIwdg.RLR, rlrBefore);
}

TEST_F(WatchdogStressTest, serviceThenCheck_100ms)
{
    Watchdog::enableWatchdog(100U, false, 32000U);
    uint32_t prBefore = fakeIwdg.PR;
    uint32_t rlrBefore = fakeIwdg.RLR;
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.PR, prBefore);
    EXPECT_EQ(fakeIwdg.RLR, rlrBefore);
}

// =============================================================================
// Multiple enable calls: last one wins = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, multipleEnable_250Then500_LastWins)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, multipleEnable_500Then100_LastWins)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 799U);
}

TEST_F(WatchdogStressTest, multipleEnable_1000Then250_LastWins)
{
    Watchdog::enableWatchdog(1000U, false, 32000U);
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogStressTest, multipleEnable_ThreeTimes_LastWins)
{
    Watchdog::enableWatchdog(100U, false, 32000U);
    Watchdog::enableWatchdog(1000U, false, 32000U);
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogStressTest, multipleEnable_SameTimeout_NoChange)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    uint32_t pr1 = fakeIwdg.PR;
    uint32_t rlr1 = fakeIwdg.RLR;
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, pr1);
    EXPECT_EQ(fakeIwdg.RLR, rlr1);
}

// =============================================================================
// Reset flag lifecycle: set, detect, clear, verify cleared = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, resetFlagLifecycle_Basic)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, resetFlagLifecycle_NoFlagInitially)
{
    fakeRcc.CSR = 0U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, resetFlagLifecycle_ClearPreservesIwdgBit)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    // clearResetFlag ORs RMVF — it does not clear IWDGRSTF (that's hw)
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_IWDGRSTF, 0U);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, resetFlagLifecycle_MultipleClearCalls)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    Watchdog::clearResetFlag();
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, resetFlagLifecycle_OtherBitsPreserved)
{
    fakeRcc.CSR = 0x12345678U | RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    // Other bits should still be set
    EXPECT_NE(fakeRcc.CSR & 0x12345678U, 0U);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

// =============================================================================
// isResetFromWatchdog with various CSR bit patterns = 10 tests
// =============================================================================

TEST_F(WatchdogStressTest, isReset_OnlyIwdgBit)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_AllZero)
{
    fakeRcc.CSR = 0U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_AllOnes)
{
    fakeRcc.CSR = 0xFFFFFFFFU;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_AllExceptIwdg)
{
    fakeRcc.CSR = 0xFFFFFFFFU & ~RCC_CSR_IWDGRSTF;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_Bit0Set)
{
    fakeRcc.CSR = 1U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_Bit28Set)
{
    fakeRcc.CSR = (1U << 28);
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_Bit30Set)
{
    fakeRcc.CSR = (1U << 30);
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_IwdgAndRmvfBothSet)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF | RCC_CSR_RMVF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_LowBitsAndIwdg)
{
    fakeRcc.CSR = 0x0000FFFFU | RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogStressTest, isReset_HighBitsNoIwdg)
{
    fakeRcc.CSR = 0xD0000000U & ~RCC_CSR_IWDGRSTF;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

// =============================================================================
// clearResetFlag preserves other CSR bits = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, clearFlag_PreservesBit0)
{
    fakeRcc.CSR = 1U;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & 1U, 0U);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, clearFlag_PreservesLowWord)
{
    fakeRcc.CSR = 0x0000FFFFU;
    Watchdog::clearResetFlag();
    EXPECT_EQ(fakeRcc.CSR & 0x0000FFFFU, 0x0000FFFFU);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogStressTest, clearFlag_PreservesIwdgBit)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_IWDGRSTF, 0U);
}

TEST_F(WatchdogStressTest, clearFlag_PreservesAllBits)
{
    fakeRcc.CSR = 0xFFFFFFFFU;
    Watchdog::clearResetFlag();
    // RMVF was already set; all bits should remain
    EXPECT_EQ(fakeRcc.CSR, 0xFFFFFFFFU);
}

TEST_F(WatchdogStressTest, clearFlag_SetsRmvfOnZero)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    EXPECT_EQ(fakeRcc.CSR, RCC_CSR_RMVF);
}

// =============================================================================
// disableWatchdog doesn't touch KR/PR/RLR = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, disable_DoesNotChangeKR)
{
    fakeIwdg.KR = 0xBEEFU;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xBEEFU);
}

TEST_F(WatchdogStressTest, disable_DoesNotChangePR)
{
    fakeIwdg.PR = 5U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.PR, 5U);
}

TEST_F(WatchdogStressTest, disable_DoesNotChangeRLR)
{
    fakeIwdg.RLR = 4095U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogStressTest, disable_DoesNotChangeSR)
{
    fakeIwdg.SR = 0x7U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.SR, 0x7U);
}

TEST_F(WatchdogStressTest, disable_DoesNotChangeWINR)
{
    fakeIwdg.WINR = 0x0FFFU;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.WINR, 0x0FFFU);
}

// =============================================================================
// getWatchdogServiceCounter thread safety: increment from initial = 5 tests
// =============================================================================

TEST_F(WatchdogStressTest, counterIncrement_SingleKick)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
}

TEST_F(WatchdogStressTest, counterIncrement_EnablePlusKick)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U); // +1
    Watchdog::serviceWatchdog(); // +1
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 2U);
}

TEST_F(WatchdogStressTest, counterIncrement_TwoEnables)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(100U, false, 32000U); // +1
    Watchdog::enableWatchdog(500U, false, 32000U); // +1
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 2U);
}

TEST_F(WatchdogStressTest, counterIncrement_EnableAndMultipleKicks)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U); // +1
    for (int i = 0; i < 10; i++) { Watchdog::serviceWatchdog(); } // +10
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 11U);
}

TEST_F(WatchdogStressTest, counterIncrement_DisableDoesNotIncrement)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::disableWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev);
}

// =============================================================================
// Boundary: prescaler code 0 (div4) vs code 6 (div256) reload values = 8 tests
// =============================================================================

TEST_F(WatchdogStressTest, boundary_Div4_MinReload)
{
    // 1ms@32kHz: ticks=(1*32)/4=8 => PR=0, RLR=7
    Watchdog::enableWatchdog(1U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 7U);
}

TEST_F(WatchdogStressTest, boundary_Div4_MaxReload)
{
    // 512ms@32kHz: ticks=(512*32)/4=4096 => PR=0, RLR=4095
    Watchdog::enableWatchdog(512U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogStressTest, boundary_Div8_JustOverDiv4)
{
    // 513ms@32kHz: ticks@/4=4104>4096; /8=2052 => PR=1, RLR=2051
    Watchdog::enableWatchdog(513U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 2051U);
}

TEST_F(WatchdogStressTest, boundary_Div256_4000Reload)
{
    // 32000ms@32kHz: /256=4000 => PR=6, RLR=3999
    Watchdog::enableWatchdog(32000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogStressTest, boundary_Div256_MaxReload)
{
    // Huge timeout => clamps to PR=6, RLR=4095
    Watchdog::enableWatchdog(100000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogStressTest, boundary_Div4_128kHz_MaxReload)
{
    // 128ms@128kHz: ticks=(128*128)/4=4096 => PR=0, RLR=4095
    Watchdog::enableWatchdog(128U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogStressTest, boundary_Div8_128kHz_JustOver)
{
    // 129ms@128kHz: ticks@/4=4128>4096; /8=2064 => PR=1, RLR=2063
    Watchdog::enableWatchdog(129U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 2063U);
}

TEST_F(WatchdogStressTest, boundary_Div256_128kHz_Overflow)
{
    // Huge timeout@128kHz => clamps to PR=6, RLR=4095
    Watchdog::enableWatchdog(200000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}
