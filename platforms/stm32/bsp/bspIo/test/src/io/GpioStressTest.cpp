// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   GpioStressTest.cpp
 * \brief  Stress and boundary-condition tests for the STM32 GPIO abstraction.
 *
 * Uses the same fake GPIO_TypeDef and RCC_TypeDef pattern as GpioTest.cpp.
 */

#include <cstdint>
#include <cstring>

#ifndef __IO
#define __IO volatile
#endif

// --- Fake GPIO_TypeDef (matches STM32 layout) ---
typedef struct
{
    __IO uint32_t MODER;
    __IO uint32_t OTYPER;
    __IO uint32_t OSPEEDR;
    __IO uint32_t PUPDR;
    __IO uint32_t IDR;
    __IO uint32_t ODR;
    __IO uint32_t BSRR;
    __IO uint32_t LCKR;
    __IO uint32_t AFR[2];
    __IO uint32_t BRR;
} GPIO_TypeDef;

// --- Fake RCC_TypeDef (minimal for GPIO clock enable) ---
typedef struct
{
    uint32_t pad[16];
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
} RCC_TypeDef;

// --- Static fake peripherals ---
static RCC_TypeDef fakeRcc;
static GPIO_TypeDef fakeGpioA;
static GPIO_TypeDef fakeGpioB;
static GPIO_TypeDef fakeGpioC;

// --- Override hardware macros ---
#define RCC   (&fakeRcc)
#define GPIOA (&fakeGpioA)
#define GPIOB (&fakeGpioB)
#define GPIOC (&fakeGpioC)

// RCC bit definitions
#if defined(STM32G474xx)
#define RCC_AHB2ENR_GPIOAEN (1U << 0)
#define RCC_AHB2ENR_GPIOBEN (1U << 1)
#define RCC_AHB2ENR_GPIOCEN (1U << 2)
#elif defined(STM32F413xx)
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
#define RCC_AHB1ENR_GPIOCEN (1U << 2)
#endif

// Include production code (header + implementation)
#include <io/Gpio.h>
#include <io/Gpio.cpp>

#include <gtest/gtest.h>

using namespace bios;

class GpioStressTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        std::memset(&fakeRcc, 0, sizeof(fakeRcc));
        std::memset(&fakeGpioA, 0, sizeof(fakeGpioA));
        std::memset(&fakeGpioB, 0, sizeof(fakeGpioB));
        std::memset(&fakeGpioC, 0, sizeof(fakeGpioC));
    }
};

// =============================================================================
// Rapid mode switching: INPUT -> OUTPUT -> ALTERNATE -> ANALOG on same pin
// 16 pins x 1 test each = 16 tests
// =============================================================================

