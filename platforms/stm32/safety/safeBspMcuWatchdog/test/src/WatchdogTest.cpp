// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   WatchdogTest.cpp
 * \brief  Unit tests for the STM32 Independent Watchdog (IWDG) driver.
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

// Include production code (header + implementation directly)
#include <watchdog/Watchdog.h>
#include <watchdog/Watchdog.cpp>

#include <gtest/gtest.h>

using namespace safety::bsp;

class WatchdogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeIwdg, 0, sizeof(fakeIwdg));
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));
        // Reset the static service counter by calling enable + reading
        // We will use a helper to reset state between tests
    }
};

// =============================================================================
// Helper to access private computePrescalerAndReload via enableWatchdog
// We verify indirectly through the registers written by enableWatchdog.
// =============================================================================

// =============================================================================
// computePrescalerAndReload tests (verified through enableWatchdog PR/RLR)
// =============================================================================

TEST_F(WatchdogTest, enableWatchdog_1ms_32000Hz)
{
    // timeout=1ms, clock=32000 => ticks = (1 * 32) / 4 = 8, prescaler=0, reload=7
    Watchdog::enableWatchdog(1U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 7U);
}

TEST_F(WatchdogTest, enableWatchdog_100ms_32000Hz)
{
    // timeout=100ms, clock=32000 => ticks = (100 * 32) / 4 = 800, prescaler=0, reload=799
    Watchdog::enableWatchdog(100U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 799U);
}

