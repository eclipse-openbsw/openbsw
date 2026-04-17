// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   AdcExtendedTest.cpp
 * \brief  Extended tests for the STM32 ADC driver (Adc class).
 *
 * Uses the same fake ADC_TypeDef, ADC_Common_TypeDef, and RCC_TypeDef pattern
 * as AdcTest.cpp.
 */

#include <cstdint>
#include <cstring>

#ifndef __IO
#define __IO volatile
#endif

// Select G4 code path
#define STM32G474xx

// --- Fake ADC_TypeDef (STM32G4 layout) ---
typedef struct
{
    __IO uint32_t ISR;
    __IO uint32_t IER;
    __IO uint32_t CR;
    __IO uint32_t CFGR;
    __IO uint32_t CFGR2;
    __IO uint32_t SMPR1;
    __IO uint32_t SMPR2;
    uint32_t RESERVED1;
    __IO uint32_t TR1;
    __IO uint32_t TR2;
    __IO uint32_t TR3;
    uint32_t RESERVED2;
    __IO uint32_t SQR1;
    __IO uint32_t SQR2;
    __IO uint32_t SQR3;
    __IO uint32_t SQR4;
    __IO uint32_t DR;
    uint32_t RESERVED3[2];
    __IO uint32_t JSQR;
    uint32_t RESERVED4[4];
    __IO uint32_t OFR1;
    __IO uint32_t OFR2;
    __IO uint32_t OFR3;
    __IO uint32_t OFR4;
    uint32_t RESERVED5[4];
    __IO uint32_t JDR1;
    __IO uint32_t JDR2;
    __IO uint32_t JDR3;
    __IO uint32_t JDR4;
    uint32_t RESERVED6[4];
    __IO uint32_t AWD2CR;
    __IO uint32_t AWD3CR;
    uint32_t RESERVED7[2];
    __IO uint32_t DIFSEL;
    __IO uint32_t CALFACT;
} ADC_TypeDef;

// --- Fake ADC_Common_TypeDef ---
typedef struct
{
    __IO uint32_t CSR;
    uint32_t RESERVED;
    __IO uint32_t CCR;
    __IO uint32_t CDR;
} ADC_Common_TypeDef;

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
static ADC_TypeDef fakeAdc1;
static ADC_Common_TypeDef fakeAdcCommon;
static RCC_TypeDef fakeRcc;

// --- Override hardware macros ---
#define ADC1          (&fakeAdc1)
#define ADC2          (&fakeAdc1)
#define ADC12_COMMON  (&fakeAdcCommon)
#define ADC123_COMMON (&fakeAdcCommon)
#define RCC           (&fakeRcc)

// --- ADC register bit definitions (STM32G4) ---
#define ADC_CR_ADEN       (1U << 0)
#define ADC_CR_ADDIS      (1U << 1)
#define ADC_CR_ADSTART    (1U << 2)
#define ADC_CR_ADCAL      (1U << 31)
#define ADC_CR_ADCALDIF   (1U << 30)
#define ADC_CR_DEEPPWD    (1U << 29)
#define ADC_CR_ADVREGEN   (1U << 28)

#define ADC_CFGR_RES      (3U << 3)
#define ADC_CFGR_CONT     (1U << 13)
#define ADC_CFGR_EXTEN    (3U << 10)
#define ADC_CFGR_ALIGN    (1U << 15)

#define ADC_ISR_ADRDY     (1U << 0)
#define ADC_ISR_EOC       (1U << 2)

#define ADC_SQR1_L        (0xFU << 0)
#define ADC_SQR1_SQ1      (0x1FU << 6)

#define RCC_AHB2ENR_ADC12EN (1U << 13)

// Provide platform/estdint.h types
#ifndef PLATFORM_ESTDINT_H
#define PLATFORM_ESTDINT_H
#include <cstdint>
#endif

// Include production code
#include <adc/Adc.h>
#include <adc/Adc.cpp>

#include <gtest/gtest.h>

using namespace bios;

class AdcExtendedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeAdc1, 0, sizeof(fakeAdc1));
        std::memset(&fakeAdcCommon, 0, sizeof(fakeAdcCommon));
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));

        // Pre-set flags so that while-loops exit immediately
        fakeAdc1.ISR = ADC_ISR_ADRDY | ADC_ISR_EOC;
    }

    Adc createDefaultAdc()
    {
        AdcConfig cfg = {ADC1, AdcResolution::BITS_12, 4U};
        return Adc(cfg);
    }

    Adc createAdc(AdcResolution res, uint8_t sampTime)
    {
        AdcConfig cfg = {ADC1, res, sampTime};
        return Adc(cfg);
    }
};

// =============================================================================
// All 19 channels individually via readChannel, verify SQR1 = 19 tests
// =============================================================================

TEST_F(AdcExtendedTest, readChannel_Ch0_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch1_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(1U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (1U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch2_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(2U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (2U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch3_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(3U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (3U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch4_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(4U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (4U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch5_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(5U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (5U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch6_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(6U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (6U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch7_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(7U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (7U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch8_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(8U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (8U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch9_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(9U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (9U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch10_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(10U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (10U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch11_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(11U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (11U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch12_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(12U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (12U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch13_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(13U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (13U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch14_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(14U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (14U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch15_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(15U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (15U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch16_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(16U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (16U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch17_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(17U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (17U << 6));
}

TEST_F(AdcExtendedTest, readChannel_Ch18_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(18U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (18U << 6));
}

// =============================================================================
// All 8 sampling time codes on channel 0, verify SMPR = 8 tests
// =============================================================================

TEST_F(AdcExtendedTest, samplingTime_Code0_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 0U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (0U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code1_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 1U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (1U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code2_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 2U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (2U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code3_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 3U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (3U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code4_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (4U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code5_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 5U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (5U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code6_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 6U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (6U << 0));
}

TEST_F(AdcExtendedTest, samplingTime_Code7_Ch0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 7U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (7U << 0));
}

// =============================================================================
// Resolution to DR range: 12-bit max 4095, 10-bit max 1023, etc (4 tests)
// =============================================================================

TEST_F(AdcExtendedTest, resolution_12bit_Max4095)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    fakeAdc1.DR = 4095U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 4095U);
}

TEST_F(AdcExtendedTest, resolution_10bit_Max1023)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 4U);
    adc.init();
    fakeAdc1.DR = 1023U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 1023U);
}

TEST_F(AdcExtendedTest, resolution_8bit_Max255)
{
    Adc adc = createAdc(AdcResolution::BITS_8, 4U);
    adc.init();
    fakeAdc1.DR = 255U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 255U);
}

TEST_F(AdcExtendedTest, resolution_6bit_Max63)
{
    Adc adc = createAdc(AdcResolution::BITS_6, 4U);
    adc.init();
    fakeAdc1.DR = 63U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 63U);
}

// =============================================================================
// Multiple reads on same channel: DR changes between reads (5 tests)
// =============================================================================

TEST_F(AdcExtendedTest, multipleReads_DRChanges_1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    EXPECT_EQ(adc.readChannel(0U), 100U);
    fakeAdc1.DR = 200U;
    EXPECT_EQ(adc.readChannel(0U), 200U);
}

TEST_F(AdcExtendedTest, multipleReads_DRChanges_2)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 0U;
    EXPECT_EQ(adc.readChannel(0U), 0U);
    fakeAdc1.DR = 4095U;
    EXPECT_EQ(adc.readChannel(0U), 4095U);
}

TEST_F(AdcExtendedTest, multipleReads_DRChanges_3)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1000U;
    EXPECT_EQ(adc.readChannel(5U), 1000U);
    fakeAdc1.DR = 2000U;
    EXPECT_EQ(adc.readChannel(5U), 2000U);
    fakeAdc1.DR = 3000U;
    EXPECT_EQ(adc.readChannel(5U), 3000U);
}

TEST_F(AdcExtendedTest, multipleReads_DRChanges_4)
{
    Adc adc = createDefaultAdc();
    adc.init();
    for (uint16_t i = 0; i < 5; i++)
    {
        fakeAdc1.DR = i * 800U;
        EXPECT_EQ(adc.readChannel(3U), i * 800U);
    }
}

TEST_F(AdcExtendedTest, multipleReads_DRChanges_5)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 4095U;
    EXPECT_EQ(adc.readChannel(10U), 4095U);
    fakeAdc1.DR = 0U;
    EXPECT_EQ(adc.readChannel(10U), 0U);
    fakeAdc1.DR = 2048U;
    EXPECT_EQ(adc.readChannel(10U), 2048U);
}

// =============================================================================
// Channel switching: read ch0 then ch5 then ch16, verify SQR1 updated (5 tests)
// =============================================================================

TEST_F(AdcExtendedTest, channelSwitch_0_5_16)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
    adc.readChannel(5U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (5U << 6));
    adc.readChannel(16U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (16U << 6));
}

TEST_F(AdcExtendedTest, channelSwitch_18_0_9)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(18U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (18U << 6));
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
    adc.readChannel(9U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (9U << 6));
}

TEST_F(AdcExtendedTest, channelSwitch_7_8_15)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(7U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (7U << 6));
    adc.readChannel(8U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (8U << 6));
    adc.readChannel(15U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (15U << 6));
}

TEST_F(AdcExtendedTest, channelSwitch_1_2_3_4)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(1U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (1U << 6));
    adc.readChannel(2U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (2U << 6));
    adc.readChannel(3U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (3U << 6));
    adc.readChannel(4U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (4U << 6));
}

TEST_F(AdcExtendedTest, channelSwitch_BackAndForth)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
    adc.readChannel(18U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (18U << 6));
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
}

// =============================================================================
// Init state: ADVREGEN set, DEEPPWD clear, ADCAL cleared = 5 tests
// =============================================================================

TEST_F(AdcExtendedTest, initState_ADVREGEN_Set)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADVREGEN, 0U);
}

TEST_F(AdcExtendedTest, initState_DEEPPWD_Cleared)
{
    fakeAdc1.CR = ADC_CR_DEEPPWD;
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_DEEPPWD, 0U);
}

TEST_F(AdcExtendedTest, initState_ADCAL_Cleared)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_ADCAL, 0U);
}

TEST_F(AdcExtendedTest, initState_ADCALDIF_Cleared)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_ADCALDIF, 0U);
}

TEST_F(AdcExtendedTest, initState_ClockEnabled)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_ADC12EN, 0U);
}

// =============================================================================
// ADC enable: ADEN set, ADRDY waited for = 5 tests
// =============================================================================

TEST_F(AdcExtendedTest, enable_ADEN_Set)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

TEST_F(AdcExtendedTest, enable_12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

TEST_F(AdcExtendedTest, enable_10bit)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 4U);
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

TEST_F(AdcExtendedTest, enable_8bit)
{
    Adc adc = createAdc(AdcResolution::BITS_8, 4U);
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

TEST_F(AdcExtendedTest, enable_6bit)
{
    Adc adc = createAdc(AdcResolution::BITS_6, 4U);
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

// =============================================================================
// CFGR: CONT clear, EXTEN clear, ALIGN clear = 5 tests
// =============================================================================

TEST_F(AdcExtendedTest, cfgr_CONT_Clear_12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_CONT, 0U);
}

TEST_F(AdcExtendedTest, cfgr_EXTEN_Clear_12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_EXTEN, 0U);
}

TEST_F(AdcExtendedTest, cfgr_ALIGN_Clear_12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_ALIGN, 0U);
}

TEST_F(AdcExtendedTest, cfgr_CONT_Clear_6bit)
{
    Adc adc = createAdc(AdcResolution::BITS_6, 2U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_CONT, 0U);
}

TEST_F(AdcExtendedTest, cfgr_EXTEN_Clear_10bit)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 3U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_EXTEN, 0U);
}

// =============================================================================
// CCIPR ADC clock select bits (3 tests)
// =============================================================================

TEST_F(AdcExtendedTest, ccipr_AdcClockSelect_12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_EQ(fakeRcc.CCIPR & (3U << 28), (1U << 28));
}

TEST_F(AdcExtendedTest, ccipr_AdcClockSelect_10bit)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 4U);
    adc.init();
    EXPECT_EQ(fakeRcc.CCIPR & (3U << 28), (1U << 28));
}

TEST_F(AdcExtendedTest, ccipr_AdcClockSelect_8bit)
{
    Adc adc = createAdc(AdcResolution::BITS_8, 4U);
    adc.init();
    EXPECT_EQ(fakeRcc.CCIPR & (3U << 28), (1U << 28));
}

// =============================================================================
// readChannel returns DR value exactly (5 tests)
// =============================================================================

TEST_F(AdcExtendedTest, readChannel_ReturnsDR_0)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 0U;
    EXPECT_EQ(adc.readChannel(0U), 0U);
}

TEST_F(AdcExtendedTest, readChannel_ReturnsDR_1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    EXPECT_EQ(adc.readChannel(0U), 1U);
}

TEST_F(AdcExtendedTest, readChannel_ReturnsDR_2048)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 2048U;
    EXPECT_EQ(adc.readChannel(0U), 2048U);
}

TEST_F(AdcExtendedTest, readChannel_ReturnsDR_4094)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 4094U;
    EXPECT_EQ(adc.readChannel(0U), 4094U);
}

TEST_F(AdcExtendedTest, readChannel_ReturnsDR_4095)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 4095U;
    EXPECT_EQ(adc.readChannel(0U), 4095U);
}

// =============================================================================
// Sequence: init -> read -> read -> read (multiple reads) (3 tests)
// =============================================================================

TEST_F(AdcExtendedTest, sequence_InitThenThreeReads_SameChannel)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    EXPECT_EQ(adc.readChannel(5U), 100U);
    fakeAdc1.DR = 200U;
    EXPECT_EQ(adc.readChannel(5U), 200U);
    fakeAdc1.DR = 300U;
    EXPECT_EQ(adc.readChannel(5U), 300U);
}

TEST_F(AdcExtendedTest, sequence_InitThenThreeReads_DifferentChannels)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1000U;
    EXPECT_EQ(adc.readChannel(0U), 1000U);
    fakeAdc1.DR = 2000U;
    EXPECT_EQ(adc.readChannel(10U), 2000U);
    fakeAdc1.DR = 3000U;
    EXPECT_EQ(adc.readChannel(18U), 3000U);
}

TEST_F(AdcExtendedTest, sequence_InitThenFiveReads)
{
    Adc adc = createDefaultAdc();
    adc.init();
    for (uint8_t ch = 0; ch < 5; ch++)
    {
        fakeAdc1.DR = static_cast<uint32_t>(ch * 500U);
        EXPECT_EQ(adc.readChannel(ch), ch * 500U);
    }
}

// =============================================================================
// No crash on readChannel(0) through readChannel(18) in sequence (3 tests)
// =============================================================================

TEST_F(AdcExtendedTest, noCrash_AllChannelsForward)
{
    Adc adc = createDefaultAdc();
    adc.init();
    for (uint8_t ch = 0; ch <= 18; ch++)
    {
        fakeAdc1.DR = static_cast<uint32_t>(ch * 100U);
        uint16_t val = adc.readChannel(ch);
        EXPECT_EQ(val, ch * 100U);
    }
}

TEST_F(AdcExtendedTest, noCrash_AllChannelsReverse)
{
    Adc adc = createDefaultAdc();
    adc.init();
    for (int ch = 18; ch >= 0; ch--)
    {
        fakeAdc1.DR = static_cast<uint32_t>(ch * 50U);
        uint16_t val = adc.readChannel(static_cast<uint8_t>(ch));
        EXPECT_EQ(val, static_cast<uint16_t>(ch * 50U));
    }
}

TEST_F(AdcExtendedTest, noCrash_AllChannelsTwice)
{
    Adc adc = createDefaultAdc();
    adc.init();
    for (int pass = 0; pass < 2; pass++)
    {
        for (uint8_t ch = 0; ch <= 18; ch++)
        {
            fakeAdc1.DR = static_cast<uint32_t>(ch + pass * 19U);
            uint16_t val = adc.readChannel(ch);
            EXPECT_EQ(val, static_cast<uint16_t>(ch + pass * 19U));
        }
    }
}
