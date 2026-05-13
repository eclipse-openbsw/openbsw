// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   AdcTest.cpp
 * \brief  Unit tests for the STM32 ADC driver (Adc class).
 *
 * Uses fake ADC_TypeDef, ADC_Common_TypeDef, and RCC_TypeDef structs so
 * register writes go to testable memory instead of real hardware.
 * Targets the STM32G474xx (G4) code path.
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
#define ADC2          (&fakeAdc1)  // Reuse for simplicity
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

// =============================================================================
// Helper: Simulate hardware behavior for init() and startAndRead().
// The init() code enters while loops waiting for flags. We pre-set the flags
// so the loops exit immediately.
// =============================================================================

class AdcTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeAdc1, 0, sizeof(fakeAdc1));
        std::memset(&fakeAdcCommon, 0, sizeof(fakeAdcCommon));
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));

        // Pre-set flags so that while-loops in init() and startAndRead() exit
        // immediately:
        //   - ADC_CR_ADCAL cleared => calibration loop exits immediately
        //   - ADC_ISR_ADRDY set => enable loop exits immediately
        //   - ADC_ISR_EOC set => conversion complete loop exits immediately
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
// init tests
// =============================================================================

TEST_F(AdcTest, init_EnablesClock)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_ADC12EN, 0U);
}

TEST_F(AdcTest, init_SetsAdcClockSelection)
{
    Adc adc = createDefaultAdc();
    adc.init();
    // CCIPR bits [29:28] = 01 for system clock
    EXPECT_EQ(fakeRcc.CCIPR & (3U << 28), (1U << 28));
}

TEST_F(AdcTest, init_ExitsDeepPowerDown)
{
    fakeAdc1.CR = ADC_CR_DEEPPWD; // Start in deep power-down
    Adc adc = createDefaultAdc();
    adc.init();
    // DEEPPWD should be cleared
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_DEEPPWD, 0U);
}

TEST_F(AdcTest, init_EnablesVoltageRegulator)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADVREGEN, 0U);
}

TEST_F(AdcTest, init_PerformsSingleEndedCalibration)
{
    Adc adc = createDefaultAdc();
    adc.init();
    // ADCALDIF should have been cleared (single-ended cal)
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_ADCALDIF, 0U);
    // ADCAL cleared means calibration finished
    EXPECT_EQ(fakeAdc1.CR & ADC_CR_ADCAL, 0U);
}

TEST_F(AdcTest, init_SetsResolution12bit)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (0U << 3));
}

TEST_F(AdcTest, init_SetsResolution10bit)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (1U << 3));
}

TEST_F(AdcTest, init_SetsResolution8bit)
{
    Adc adc = createAdc(AdcResolution::BITS_8, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (2U << 3));
}

TEST_F(AdcTest, init_SetsResolution6bit)
{
    Adc adc = createAdc(AdcResolution::BITS_6, 4U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (3U << 3));
}

TEST_F(AdcTest, init_SingleConversionMode)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_CONT, 0U);
}

TEST_F(AdcTest, init_SoftwareTrigger)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_EXTEN, 0U);
}

TEST_F(AdcTest, init_RightAligned)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_ALIGN, 0U);
}

TEST_F(AdcTest, init_EnablesAdc)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADEN, 0U);
}

// =============================================================================
// configureChannel tests (via readChannel which calls configureChannel internally)
// =============================================================================

TEST_F(AdcTest, configureChannel_Channel0_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1234U;
    adc.readChannel(0U);
    // SQR1: L=0, SQ1=0
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (0U << 6));
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_L, 0U);
}

TEST_F(AdcTest, configureChannel_Channel1_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(1U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (1U << 6));
}

TEST_F(AdcTest, configureChannel_Channel5_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 200U;
    adc.readChannel(5U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (5U << 6));
}

TEST_F(AdcTest, configureChannel_Channel9_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 300U;
    adc.readChannel(9U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (9U << 6));
}

TEST_F(AdcTest, configureChannel_Channel10_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 400U;
    adc.readChannel(10U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (10U << 6));
}

TEST_F(AdcTest, configureChannel_Channel15_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 500U;
    adc.readChannel(15U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (15U << 6));
}

TEST_F(AdcTest, configureChannel_Channel16_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 600U;
    adc.readChannel(16U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (16U << 6));
}