TEST_F(WatchdogTest, enableWatchdog_250ms_32000Hz)
{
    // timeout=250ms, clock=32000 => ticks = (250 * 32) / 4 = 2000, prescaler=0, reload=1999
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogTest, enableWatchdog_500ms_32000Hz)
{
    // timeout=500ms, clock=32000 => ticks = (500 * 32) / 4 = 4000, prescaler=0, reload=3999
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_1000ms_32000Hz)
{
    // timeout=1000ms, clock=32000 => ticks = (1000 * 32) / 4 = 8000 > 4096
    // Try prescaler 1 (/8): ticks = (1000 * 32) / 8 = 4000, prescaler=1, reload=3999
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_2000ms_32000Hz)
{
    // ticks@/4 = 16000; /8 = 8000; /16 = 4000 => prescaler=2, reload=3999
    Watchdog::enableWatchdog(2000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_4000ms_32000Hz)
{
    // ticks@/4=32000; /8=16000; /16=8000; /32=4000 => prescaler=3, reload=3999
    Watchdog::enableWatchdog(4000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_8000ms_32000Hz)
{
    // /64 = 4000 => prescaler=4, reload=3999
    Watchdog::enableWatchdog(8000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 4U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_16000ms_32000Hz)
{
    // /128 = 4000 => prescaler=5, reload=3999
    Watchdog::enableWatchdog(16000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_32000ms_32000Hz)
{
    // /256 = 4000 => prescaler=6, reload=3999
    Watchdog::enableWatchdog(32000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_MaxTimeout_Clamps)
{
    // Extremely large timeout, should clamp to prescaler=6, reload=4095
    Watchdog::enableWatchdog(100000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

// --- Different clock speeds ---

TEST_F(WatchdogTest, enableWatchdog_250ms_40000Hz)
{
    // ticks = (250 * 40) / 4 = 2500, prescaler=0, reload=2499
    Watchdog::enableWatchdog(250U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(WatchdogTest, enableWatchdog_1000ms_40000Hz)
{
    // ticks@/4 = 10000 > 4096; /8 = 5000 > 4096; /16 = 2500 => prescaler=2, reload=2499
    Watchdog::enableWatchdog(1000U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(WatchdogTest, enableWatchdog_100ms_128000Hz)
{
    // ticks = (100 * 128) / 4 = 3200, prescaler=0, reload=3199
    Watchdog::enableWatchdog(100U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 3199U);
}

TEST_F(WatchdogTest, enableWatchdog_250ms_128000Hz)
{
    // ticks@/4 = 8000; /8 = 4000 => prescaler=1, reload=3999
    Watchdog::enableWatchdog(250U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_1000ms_128000Hz)
{
    // ticks@/4=32000; /8=16000; /16=8000; /32=4000 => prescaler=3, reload=3999
    Watchdog::enableWatchdog(1000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, enableWatchdog_500ms_128000Hz)
{
    // ticks@/4=16000; /8=8000; /16=4000 => prescaler=2, reload=3999
    Watchdog::enableWatchdog(500U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

// --- Prescaler code boundary: each prescaler code (0-6) ---

TEST_F(WatchdogTest, prescalerCode0_SmallTimeout)
{
    // 10ms at 32kHz: ticks = (10*32)/4 = 80 => code 0, reload=79
    Watchdog::enableWatchdog(10U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 79U);
}

TEST_F(WatchdogTest, prescalerCode0_AtLimit)
{
    // 512ms at 32kHz: ticks = (512*32)/4 = 4096 => code 0, reload=4095
    Watchdog::enableWatchdog(512U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogTest, prescalerCode1_JustOverCode0)
{
    // 513ms at 32kHz: ticks@/4 = 4104 > 4096; /8 = 2052 => code 1, reload=2051
    Watchdog::enableWatchdog(513U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 2051U);
}

TEST_F(WatchdogTest, prescalerCode2_Boundary)
{
    // 1025ms at 32kHz: ticks@/4=8200; /8=4100>4096; /16=2050 => code 2, reload=2049
    Watchdog::enableWatchdog(1025U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 2U);
    EXPECT_EQ(fakeIwdg.RLR, 2049U);
}

TEST_F(WatchdogTest, prescalerCode3_Boundary)
{
    // 2049ms at 32kHz: ticks = (2049*32)/32 = 2049, reload = 2049-1 = 2048
    Watchdog::enableWatchdog(2049U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 2048U);
}

TEST_F(WatchdogTest, prescalerCode4_Boundary)
{
    // 4097ms at 32kHz: ticks=(4097*32)/64=2048, reload=2048-1=2047
    Watchdog::enableWatchdog(4097U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 4U);
    EXPECT_EQ(fakeIwdg.RLR, 2047U);
}

TEST_F(WatchdogTest, prescalerCode5_Boundary)
{
    // 8193ms at 32kHz: (8193*32)/64=4096 fits code 4; need >4096 to force code 5
    // Use 8194ms: (8194*32)/64=4097>4096 → code 5, ticks=(8194*32)/128=2048, rlr=2047
    Watchdog::enableWatchdog(8194U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 2047U);
}

TEST_F(WatchdogTest, prescalerCode6_Boundary)
{
    // 16385ms: (16385*32)/128=4096 fits code 5; need >4096 to force code 6
    // 16386*32/128=4096 still fits code 5. Use 16389: (16389*32)/128=4100>4096 → code 6
    // ticks=(16389*32)/256=2049, rlr=2048
    Watchdog::enableWatchdog(16389U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 2047U);
}

// =============================================================================
// enableWatchdog: KR unlock/start sequence
// =============================================================================

TEST_F(WatchdogTest, enableWatchdog_WritesUnlockKey)
{
    // We check that KR received the start key last (0xCCCC written, then 0xAAAA from serviceWatchdog)
    Watchdog::enableWatchdog(250U, false, 32000U);
    // After enableWatchdog, the last KR write was serviceWatchdog => 0xAAAA
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, enableWatchdog_SetsPrescaler)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
}

TEST_F(WatchdogTest, enableWatchdog_SetsReload)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogTest, enableWatchdog_DefaultClockSpeed)
{
    Watchdog::enableWatchdog(250U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogTest, enableWatchdog_IgnoresInterruptFlag)
{
    Watchdog::enableWatchdog(250U, true, 32000U);
    // Same result regardless of interruptActive
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

// =============================================================================
// serviceWatchdog tests
// =============================================================================

TEST_F(WatchdogTest, serviceWatchdog_WritesKickKey)
{
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, serviceWatchdog_IncrementsCounter)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 1U);
}

TEST_F(WatchdogTest, serviceWatchdog_MultipleKicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 3U);
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, serviceWatchdog_KRAlwaysKickValue)
{
    fakeIwdg.KR = 0x1234U;
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, serviceWatchdog_TenKicks)
{
    uint32_t before = Watchdog::getWatchdogServiceCounter();
    for (int i = 0; i < 10; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), before + 10U);
}

// =============================================================================
// checkWatchdogConfiguration tests
// =============================================================================

TEST_F(WatchdogTest, checkConfig_MatchingConfig_ReturnsTrue)
{
    // Set up fake IWDG to match 250ms at 32kHz: PR=0, RLR=1999
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 1999U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_MismatchedPR_ReturnsFalse)
{
    fakeIwdg.PR  = 1U; // Wrong prescaler
    fakeIwdg.RLR = 1999U;
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_MismatchedRLR_ReturnsFalse)
{
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 2000U; // Wrong reload
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_BothMismatched_ReturnsFalse)
{
    fakeIwdg.PR  = 3U;
    fakeIwdg.RLR = 500U;
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_1000ms_32kHz_Match)
{
    fakeIwdg.PR  = 1U;
    fakeIwdg.RLR = 3999U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(1000U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_100ms_128kHz_Match)
{
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 3199U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(100U, 128000U));
}

TEST_F(WatchdogTest, checkConfig_DefaultClockSpeed)
{
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 1999U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U));
}

TEST_F(WatchdogTest, checkConfig_ExtraBitsInPR_Masked)
{
    // PR has bits above bit 2 set — should be masked to 3 bits
    fakeIwdg.PR  = 0U | (0xF0U);
    fakeIwdg.RLR = 1999U;
    // checkWatchdogConfiguration masks PR with 0x7
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_ExtraBitsInRLR_Masked)
{
    // RLR has bits above bit 11 set — should be masked to 12 bits
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 1999U | (0xF000U);
    // checkWatchdogConfiguration masks RLR with 0xFFF
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_500ms_40kHz_Match)
{
    // 500ms@40kHz: /4 => ticks=5000>4096; /8=>2500 => PR=1, RLR=2499
    fakeIwdg.PR  = 1U;
    fakeIwdg.RLR = 2499U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(500U, 40000U));
}

TEST_F(WatchdogTest, checkConfig_AfterEnable_Consistent)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(500U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_AfterEnable_DifferentTimeout_False)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
}

// =============================================================================
// isResetFromWatchdog tests
// =============================================================================

TEST_F(WatchdogTest, isResetFromWatchdog_FlagSet)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogTest, isResetFromWatchdog_FlagCleared)
{
    fakeRcc.CSR = 0U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogTest, isResetFromWatchdog_OtherBitsSet)
{
    fakeRcc.CSR = 0xFFFFFFFFU & ~RCC_CSR_IWDGRSTF;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogTest, isResetFromWatchdog_AllBitsSet)
{
    fakeRcc.CSR = 0xFFFFFFFFU;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogTest, isResetFromWatchdog_OnlyIwdgBit)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
}

// =============================================================================
// clearResetFlag tests
// =============================================================================

TEST_F(WatchdogTest, clearResetFlag_SetsRMVF)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

TEST_F(WatchdogTest, clearResetFlag_PreservesOtherBits)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_IWDGRSTF, 0U);
}

TEST_F(WatchdogTest, clearResetFlag_MultipleCalls)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

// =============================================================================
// getWatchdogServiceCounter tests
// =============================================================================

TEST_F(WatchdogTest, getWatchdogServiceCounter_IncrementsOnService)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
}

TEST_F(WatchdogTest, getWatchdogServiceCounter_EnableAlsoKicks)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::enableWatchdog(250U, false, 32000U);
    // enableWatchdog calls serviceWatchdog once at the end
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
}

TEST_F(WatchdogTest, getWatchdogServiceCounter_ConsistentAfterMultipleCalls)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    for (uint32_t i = 0; i < 20; i++)
    {
        Watchdog::serviceWatchdog();
    }
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 20U);
}