TEST_F(GpioStressTest, rapidModeSwitch_Pin0)
{
    Gpio::setMode(GPIOA, 0U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), 0U);
    Gpio::setMode(GPIOA, 0U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (1U << 0));
    Gpio::setMode(GPIOA, 0U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (2U << 0));
    Gpio::setMode(GPIOA, 0U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (3U << 0));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin1)
{
    Gpio::setMode(GPIOA, 1U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), 0U);
    Gpio::setMode(GPIOA, 1U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (1U << 2));
    Gpio::setMode(GPIOA, 1U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (2U << 2));
    Gpio::setMode(GPIOA, 1U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (3U << 2));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin2)
{
    Gpio::setMode(GPIOA, 2U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), 0U);
    Gpio::setMode(GPIOA, 2U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (1U << 4));
    Gpio::setMode(GPIOA, 2U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (2U << 4));
    Gpio::setMode(GPIOA, 2U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (3U << 4));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin3)
{
    Gpio::setMode(GPIOA, 3U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), 0U);
    Gpio::setMode(GPIOA, 3U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (1U << 6));
    Gpio::setMode(GPIOA, 3U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (2U << 6));
    Gpio::setMode(GPIOA, 3U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (3U << 6));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin4)
{
    Gpio::setMode(GPIOA, 4U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), 0U);
    Gpio::setMode(GPIOA, 4U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (1U << 8));
    Gpio::setMode(GPIOA, 4U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (2U << 8));
    Gpio::setMode(GPIOA, 4U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (3U << 8));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin5)
{
    Gpio::setMode(GPIOA, 5U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), 0U);
    Gpio::setMode(GPIOA, 5U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (1U << 10));
    Gpio::setMode(GPIOA, 5U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (2U << 10));
    Gpio::setMode(GPIOA, 5U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (3U << 10));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin6)
{
    Gpio::setMode(GPIOA, 6U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), 0U);
    Gpio::setMode(GPIOA, 6U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (1U << 12));
    Gpio::setMode(GPIOA, 6U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (2U << 12));
    Gpio::setMode(GPIOA, 6U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (3U << 12));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin7)
{
    Gpio::setMode(GPIOA, 7U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), 0U);
    Gpio::setMode(GPIOA, 7U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (1U << 14));
    Gpio::setMode(GPIOA, 7U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (2U << 14));
    Gpio::setMode(GPIOA, 7U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (3U << 14));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin8)
{
    Gpio::setMode(GPIOA, 8U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), 0U);
    Gpio::setMode(GPIOA, 8U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (1U << 16));
    Gpio::setMode(GPIOA, 8U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (2U << 16));
    Gpio::setMode(GPIOA, 8U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (3U << 16));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin9)
{
    Gpio::setMode(GPIOA, 9U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), 0U);
    Gpio::setMode(GPIOA, 9U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (1U << 18));
    Gpio::setMode(GPIOA, 9U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (2U << 18));
    Gpio::setMode(GPIOA, 9U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (3U << 18));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin10)
{
    Gpio::setMode(GPIOA, 10U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), 0U);
    Gpio::setMode(GPIOA, 10U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (1U << 20));
    Gpio::setMode(GPIOA, 10U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (2U << 20));
    Gpio::setMode(GPIOA, 10U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (3U << 20));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin11)
{
    Gpio::setMode(GPIOA, 11U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), 0U);
    Gpio::setMode(GPIOA, 11U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (1U << 22));
    Gpio::setMode(GPIOA, 11U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (2U << 22));
    Gpio::setMode(GPIOA, 11U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (3U << 22));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin12)
{
    Gpio::setMode(GPIOA, 12U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), 0U);
    Gpio::setMode(GPIOA, 12U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (1U << 24));
    Gpio::setMode(GPIOA, 12U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (2U << 24));
    Gpio::setMode(GPIOA, 12U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (3U << 24));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin13)
{
    Gpio::setMode(GPIOA, 13U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), 0U);
    Gpio::setMode(GPIOA, 13U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (1U << 26));
    Gpio::setMode(GPIOA, 13U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (2U << 26));
    Gpio::setMode(GPIOA, 13U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (3U << 26));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin14)
{
    Gpio::setMode(GPIOA, 14U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), 0U);
    Gpio::setMode(GPIOA, 14U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (1U << 28));
    Gpio::setMode(GPIOA, 14U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (2U << 28));
    Gpio::setMode(GPIOA, 14U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (3U << 28));
}

TEST_F(GpioStressTest, rapidModeSwitch_Pin15)
{
    Gpio::setMode(GPIOA, 15U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), 0U);
    Gpio::setMode(GPIOA, 15U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (1U << 30));
    Gpio::setMode(GPIOA, 15U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (2U << 30));
    Gpio::setMode(GPIOA, 15U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (3U << 30));
}

// =============================================================================
// Register isolation: changing pin N mode doesn't affect pin N-1 or N+1
// 15 tests (pins 1-15, check N-1)
// =============================================================================

TEST_F(GpioStressTest, modeIsolation_Pin1_DoesNotAffectPin0)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 1U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (3U << 0));
}

TEST_F(GpioStressTest, modeIsolation_Pin2_DoesNotAffectPin1Or3)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 2U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (3U << 2));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (3U << 6));
}

TEST_F(GpioStressTest, modeIsolation_Pin3_DoesNotAffectPin2Or4)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 3U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (3U << 4));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (3U << 8));
}

TEST_F(GpioStressTest, modeIsolation_Pin4_DoesNotAffectPin3Or5)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 4U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (3U << 6));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (3U << 10));
}

TEST_F(GpioStressTest, modeIsolation_Pin5_DoesNotAffectPin4Or6)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 5U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (3U << 8));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (3U << 12));
}