TEST_F(AdcTest, configureChannel_Channel18_SQR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 700U;
    adc.readChannel(18U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (18U << 6));
}

// --- Sampling time register tests ---

TEST_F(AdcTest, configureChannel_Channel0_SMPR1_SamplingTime4)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(0U);
    // Channel 0 in SMPR1, bits [2:0] = 4
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (4U << 0));
}

TEST_F(AdcTest, configureChannel_Channel3_SMPR1_SamplingTime7)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 7U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(3U);
    // Channel 3 in SMPR1, bits [11:9] = 7
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 9), (7U << 9));
}

TEST_F(AdcTest, configureChannel_Channel5_SMPR1_SamplingTime2)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 2U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(5U);
    // Channel 5 in SMPR1, bits [17:15] = 2
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 15), (2U << 15));
}

TEST_F(AdcTest, configureChannel_Channel9_SMPR1_SamplingTime6)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 6U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(9U);
    // Channel 9 in SMPR1, bits [29:27] = 6
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 27), (6U << 27));
}

TEST_F(AdcTest, configureChannel_Channel10_SMPR2_SamplingTime3)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 3U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(10U);
    // Channel 10 in SMPR2, bits [2:0] = 3
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 0), (3U << 0));
}

TEST_F(AdcTest, configureChannel_Channel13_SMPR2_SamplingTime5)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 5U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(13U);
    // Channel 13 in SMPR2, bits [11:9] = 5
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 9), (5U << 9));
}

TEST_F(AdcTest, configureChannel_Channel16_SMPR2_SamplingTime1)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 1U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(16U);
    // Channel 16 => channel-10=6, SMPR2 bits [20:18] = 1
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 18), (1U << 18));
}

TEST_F(AdcTest, configureChannel_Channel18_SMPR2_SamplingTime0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 0U);
    adc.init();
    fakeAdc1.DR = 100U;
    adc.readChannel(18U);
    // Channel 18 => channel-10=8, SMPR2 bits [26:24] = 0
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 24), 0U);
}

// =============================================================================
// readChannel tests
// =============================================================================

TEST_F(AdcTest, readChannel_ReturnsDRValue)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 2048U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 2048U);
}

TEST_F(AdcTest, readChannel_ReturnsDRValue_4095)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 4095U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 4095U);
}

TEST_F(AdcTest, readChannel_ReturnsDRValue_0)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 0U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 0U);
}

TEST_F(AdcTest, readChannel_BeforeInit_Returns0)
{
    Adc adc = createDefaultAdc();
    // Do NOT call init()
    fakeAdc1.DR = 1234U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 0U);
}

TEST_F(AdcTest, readChannel_Channel0_Value100)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 100U;
    EXPECT_EQ(adc.readChannel(0U), 100U);
}

TEST_F(AdcTest, readChannel_Channel1_Value200)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 200U;
    EXPECT_EQ(adc.readChannel(1U), 200U);
}

TEST_F(AdcTest, readChannel_Channel2_Value300)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 300U;
    EXPECT_EQ(adc.readChannel(2U), 300U);
}

TEST_F(AdcTest, readChannel_Channel3_Value400)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 400U;
    EXPECT_EQ(adc.readChannel(3U), 400U);
}

TEST_F(AdcTest, readChannel_Channel4_Value500)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 500U;
    EXPECT_EQ(adc.readChannel(4U), 500U);
}

TEST_F(AdcTest, readChannel_Channel5_Value600)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 600U;
    EXPECT_EQ(adc.readChannel(5U), 600U);
}

TEST_F(AdcTest, readChannel_Channel6_Value700)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 700U;
    EXPECT_EQ(adc.readChannel(6U), 700U);
}

TEST_F(AdcTest, readChannel_Channel7_Value800)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 800U;
    EXPECT_EQ(adc.readChannel(7U), 800U);
}

TEST_F(AdcTest, readChannel_Channel8_Value900)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 900U;
    EXPECT_EQ(adc.readChannel(8U), 900U);
}

TEST_F(AdcTest, readChannel_Channel9_Value1000)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1000U;
    EXPECT_EQ(adc.readChannel(9U), 1000U);
}

TEST_F(AdcTest, readChannel_Channel10_Value1100)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1100U;
    EXPECT_EQ(adc.readChannel(10U), 1100U);
}

TEST_F(AdcTest, readChannel_Channel11_Value1200)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1200U;
    EXPECT_EQ(adc.readChannel(11U), 1200U);
}