// =============================================================================
// disableWatchdog tests (no-op verification)
// =============================================================================

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangeKR)
{
    fakeIwdg.KR = 0x1234U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0x1234U);
}

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangePR)
{
    fakeIwdg.PR = 3U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.PR, 3U);
}

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangeRLR)
{
    fakeIwdg.RLR = 2000U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.RLR, 2000U);
}

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangeSR)
{
    fakeIwdg.SR = 0x3U;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.SR, 0x3U);
}

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangeServiceCounter)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::disableWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev);
}

// =============================================================================
// Edge case: timeout 0
// =============================================================================

TEST_F(WatchdogTest, enableWatchdog_Timeout0)
{
    // timeout=0ms, clock=32000 => ticks = 0, forced to 1, prescaler=0, reload=0
    Watchdog::enableWatchdog(0U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 0U);
}

TEST_F(WatchdogTest, enableWatchdog_Timeout0_128kHz)
{
    Watchdog::enableWatchdog(0U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 0U);
}

// =============================================================================
// Edge case: very large timeout overflows to max
// =============================================================================

TEST_F(WatchdogTest, enableWatchdog_Overflow_50000ms)
{
    Watchdog::enableWatchdog(50000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

TEST_F(WatchdogTest, enableWatchdog_Overflow_128kHz_50000ms)
{
    Watchdog::enableWatchdog(50000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 4095U);
}

// =============================================================================
// Constructor test
// =============================================================================

TEST_F(WatchdogTest, constructor_CallsEnableWatchdog)
{
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog wdt(250U, 32000U);
    // Constructor calls enableWatchdog which calls serviceWatchdog
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);
}

TEST_F(WatchdogTest, constructor_DefaultClock)
{
    Watchdog wdt(100U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 799U);
}

// =============================================================================
// Full sequence tests
// =============================================================================

TEST_F(WatchdogTest, fullSequence_EnableCheckService)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(250U, 32000U));
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, fullSequence_EnableServiceDisableService)
{
    Watchdog::enableWatchdog(500U, false, 32000U);
    Watchdog::serviceWatchdog();
    Watchdog::disableWatchdog(); // no-op
    // Registers should still be from enableWatchdog
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(500U, 32000U));
    Watchdog::serviceWatchdog();
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, fullSequence_CheckResetFlagFlow)
{
    fakeRcc.CSR = RCC_CSR_IWDGRSTF;
    EXPECT_TRUE(Watchdog::isResetFromWatchdog());
    Watchdog::clearResetFlag();
    EXPECT_NE(fakeRcc.CSR & RCC_CSR_RMVF, 0U);
}