TEST_F(GpioStressTest, modeIsolation_Pin6_DoesNotAffectPin5Or7)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 6U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (3U << 10));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (3U << 14));
}

TEST_F(GpioStressTest, modeIsolation_Pin7_DoesNotAffectPin6Or8)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 7U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (3U << 12));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (3U << 16));
}

TEST_F(GpioStressTest, modeIsolation_Pin8_DoesNotAffectPin7Or9)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 8U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (3U << 14));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (3U << 18));
}

TEST_F(GpioStressTest, modeIsolation_Pin9_DoesNotAffectPin8Or10)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 9U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (3U << 16));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (3U << 20));
}

TEST_F(GpioStressTest, modeIsolation_Pin10_DoesNotAffectPin9Or11)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 10U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (3U << 18));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (3U << 22));
}

TEST_F(GpioStressTest, modeIsolation_Pin11_DoesNotAffectPin10Or12)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 11U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (3U << 20));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (3U << 24));
}

TEST_F(GpioStressTest, modeIsolation_Pin12_DoesNotAffectPin11Or13)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 12U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (3U << 22));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (3U << 26));
}

TEST_F(GpioStressTest, modeIsolation_Pin13_DoesNotAffectPin12Or14)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 13U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (3U << 24));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (3U << 28));
}

TEST_F(GpioStressTest, modeIsolation_Pin14_DoesNotAffectPin13Or15)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 14U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (3U << 26));
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (3U << 30));
}

TEST_F(GpioStressTest, modeIsolation_Pin15_DoesNotAffectPin14)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 15U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), 0U);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (3U << 28));
}

// =============================================================================
// Full port scan: all 16 pins as OUTPUT => MODER = 0x55555555
// =============================================================================

TEST_F(GpioStressTest, fullPortScan_AllOutput)
{
    for (uint8_t pin = 0U; pin < 16U; pin++)
    {
        Gpio::setMode(GPIOA, pin, GpioMode::OUTPUT);
    }
    EXPECT_EQ(fakeGpioA.MODER, 0x55555555U);
}

// =============================================================================
// Full port scan: all 16 pins as INPUT => MODER = 0x00000000
// =============================================================================

TEST_F(GpioStressTest, fullPortScan_AllInput)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    for (uint8_t pin = 0U; pin < 16U; pin++)
    {
        Gpio::setMode(GPIOA, pin, GpioMode::INPUT);
    }
    EXPECT_EQ(fakeGpioA.MODER, 0x00000000U);
}

// =============================================================================
// AFR register isolation: setting AF on pin 7 doesn't affect pin 6 or pin 8
// 5 tests
// =============================================================================

TEST_F(GpioStressTest, afrIsolation_Pin7_DoesNotAffectPin6)
{
    Gpio::setAlternateFunction(GPIOA, 6U, 5U);
    uint32_t afr0Before = fakeGpioA.AFR[0] & (0xFU << 24);
    Gpio::setAlternateFunction(GPIOA, 7U, 9U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 28), (9U << 28));
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 24), afr0Before);
}

TEST_F(GpioStressTest, afrIsolation_Pin7_DoesNotAffectPin8)
{
    Gpio::setAlternateFunction(GPIOA, 8U, 3U);
    uint32_t afr1Pin8 = fakeGpioA.AFR[1] & (0xFU << 0);
    Gpio::setAlternateFunction(GPIOA, 7U, 11U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 28), (11U << 28));
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 0), afr1Pin8);
}

TEST_F(GpioStressTest, afrIsolation_Pin3_DoesNotAffectPin2Or4)
{
    Gpio::setAlternateFunction(GPIOA, 2U, 7U);
    Gpio::setAlternateFunction(GPIOA, 4U, 10U);
    uint32_t pin2Val = fakeGpioA.AFR[0] & (0xFU << 8);
    uint32_t pin4Val = fakeGpioA.AFR[0] & (0xFU << 16);
    Gpio::setAlternateFunction(GPIOA, 3U, 13U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 12), (13U << 12));
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 8), pin2Val);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 16), pin4Val);
}