TEST_F(AdcTest, readChannel_Channel12_Value1300)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1300U;
    EXPECT_EQ(adc.readChannel(12U), 1300U);
}

TEST_F(AdcTest, readChannel_Channel13_Value1400)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1400U;
    EXPECT_EQ(adc.readChannel(13U), 1400U);
}

TEST_F(AdcTest, readChannel_Channel14_Value1500)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1500U;
    EXPECT_EQ(adc.readChannel(14U), 1500U);
}

TEST_F(AdcTest, readChannel_Channel15_Value1600)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1600U;
    EXPECT_EQ(adc.readChannel(15U), 1600U);
}

TEST_F(AdcTest, readChannel_Channel16_Value1700)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1700U;
    EXPECT_EQ(adc.readChannel(16U), 1700U);
}

TEST_F(AdcTest, readChannel_Channel17_Value1800)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1800U;
    EXPECT_EQ(adc.readChannel(17U), 1800U);
}

TEST_F(AdcTest, readChannel_Channel18_Value1900)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1900U;
    EXPECT_EQ(adc.readChannel(18U), 1900U);
}

// =============================================================================
// readTemperature tests (G4: channel 16)
// =============================================================================

TEST_F(AdcTest, readTemperature_ReadsChannel16)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 950U;
    uint16_t val = adc.readTemperature();
    EXPECT_EQ(val, 950U);
    // Verify channel 16 was selected in SQR1
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (16U << 6));
}

TEST_F(AdcTest, readTemperature_BeforeInit_Returns0)
{
    Adc adc = createDefaultAdc();
    fakeAdc1.DR = 950U;
    uint16_t val = adc.readTemperature();
    EXPECT_EQ(val, 0U);
}

TEST_F(AdcTest, readTemperature_VariousValues)
{
    Adc adc = createDefaultAdc();
    adc.init();

    fakeAdc1.DR = 0U;
    EXPECT_EQ(adc.readTemperature(), 0U);

    fakeAdc1.DR = 4095U;
    EXPECT_EQ(adc.readTemperature(), 4095U);

    fakeAdc1.DR = 2000U;
    EXPECT_EQ(adc.readTemperature(), 2000U);
}

// =============================================================================
// readVrefint tests (G4: channel 18)
// =============================================================================

TEST_F(AdcTest, readVrefint_ReadsChannel18)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1500U;
    uint16_t val = adc.readVrefint();
    EXPECT_EQ(val, 1500U);
    // Verify channel 18 was selected in SQR1
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (18U << 6));
}

TEST_F(AdcTest, readVrefint_BeforeInit_Returns0)
{
    Adc adc = createDefaultAdc();
    fakeAdc1.DR = 1500U;
    uint16_t val = adc.readVrefint();
    EXPECT_EQ(val, 0U);
}

TEST_F(AdcTest, readVrefint_VariousValues)
{
    Adc adc = createDefaultAdc();
    adc.init();

    fakeAdc1.DR = 0U;
    EXPECT_EQ(adc.readVrefint(), 0U);

    fakeAdc1.DR = 4095U;
    EXPECT_EQ(adc.readVrefint(), 4095U);

    fakeAdc1.DR = 1650U;
    EXPECT_EQ(adc.readVrefint(), 1650U);
}

// =============================================================================
// Resolution tests: verify CFGR_RES field
// =============================================================================

TEST_F(AdcTest, resolution_12bit_CFGR)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 0U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (0U << 3));
}

TEST_F(AdcTest, resolution_10bit_CFGR)
{
    Adc adc = createAdc(AdcResolution::BITS_10, 0U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (1U << 3));
}

TEST_F(AdcTest, resolution_8bit_CFGR)
{
    Adc adc = createAdc(AdcResolution::BITS_8, 0U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (2U << 3));
}

TEST_F(AdcTest, resolution_6bit_CFGR)
{
    Adc adc = createAdc(AdcResolution::BITS_6, 0U);
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_RES, (3U << 3));
}

// =============================================================================
// startAndRead: ADSTART bit and EOC/DR read (tested via readChannel)
// =============================================================================

TEST_F(AdcTest, startAndRead_SetsADSTART)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1000U;
    adc.readChannel(0U);
    // CR should have ADSTART set (it is write-only on real HW; in fake we see it)
    EXPECT_NE(fakeAdc1.CR & ADC_CR_ADSTART, 0U);
}