// =============================================================================
// Additional prescaler/reload combination tests
// =============================================================================

TEST_F(WatchdogTest, prescaler_5ms_32kHz)
{
    // ticks = (5*32)/4 = 40 => code 0, reload=39
    Watchdog::enableWatchdog(5U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 39U);
}

TEST_F(WatchdogTest, prescaler_50ms_32kHz)
{
    // ticks = (50*32)/4 = 400 => code 0, reload=399
    Watchdog::enableWatchdog(50U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 399U);
}

TEST_F(WatchdogTest, prescaler_200ms_32kHz)
{
    // ticks = (200*32)/4 = 1600 => code 0, reload=1599
    Watchdog::enableWatchdog(200U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1599U);
}

TEST_F(WatchdogTest, prescaler_3000ms_32kHz)
{
    // ticks@/4=24000; /8=12000; /16=6000>4096; /32=3000 => code 3, reload=2999
    Watchdog::enableWatchdog(3000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 2999U);
}

TEST_F(WatchdogTest, prescaler_10000ms_32kHz)
{
    // /64=5000>4096; /128=2500 => code 5, reload=2499
    Watchdog::enableWatchdog(10000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(WatchdogTest, prescaler_20000ms_32kHz)
{
    // /128=5000>4096; /256=2500 => code 6, reload=2499
    Watchdog::enableWatchdog(20000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(WatchdogTest, prescaler_2ms_40kHz)
{
    // ticks = (2*40)/4 = 20 => code 0, reload=19
    Watchdog::enableWatchdog(2U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 19U);
}

TEST_F(WatchdogTest, prescaler_10ms_128kHz)
{
    // ticks = (10*128)/4 = 320 => code 0, reload=319
    Watchdog::enableWatchdog(10U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 319U);
}

TEST_F(WatchdogTest, prescaler_2000ms_128kHz)
{
    // /4=64000; /8=32000; /16=16000; /32=8000; /64=4000 => code 4, reload=3999
    Watchdog::enableWatchdog(2000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 4U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, prescaler_4000ms_128kHz)
{
    // /64=8000; /128=4000 => code 5, reload=3999
    Watchdog::enableWatchdog(4000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 5U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, prescaler_8000ms_128kHz)
{
    // /128=8000; /256=4000 => code 6, reload=3999
    Watchdog::enableWatchdog(8000U, false, 128000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

// =============================================================================
// Re-enable with different timeouts
// =============================================================================

TEST_F(WatchdogTest, reEnable_DifferentTimeout)
{
    Watchdog::enableWatchdog(250U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1999U);

    // Re-enable with larger timeout
    Watchdog::enableWatchdog(1000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

TEST_F(WatchdogTest, reEnable_SmallToLarge)
{
    Watchdog::enableWatchdog(1U, false, 32000U);
    EXPECT_EQ(fakeIwdg.RLR, 7U);

    Watchdog::enableWatchdog(32000U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 6U);
    EXPECT_EQ(fakeIwdg.RLR, 3999U);
}

// =============================================================================
// Additional tests to reach 100+
// =============================================================================

TEST_F(WatchdogTest, enableWatchdog_300ms_32kHz)
{
    // ticks = (300*32)/4 = 2400 => code 0, reload=2399
    Watchdog::enableWatchdog(300U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 2399U);
}

TEST_F(WatchdogTest, enableWatchdog_400ms_32kHz)
{
    // ticks = (400*32)/4 = 3200 => code 0, reload=3199
    Watchdog::enableWatchdog(400U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 3199U);
}

TEST_F(WatchdogTest, enableWatchdog_150ms_40kHz)
{
    // ticks = (150*40)/4 = 1500 => code 0, reload=1499
    Watchdog::enableWatchdog(150U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 1499U);
}

TEST_F(WatchdogTest, enableWatchdog_50ms_40kHz)
{
    // ticks = (50*40)/4 = 500 => code 0, reload=499
    Watchdog::enableWatchdog(50U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 499U);
}

TEST_F(WatchdogTest, enableWatchdog_2000ms_40kHz)
{
    // /4=20000; /8=10000; /16=5000>4096; /32=2500 => code 3, reload=2499
    Watchdog::enableWatchdog(2000U, false, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 3U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}

TEST_F(WatchdogTest, checkConfig_2000ms_40kHz_Match)
{
    fakeIwdg.PR  = 3U;
    fakeIwdg.RLR = 2499U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(2000U, 40000U));
}

TEST_F(WatchdogTest, checkConfig_2000ms_40kHz_WrongPR)
{
    fakeIwdg.PR  = 2U;
    fakeIwdg.RLR = 2499U;
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(2000U, 40000U));
}

TEST_F(WatchdogTest, checkConfig_2000ms_40kHz_WrongRLR)
{
    fakeIwdg.PR  = 3U;
    fakeIwdg.RLR = 2498U;
    EXPECT_FALSE(Watchdog::checkWatchdogConfiguration(2000U, 40000U));
}

TEST_F(WatchdogTest, checkConfig_32000ms_32kHz_Match)
{
    fakeIwdg.PR  = 6U;
    fakeIwdg.RLR = 3999U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(32000U, 32000U));
}

TEST_F(WatchdogTest, checkConfig_1ms_32kHz_Match)
{
    fakeIwdg.PR  = 0U;
    fakeIwdg.RLR = 7U;
    EXPECT_TRUE(Watchdog::checkWatchdogConfiguration(1U, 32000U));
}

TEST_F(WatchdogTest, serviceWatchdog_AfterDisable_StillWorks)
{
    Watchdog::disableWatchdog();
    uint32_t prev = Watchdog::getWatchdogServiceCounter();
    Watchdog::serviceWatchdog();
    EXPECT_EQ(Watchdog::getWatchdogServiceCounter(), prev + 1U);
    EXPECT_EQ(fakeIwdg.KR, 0xAAAAU);
}

TEST_F(WatchdogTest, disableWatchdog_DoesNotChangeWINR)
{
    fakeIwdg.WINR = 0xABCDU;
    Watchdog::disableWatchdog();
    EXPECT_EQ(fakeIwdg.WINR, 0xABCDU);
}

TEST_F(WatchdogTest, isResetFromWatchdog_ZeroCSR)
{
    fakeRcc.CSR = 0U;
    EXPECT_FALSE(Watchdog::isResetFromWatchdog());
}

TEST_F(WatchdogTest, clearResetFlag_InitiallyZero)
{
    fakeRcc.CSR = 0U;
    Watchdog::clearResetFlag();
    EXPECT_EQ(fakeRcc.CSR, RCC_CSR_RMVF);
}

TEST_F(WatchdogTest, enableWatchdog_25ms_32kHz)
{
    // ticks = (25*32)/4 = 200 => code 0, reload=199
    Watchdog::enableWatchdog(25U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 199U);
}

TEST_F(WatchdogTest, enableWatchdog_75ms_32kHz)
{
    // ticks = (75*32)/4 = 600 => code 0, reload=599
    Watchdog::enableWatchdog(75U, false, 32000U);
    EXPECT_EQ(fakeIwdg.PR, 0U);
    EXPECT_EQ(fakeIwdg.RLR, 599U);
}

TEST_F(WatchdogTest, constructor_500ms_40kHz)
{
    Watchdog wdt(500U, 40000U);
    EXPECT_EQ(fakeIwdg.PR, 1U);
    EXPECT_EQ(fakeIwdg.RLR, 2499U);
}