TEST_F(GpioStressTest, afrIsolation_Pin9_DoesNotAffectPin8Or10)
{
    Gpio::setAlternateFunction(GPIOA, 8U, 2U);
    Gpio::setAlternateFunction(GPIOA, 10U, 6U);
    uint32_t pin8Val = fakeGpioA.AFR[1] & (0xFU << 0);
    uint32_t pin10Val = fakeGpioA.AFR[1] & (0xFU << 8);
    Gpio::setAlternateFunction(GPIOA, 9U, 14U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 4), (14U << 4));
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 0), pin8Val);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 8), pin10Val);
}

TEST_F(GpioStressTest, afrIsolation_Pin15_DoesNotAffectPin14)
{
    Gpio::setAlternateFunction(GPIOA, 14U, 4U);
    uint32_t pin14Val = fakeGpioA.AFR[1] & (0xFU << 24);
    Gpio::setAlternateFunction(GPIOA, 15U, 8U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 28), (8U << 28));
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 24), pin14Val);
}

// =============================================================================
// BSRR atomic write: write high on pin N, verify only bit N set (16 tests)
// =============================================================================

TEST_F(GpioStressTest, bsrrHigh_Pin0)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 0U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 0));
}

TEST_F(GpioStressTest, bsrrHigh_Pin1)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 1U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 1));
}

TEST_F(GpioStressTest, bsrrHigh_Pin2)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 2U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 2));
}

TEST_F(GpioStressTest, bsrrHigh_Pin3)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 3U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 3));
}

TEST_F(GpioStressTest, bsrrHigh_Pin4)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 4U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 4));
}

TEST_F(GpioStressTest, bsrrHigh_Pin5)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 5U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 5));
}

TEST_F(GpioStressTest, bsrrHigh_Pin6)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 6U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 6));
}

TEST_F(GpioStressTest, bsrrHigh_Pin7)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 7U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 7));
}

TEST_F(GpioStressTest, bsrrHigh_Pin8)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 8U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 8));
}

TEST_F(GpioStressTest, bsrrHigh_Pin9)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 9U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 9));
}

TEST_F(GpioStressTest, bsrrHigh_Pin10)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 10U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 10));
}

TEST_F(GpioStressTest, bsrrHigh_Pin11)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 11U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 11));
}

TEST_F(GpioStressTest, bsrrHigh_Pin12)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 12U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 12));
}

TEST_F(GpioStressTest, bsrrHigh_Pin13)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 13U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 13));
}

TEST_F(GpioStressTest, bsrrHigh_Pin14)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 14U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 14));
}

TEST_F(GpioStressTest, bsrrHigh_Pin15)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 15U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 15));
}

// =============================================================================
// BSRR atomic write: write low on pin N, verify only bit N+16 set (16 tests)
// =============================================================================

TEST_F(GpioStressTest, bsrrLow_Pin0)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 0U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 16));
}

TEST_F(GpioStressTest, bsrrLow_Pin1)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 1U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 17));
}

TEST_F(GpioStressTest, bsrrLow_Pin2)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 2U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 18));
}

TEST_F(GpioStressTest, bsrrLow_Pin3)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 3U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 19));
}

TEST_F(GpioStressTest, bsrrLow_Pin4)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 4U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 20));
}

TEST_F(GpioStressTest, bsrrLow_Pin5)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 5U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 21));
}

TEST_F(GpioStressTest, bsrrLow_Pin6)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 6U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 22));
}

TEST_F(GpioStressTest, bsrrLow_Pin7)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 7U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 23));
}

TEST_F(GpioStressTest, bsrrLow_Pin8)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 8U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 24));
}

TEST_F(GpioStressTest, bsrrLow_Pin9)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 9U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 25));
}

TEST_F(GpioStressTest, bsrrLow_Pin10)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 10U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 26));
}

TEST_F(GpioStressTest, bsrrLow_Pin11)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 11U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 27));
}

TEST_F(GpioStressTest, bsrrLow_Pin12)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 12U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 28));
}

TEST_F(GpioStressTest, bsrrLow_Pin13)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 13U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 29));
}

TEST_F(GpioStressTest, bsrrLow_Pin14)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 14U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 30));
}

TEST_F(GpioStressTest, bsrrLow_Pin15)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 15U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 31));
}

// =============================================================================
// ODR toggle sequence: toggle same pin 10 times, verify final state (16 tests)
// Even toggles => back to original; 10 is even => ODR bit should be 0
// =============================================================================