TEST_F(AdcTest, startAndRead_ClearsEocBeforeStart)
{
    // After readChannel, ISR_EOC was written (set) to clear it before start
    // Then EOC was polled. Since we pre-set ISR with EOC, it exits immediately.
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 500U;
    uint16_t val = adc.readChannel(0U);
    EXPECT_EQ(val, 500U);
}

// =============================================================================
// Multiple sequential reads
// =============================================================================

TEST_F(AdcTest, multipleReads_DifferentChannels)
{
    Adc adc = createDefaultAdc();
    adc.init();

    fakeAdc1.DR = 100U;
    EXPECT_EQ(adc.readChannel(0U), 100U);

    fakeAdc1.DR = 200U;
    EXPECT_EQ(adc.readChannel(5U), 200U);

    fakeAdc1.DR = 300U;
    EXPECT_EQ(adc.readChannel(10U), 300U);

    fakeAdc1.DR = 400U;
    EXPECT_EQ(adc.readChannel(15U), 400U);
}

TEST_F(AdcTest, multipleReads_SameChannel)
{
    Adc adc = createDefaultAdc();
    adc.init();

    fakeAdc1.DR = 111U;
    EXPECT_EQ(adc.readChannel(7U), 111U);

    fakeAdc1.DR = 222U;
    EXPECT_EQ(adc.readChannel(7U), 222U);

    fakeAdc1.DR = 333U;
    EXPECT_EQ(adc.readChannel(7U), 333U);
}

// =============================================================================
// Edge case: readChannel before init returns 0 for all channels
// =============================================================================

TEST_F(AdcTest, readChannel_BeforeInit_Channel0_Returns0)
{
    Adc adc = createDefaultAdc();
    fakeAdc1.DR = 9999U;
    EXPECT_EQ(adc.readChannel(0U), 0U);
}

TEST_F(AdcTest, readChannel_BeforeInit_Channel10_Returns0)
{
    Adc adc = createDefaultAdc();
    fakeAdc1.DR = 9999U;
    EXPECT_EQ(adc.readChannel(10U), 0U);
}

TEST_F(AdcTest, readChannel_BeforeInit_Channel18_Returns0)
{
    Adc adc = createDefaultAdc();
    fakeAdc1.DR = 9999U;
    EXPECT_EQ(adc.readChannel(18U), 0U);
}

// =============================================================================
// Sampling time code 0-7 on same channel
// =============================================================================

TEST_F(AdcTest, samplingTime0_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 0U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), 0U);
}

TEST_F(AdcTest, samplingTime1_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 1U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (1U << 0));
}

TEST_F(AdcTest, samplingTime2_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 2U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (2U << 0));
}

TEST_F(AdcTest, samplingTime3_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 3U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (3U << 0));
}

TEST_F(AdcTest, samplingTime4_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (4U << 0));
}

TEST_F(AdcTest, samplingTime5_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 5U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (5U << 0));
}

TEST_F(AdcTest, samplingTime6_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 6U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (6U << 0));
}

TEST_F(AdcTest, samplingTime7_Channel0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 7U);
    adc.init();
    fakeAdc1.DR = 10U;
    adc.readChannel(0U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 0), (7U << 0));
}

// =============================================================================
// Channel selection: all 19 channels (0-18) set SQR1 correctly
// =============================================================================

TEST_F(AdcTest, channelSelect_2)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(2U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (2U << 6));
}

TEST_F(AdcTest, channelSelect_3)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(3U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (3U << 6));
}

TEST_F(AdcTest, channelSelect_4)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(4U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (4U << 6));
}

TEST_F(AdcTest, channelSelect_6)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(6U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (6U << 6));
}

TEST_F(AdcTest, channelSelect_7)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(7U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (7U << 6));
}

TEST_F(AdcTest, channelSelect_8)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(8U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (8U << 6));
}

TEST_F(AdcTest, channelSelect_11)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(11U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (11U << 6));
}

TEST_F(AdcTest, channelSelect_12)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(12U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (12U << 6));
}

TEST_F(AdcTest, channelSelect_14)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(14U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (14U << 6));
}

TEST_F(AdcTest, channelSelect_17)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(17U);
    EXPECT_EQ(fakeAdc1.SQR1 & ADC_SQR1_SQ1, (17U << 6));
}

// =============================================================================
// Init + multiple reads with changing DR
// =============================================================================

TEST_F(AdcTest, sequentialReads_RampUp)
{
    Adc adc = createDefaultAdc();
    adc.init();

    for (uint16_t v = 0; v < 10; v++)
    {
        fakeAdc1.DR = v * 100U;
        EXPECT_EQ(adc.readChannel(0U), v * 100U);
    }
}

TEST_F(AdcTest, sequentialReads_AllChannelsSameValue)
{
    Adc adc = createDefaultAdc();
    adc.init();

    fakeAdc1.DR = 2048U;
    for (uint8_t ch = 0; ch <= 18; ch++)
    {
        EXPECT_EQ(adc.readChannel(ch), 2048U);
    }
}

// =============================================================================
// Additional tests to reach 100+
// =============================================================================

TEST_F(AdcTest, init_ClockEnableIdempotent)
{
    Adc adc = createDefaultAdc();
    adc.init();
    uint32_t ahb2 = fakeRcc.AHB2ENR;
    // Re-init (create new ADC)
    Adc adc2 = createDefaultAdc();
    adc2.init();
    EXPECT_EQ(fakeRcc.AHB2ENR, ahb2);
}

TEST_F(AdcTest, init_DoesNotSetCONT)
{
    Adc adc = createDefaultAdc();
    adc.init();
    EXPECT_EQ(fakeAdc1.CFGR & ADC_CFGR_CONT, 0U);
}

TEST_F(AdcTest, configureChannel_Channel1_SMPR1_SamplingTime3)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 3U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(1U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 3), (3U << 3));
}

TEST_F(AdcTest, configureChannel_Channel2_SMPR1_SamplingTime6)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 6U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(2U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 6), (6U << 6));
}

TEST_F(AdcTest, configureChannel_Channel4_SMPR1_SamplingTime2)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 2U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(4U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 12), (2U << 12));
}

TEST_F(AdcTest, configureChannel_Channel6_SMPR1_SamplingTime4)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 4U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(6U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 18), (4U << 18));
}

TEST_F(AdcTest, configureChannel_Channel7_SMPR1_SamplingTime1)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 1U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(7U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 21), (1U << 21));
}

TEST_F(AdcTest, configureChannel_Channel8_SMPR1_SamplingTime5)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 5U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(8U);
    EXPECT_EQ(fakeAdc1.SMPR1 & (7U << 24), (5U << 24));
}

TEST_F(AdcTest, configureChannel_Channel11_SMPR2_SamplingTime7)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 7U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(11U);
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 3), (7U << 3));
}

TEST_F(AdcTest, configureChannel_Channel12_SMPR2_SamplingTime0)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 0U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(12U);
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 6), 0U);
}

TEST_F(AdcTest, configureChannel_Channel14_SMPR2_SamplingTime2)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 2U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(14U);
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 12), (2U << 12));
}

TEST_F(AdcTest, configureChannel_Channel15_SMPR2_SamplingTime6)
{
    Adc adc = createAdc(AdcResolution::BITS_12, 6U);
    adc.init();
    fakeAdc1.DR = 1U;
    adc.readChannel(15U);
    EXPECT_EQ(fakeAdc1.SMPR2 & (7U << 15), (6U << 15));
}

TEST_F(AdcTest, readChannel_DRMaxUint16)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 0xFFFFU;
    EXPECT_EQ(adc.readChannel(0U), 0xFFFFU);
}

TEST_F(AdcTest, readChannel_DR1)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1U;
    EXPECT_EQ(adc.readChannel(0U), 1U);
}

TEST_F(AdcTest, readTemperature_AfterInit_Channel16Selected)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1234U;
    adc.readTemperature();
    // SQR1 SQ1 field should be 16
    uint32_t sq1 = (fakeAdc1.SQR1 & ADC_SQR1_SQ1) >> 6;
    EXPECT_EQ(sq1, 16U);
}

TEST_F(AdcTest, readVrefint_AfterInit_Channel18Selected)
{
    Adc adc = createDefaultAdc();
    adc.init();
    fakeAdc1.DR = 1234U;
    adc.readVrefint();
    uint32_t sq1 = (fakeAdc1.SQR1 & ADC_SQR1_SQ1) >> 6;
    EXPECT_EQ(sq1, 18U);
}