TEST_F(GpioStressTest, odrToggle10_Pin0)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 0U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 0), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin1)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 1U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 1), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin2)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 2U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 2), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin3)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 3U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 3), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin4)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 4U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 4), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin5)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 5U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 5), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin6)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 6U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 6), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin7)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 7U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 7), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin8)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 8U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 8), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin9)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 9U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 9), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin10)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 10U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 10), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin11)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 11U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 11), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin12)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 12U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 12), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin13)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 13U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 13), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin14)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 14U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 14), 0U);
}

TEST_F(GpioStressTest, odrToggle10_Pin15)
{
    fakeGpioA.ODR = 0U;
    for (int i = 0; i < 10; i++) { Gpio::togglePin(GPIOA, 15U); }
    EXPECT_EQ(fakeGpioA.ODR & (1U << 15), 0U);
}

// =============================================================================
// Pull configuration isolation: set PULLUP on pin N, verify N-1/N+1 unaffected
// 8 tests
// =============================================================================

TEST_F(GpioStressTest, pullIsolation_Pin5_DoesNotAffectPin4)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 4U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 5U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), (1U << 10));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 8), (2U << 8));
}

TEST_F(GpioStressTest, pullIsolation_Pin5_DoesNotAffectPin6)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 6U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 5U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), (1U << 10));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 12), (2U << 12));
}

TEST_F(GpioStressTest, pullIsolation_Pin0_DoesNotAffectPin1)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 1U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 0U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 0), (1U << 0));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 2), (2U << 2));
}

TEST_F(GpioStressTest, pullIsolation_Pin15_DoesNotAffectPin14)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 14U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 15U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), (1U << 30));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 28), (2U << 28));
}

TEST_F(GpioStressTest, pullIsolation_Pin8_DoesNotAffectPin7Or9)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 7U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 9U, GpioPull::DOWN);
    Gpio::setPull(GPIOA, 8U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 16), (1U << 16));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), (2U << 14));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 18), (2U << 18));
}

TEST_F(GpioStressTest, pullIsolation_Pin10_NoneDoesNotAffectPin9Or11)
{
    fakeGpioA.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOA, 10U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 20), 0U);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 18), (3U << 18));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 22), (3U << 22));
}

TEST_F(GpioStressTest, pullIsolation_Pin3_UpDoesNotAffectPin2Or4)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 3U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 6), (1U << 6));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 4), 0U);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 8), 0U);
}

TEST_F(GpioStressTest, pullIsolation_Pin12_DownDoesNotAffectPin11Or13)
{
    fakeGpioA.PUPDR = 0U;
    Gpio::setPull(GPIOA, 12U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 24), (2U << 24));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 22), 0U);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 26), 0U);
}

// =============================================================================
// Speed configuration isolation (8 tests)
// =============================================================================

TEST_F(GpioStressTest, speedIsolation_Pin5_DoesNotAffectPin4Or6)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), (3U << 10));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 8), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 12), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin0_DoesNotAffectPin1)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 0), (2U << 0));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 2), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin15_DoesNotAffectPin14)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), (3U << 30));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 28), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin8_DoesNotAffectPin7Or9)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 8U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 16), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 14), (3U << 14));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 18), (3U << 18));
}

TEST_F(GpioStressTest, speedIsolation_Pin3_MedDoesNotAffectPin2Or4)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 3U, GpioSpeed::MEDIUM);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 6), (1U << 6));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 4), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 8), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin10_HighDoesNotAffectPin9Or11)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 10U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 20), (2U << 20));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 18), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 22), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin12_VeryHighDoesNotAffectPin11Or13)
{
    fakeGpioA.OSPEEDR = 0U;
    Gpio::setSpeed(GPIOA, 12U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 24), (3U << 24));
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 22), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 26), 0U);
}

TEST_F(GpioStressTest, speedIsolation_Pin7_LowClearsDoesNotAffectPin6)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 7U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 14), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 12), (3U << 12));
}

// =============================================================================
// Output type isolation (8 tests)
// =============================================================================

TEST_F(GpioStressTest, otypeIsolation_Pin5_PushPullDoesNotAffectPin4Or6)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 5U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 5), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 4), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 6), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin0_OpenDrainDoesNotAffectPin1)
{
    fakeGpioA.OTYPER = 0U;
    Gpio::setOutputType(GPIOA, 0U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 0), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 1), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin15_OpenDrainDoesNotAffectPin14)
{
    fakeGpioA.OTYPER = 0U;
    Gpio::setOutputType(GPIOA, 15U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 15), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 14), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin8_PushPullDoesNotAffectPin7Or9)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 8U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 8), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 7), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 9), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin3_OpenDrainDoesNotAffectPin2Or4)
{
    fakeGpioA.OTYPER = 0U;
    Gpio::setOutputType(GPIOA, 3U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 3), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 2), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 4), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin10_PushPullClearsDoesNotAffectPin9Or11)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 10U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 10), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 9), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 11), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin12_OpenDrainDoesNotAffectPin11Or13)
{
    fakeGpioA.OTYPER = 0U;
    Gpio::setOutputType(GPIOA, 12U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 12), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 11), 0U);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 13), 0U);
}

TEST_F(GpioStressTest, otypeIsolation_Pin7_PushPullClearsDoesNotAffectPin6)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 7U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 7), 0U);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 6), 0U);
}

// =============================================================================
// Configure then reconfigure with different settings (10 tests)
// =============================================================================

TEST_F(GpioStressTest, reconfig_OutputToInput)
{
    Gpio::setMode(GPIOA, 5U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (1U << 10));
    Gpio::setMode(GPIOA, 5U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), 0U);
}

TEST_F(GpioStressTest, reconfig_InputToAlternate)
{
    Gpio::setMode(GPIOB, 3U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 6), 0U);
    Gpio::setMode(GPIOB, 3U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 6), (2U << 6));
}

TEST_F(GpioStressTest, reconfig_AnalogToOutput)
{
    Gpio::setMode(GPIOC, 13U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioC.MODER & (3U << 26), (3U << 26));
    Gpio::setMode(GPIOC, 13U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioC.MODER & (3U << 26), (1U << 26));
}

TEST_F(GpioStressTest, reconfig_SpeedLowToVeryHigh)
{
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), 0U);
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), (3U << 10));
}

TEST_F(GpioStressTest, reconfig_PullUpToDown)
{
    Gpio::setPull(GPIOA, 5U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), (1U << 10));
    Gpio::setPull(GPIOA, 5U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), (2U << 10));
}

TEST_F(GpioStressTest, reconfig_PullDownToNone)
{
    Gpio::setPull(GPIOA, 7U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), (2U << 14));
    Gpio::setPull(GPIOA, 7U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), 0U);
}

TEST_F(GpioStressTest, reconfig_OpenDrainToPushPull)
{
    Gpio::setOutputType(GPIOA, 5U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 5), 0U);
    Gpio::setOutputType(GPIOA, 5U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 5), 0U);
}

TEST_F(GpioStressTest, reconfig_AF5ToAF7)
{
    Gpio::setAlternateFunction(GPIOA, 5U, 5U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 20), (5U << 20));
    Gpio::setAlternateFunction(GPIOA, 5U, 7U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 20), (7U << 20));
}

TEST_F(GpioStressTest, reconfig_FullConfigure_OutputThenAlternate)
{
    GpioConfig const cfg1 = {
        GPIOA, 11U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL,
        GpioSpeed::LOW, GpioPull::NONE, 0U};
    Gpio::configure(cfg1);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (1U << 22));

    GpioConfig const cfg2 = {
        GPIOA, 11U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL,
        GpioSpeed::HIGH, GpioPull::UP, 9U};
    Gpio::configure(cfg2);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (2U << 22));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 22), (1U << 22));
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 12), (9U << 12));
}

TEST_F(GpioStressTest, reconfig_FullConfigure_AlternateThenAnalog)
{
    GpioConfig const cfg1 = {
        GPIOB, 7U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL,
        GpioSpeed::VERY_HIGH, GpioPull::UP, 4U};
    Gpio::configure(cfg1);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 14), (2U << 14));

    GpioConfig const cfg2 = {
        GPIOB, 7U, GpioMode::ANALOG, GpioOutputType::PUSH_PULL,
        GpioSpeed::LOW, GpioPull::NONE, 0U};
    Gpio::configure(cfg2);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 14), (3U << 14));
    EXPECT_EQ(fakeGpioB.PUPDR & (3U << 14), 0U);
}
