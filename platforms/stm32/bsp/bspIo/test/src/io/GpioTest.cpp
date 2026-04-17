// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   GpioTest.cpp
 * \brief  Unit tests for the STM32 GPIO abstraction (Gpio class).
 *
 * Uses fake GPIO_TypeDef and RCC_TypeDef structs allocated in RAM
 * so register writes go to testable memory instead of real hardware.
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

// RCC bit definitions — define both sets so tests can reference either
#define RCC_AHB2ENR_GPIOAEN (1U << 0)
#define RCC_AHB2ENR_GPIOBEN (1U << 1)
#define RCC_AHB2ENR_GPIOCEN (1U << 2)
#define RCC_AHB1ENR_GPIOAEN (1U << 0)
#define RCC_AHB1ENR_GPIOBEN (1U << 1)
#define RCC_AHB1ENR_GPIOCEN (1U << 2)

// Select G4 code path for enablePortClock tests
#define STM32G474xx

// Include production code (header + implementation)
#include <io/Gpio.h>
#include <io/Gpio.cpp>

#include <gtest/gtest.h>

using namespace bios;

class GpioTest : public ::testing::Test
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

// --- Mode configuration tests ---

TEST_F(GpioTest, setMode_Input)
{
    fakeGpioA.MODER = 0xFFFFFFFFU; // all analog (default on STM32)
    Gpio::setMode(GPIOA, 5U, GpioMode::INPUT);
    // Pin 5: bits [11:10] should be 00
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), 0U);
}

TEST_F(GpioTest, setMode_Output)
{
    Gpio::setMode(GPIOA, 5U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (1U << 10));
}

TEST_F(GpioTest, setMode_Alternate)
{
    Gpio::setMode(GPIOB, 7U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 14), (2U << 14));
}

TEST_F(GpioTest, setMode_Analog)
{
    Gpio::setMode(GPIOC, 13U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioC.MODER & (3U << 26), (3U << 26));
}

// --- Output type tests ---

TEST_F(GpioTest, setOutputType_PushPull)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 5U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 5), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain)
{
    Gpio::setOutputType(GPIOA, 5U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 5), 0U);
}

// --- Speed tests ---

TEST_F(GpioTest, setSpeed_VeryHigh)
{
    Gpio::setSpeed(GPIOA, 12U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 24), (3U << 24));
}

// --- Pull-up/pull-down tests ---

TEST_F(GpioTest, setPull_Up)
{
    Gpio::setPull(GPIOA, 11U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 22), (1U << 22));
}

TEST_F(GpioTest, setPull_Down)
{
    Gpio::setPull(GPIOB, 0U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioB.PUPDR & 3U, 2U);
}

TEST_F(GpioTest, setPull_None)
{
    fakeGpioA.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOA, 5U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), 0U);
}

// --- Alternate function tests ---

TEST_F(GpioTest, setAlternateFunction_LowPin)
{
    Gpio::setAlternateFunction(GPIOA, 5U, 9U);
    // Pin 5 is in AFR[0], bits [23:20]
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 20), (9U << 20));
}

TEST_F(GpioTest, setAlternateFunction_HighPin)
{
    Gpio::setAlternateFunction(GPIOA, 11U, 9U);
    // Pin 11 is in AFR[1], bits [15:12] (pin-8=3, 3*4=12)
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 12), (9U << 12));
}

// --- Read/write/toggle tests ---

TEST_F(GpioTest, readPin_High)
{
    fakeGpioC.IDR = (1U << 13);
    EXPECT_TRUE(Gpio::readPin(GPIOC, 13U));
}

TEST_F(GpioTest, readPin_Low)
{
    fakeGpioC.IDR = 0U;
    EXPECT_FALSE(Gpio::readPin(GPIOC, 13U));
}

TEST_F(GpioTest, writePin_High)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 5U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 5));
}

TEST_F(GpioTest, writePin_Low)
{
    fakeGpioA.BSRR = 0U;
    Gpio::writePin(GPIOA, 5U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 21)); // pin + 16
}

TEST_F(GpioTest, togglePin)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 5U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 5), 0U);
    Gpio::togglePin(GPIOA, 5U);
    EXPECT_EQ(fakeGpioA.ODR & (1U << 5), 0U);
}

// --- Full configure test ---

TEST_F(GpioTest, configure_OutputPin)
{
    GpioConfig const cfg = {
        GPIOA,
        5U,
        GpioMode::OUTPUT,
        GpioOutputType::PUSH_PULL,
        GpioSpeed::LOW,
        GpioPull::NONE,
        0U};
    Gpio::configure(cfg);

    // Mode = 01 (output)
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (1U << 10));
    // OTYPER bit 5 = 0 (push-pull)
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 5), 0U);
    // Speed = 00 (low)
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), 0U);
    // Pull = 00 (none)
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 10), 0U);
}

TEST_F(GpioTest, configure_AlternateFunctionPin)
{
    GpioConfig const cfg = {
        GPIOA,
        11U,
        GpioMode::ALTERNATE,
        GpioOutputType::PUSH_PULL,
        GpioSpeed::HIGH,
        GpioPull::UP,
        9U};
    Gpio::configure(cfg);

    // Mode = 10 (AF)
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (2U << 22));
    // Pull = 01 (up)
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 22), (1U << 22));
    // AF9 on pin 11 → AFR[1] bits [15:12]
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 12), (9U << 12));
}

// =============================================================================
// Additional tests: setMode for all 16 pins in each mode (64 tests)
// =============================================================================

TEST_F(GpioTest, setMode_Input_Pin0)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 0U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin1)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 1U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin2)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 2U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin3)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 3U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin4)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 4U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin5)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 5U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin6)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 6U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin7)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 7U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin8)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 8U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin9)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 9U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin10)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 10U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin11)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 11U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin12)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 12U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin13)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 13U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin14)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 14U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), 0U);
}

TEST_F(GpioTest, setMode_Input_Pin15)
{
    fakeGpioA.MODER = 0xFFFFFFFFU;
    Gpio::setMode(GPIOA, 15U, GpioMode::INPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), 0U);
}

TEST_F(GpioTest, setMode_Output_Pin0)
{
    Gpio::setMode(GPIOA, 0U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (1U << 0));
}

TEST_F(GpioTest, setMode_Output_Pin1)
{
    Gpio::setMode(GPIOA, 1U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (1U << 2));
}

TEST_F(GpioTest, setMode_Output_Pin2)
{
    Gpio::setMode(GPIOA, 2U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (1U << 4));
}

TEST_F(GpioTest, setMode_Output_Pin3)
{
    Gpio::setMode(GPIOA, 3U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (1U << 6));
}

TEST_F(GpioTest, setMode_Output_Pin4)
{
    Gpio::setMode(GPIOA, 4U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (1U << 8));
}

TEST_F(GpioTest, setMode_Output_Pin5_Explicit)
{
    Gpio::setMode(GPIOB, 5U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 10), (1U << 10));
}

TEST_F(GpioTest, setMode_Output_Pin6)
{
    Gpio::setMode(GPIOA, 6U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (1U << 12));
}

TEST_F(GpioTest, setMode_Output_Pin7)
{
    Gpio::setMode(GPIOA, 7U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (1U << 14));
}

TEST_F(GpioTest, setMode_Output_Pin8)
{
    Gpio::setMode(GPIOA, 8U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (1U << 16));
}

TEST_F(GpioTest, setMode_Output_Pin9)
{
    Gpio::setMode(GPIOA, 9U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (1U << 18));
}

TEST_F(GpioTest, setMode_Output_Pin10)
{
    Gpio::setMode(GPIOA, 10U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (1U << 20));
}

TEST_F(GpioTest, setMode_Output_Pin11)
{
    Gpio::setMode(GPIOA, 11U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (1U << 22));
}

TEST_F(GpioTest, setMode_Output_Pin12)
{
    Gpio::setMode(GPIOA, 12U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (1U << 24));
}

TEST_F(GpioTest, setMode_Output_Pin13)
{
    Gpio::setMode(GPIOA, 13U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (1U << 26));
}

TEST_F(GpioTest, setMode_Output_Pin14)
{
    Gpio::setMode(GPIOA, 14U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (1U << 28));
}

TEST_F(GpioTest, setMode_Output_Pin15)
{
    Gpio::setMode(GPIOA, 15U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (1U << 30));
}

TEST_F(GpioTest, setMode_Alternate_Pin0)
{
    Gpio::setMode(GPIOA, 0U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (2U << 0));
}

TEST_F(GpioTest, setMode_Alternate_Pin1)
{
    Gpio::setMode(GPIOA, 1U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (2U << 2));
}

TEST_F(GpioTest, setMode_Alternate_Pin2)
{
    Gpio::setMode(GPIOA, 2U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (2U << 4));
}

TEST_F(GpioTest, setMode_Alternate_Pin3)
{
    Gpio::setMode(GPIOA, 3U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (2U << 6));
}

TEST_F(GpioTest, setMode_Alternate_Pin4)
{
    Gpio::setMode(GPIOA, 4U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (2U << 8));
}

TEST_F(GpioTest, setMode_Alternate_Pin5)
{
    Gpio::setMode(GPIOB, 5U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 10), (2U << 10));
}

TEST_F(GpioTest, setMode_Alternate_Pin6)
{
    Gpio::setMode(GPIOA, 6U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (2U << 12));
}

TEST_F(GpioTest, setMode_Alternate_Pin8)
{
    Gpio::setMode(GPIOA, 8U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (2U << 16));
}

TEST_F(GpioTest, setMode_Alternate_Pin9)
{
    Gpio::setMode(GPIOA, 9U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (2U << 18));
}

TEST_F(GpioTest, setMode_Alternate_Pin10)
{
    Gpio::setMode(GPIOA, 10U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (2U << 20));
}

TEST_F(GpioTest, setMode_Alternate_Pin11_Explicit)
{
    Gpio::setMode(GPIOC, 11U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioC.MODER & (3U << 22), (2U << 22));
}

TEST_F(GpioTest, setMode_Alternate_Pin12)
{
    Gpio::setMode(GPIOA, 12U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (2U << 24));
}

TEST_F(GpioTest, setMode_Alternate_Pin13)
{
    Gpio::setMode(GPIOA, 13U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 26), (2U << 26));
}

TEST_F(GpioTest, setMode_Alternate_Pin14)
{
    Gpio::setMode(GPIOA, 14U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (2U << 28));
}

TEST_F(GpioTest, setMode_Alternate_Pin15)
{
    Gpio::setMode(GPIOA, 15U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (2U << 30));
}

TEST_F(GpioTest, setMode_Analog_Pin0)
{
    Gpio::setMode(GPIOA, 0U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (3U << 0));
}

TEST_F(GpioTest, setMode_Analog_Pin1)
{
    Gpio::setMode(GPIOA, 1U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (3U << 2));
}

TEST_F(GpioTest, setMode_Analog_Pin2)
{
    Gpio::setMode(GPIOA, 2U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 4), (3U << 4));
}

TEST_F(GpioTest, setMode_Analog_Pin3)
{
    Gpio::setMode(GPIOA, 3U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 6), (3U << 6));
}

TEST_F(GpioTest, setMode_Analog_Pin4)
{
    Gpio::setMode(GPIOA, 4U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 8), (3U << 8));
}

TEST_F(GpioTest, setMode_Analog_Pin5)
{
    Gpio::setMode(GPIOA, 5U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (3U << 10));
}

TEST_F(GpioTest, setMode_Analog_Pin6)
{
    Gpio::setMode(GPIOA, 6U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 12), (3U << 12));
}

TEST_F(GpioTest, setMode_Analog_Pin7)
{
    Gpio::setMode(GPIOA, 7U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 14), (3U << 14));
}

TEST_F(GpioTest, setMode_Analog_Pin8)
{
    Gpio::setMode(GPIOA, 8U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (3U << 16));
}

TEST_F(GpioTest, setMode_Analog_Pin9)
{
    Gpio::setMode(GPIOA, 9U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 18), (3U << 18));
}

TEST_F(GpioTest, setMode_Analog_Pin10)
{
    Gpio::setMode(GPIOA, 10U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 20), (3U << 20));
}

TEST_F(GpioTest, setMode_Analog_Pin11)
{
    Gpio::setMode(GPIOA, 11U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 22), (3U << 22));
}

TEST_F(GpioTest, setMode_Analog_Pin12)
{
    Gpio::setMode(GPIOA, 12U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 24), (3U << 24));
}

TEST_F(GpioTest, setMode_Analog_Pin14)
{
    Gpio::setMode(GPIOA, 14U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 28), (3U << 28));
}

TEST_F(GpioTest, setMode_Analog_Pin15)
{
    Gpio::setMode(GPIOA, 15U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (3U << 30));
}

// =============================================================================
// setOutputType for all 16 pins (32 tests: push-pull + open-drain per pin)
// =============================================================================

TEST_F(GpioTest, setOutputType_PushPull_Pin0)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 0U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 0), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin0)
{
    Gpio::setOutputType(GPIOA, 0U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 0), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin1)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 1U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 1), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin1)
{
    Gpio::setOutputType(GPIOA, 1U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 1), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin2)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 2U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 2), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin2)
{
    Gpio::setOutputType(GPIOA, 2U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 2), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin3)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 3U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 3), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin3)
{
    Gpio::setOutputType(GPIOA, 3U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 3), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin4)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 4U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 4), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin4)
{
    Gpio::setOutputType(GPIOA, 4U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 4), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin6)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 6U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 6), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin6)
{
    Gpio::setOutputType(GPIOA, 6U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 6), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin7)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 7U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 7), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin7)
{
    Gpio::setOutputType(GPIOA, 7U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 7), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin8)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 8U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 8), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin8)
{
    Gpio::setOutputType(GPIOA, 8U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 8), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin9)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 9U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 9), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin9)
{
    Gpio::setOutputType(GPIOA, 9U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 9), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin10)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 10U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 10), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin10)
{
    Gpio::setOutputType(GPIOA, 10U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 10), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin11)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 11U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 11), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin11)
{
    Gpio::setOutputType(GPIOA, 11U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 11), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin12)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 12U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 12), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin12)
{
    Gpio::setOutputType(GPIOA, 12U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 12), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin13)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 13U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 13), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin13)
{
    Gpio::setOutputType(GPIOA, 13U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 13), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin14)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 14U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 14), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin14)
{
    Gpio::setOutputType(GPIOA, 14U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 14), 0U);
}

TEST_F(GpioTest, setOutputType_PushPull_Pin15)
{
    fakeGpioA.OTYPER = 0xFFFFU;
    Gpio::setOutputType(GPIOA, 15U, GpioOutputType::PUSH_PULL);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 15), 0U);
}

TEST_F(GpioTest, setOutputType_OpenDrain_Pin15)
{
    Gpio::setOutputType(GPIOA, 15U, GpioOutputType::OPEN_DRAIN);
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 15), 0U);
}

// =============================================================================
// setSpeed for all 4 speeds on multiple pins (16 tests)
// =============================================================================

TEST_F(GpioTest, setSpeed_Low_Pin0)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 0), 0U);
}

TEST_F(GpioTest, setSpeed_Medium_Pin0)
{
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::MEDIUM);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 0), (1U << 0));
}

TEST_F(GpioTest, setSpeed_High_Pin0)
{
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 0), (2U << 0));
}

TEST_F(GpioTest, setSpeed_VeryHigh_Pin0)
{
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 0), (3U << 0));
}

TEST_F(GpioTest, setSpeed_Low_Pin5)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), 0U);
}

TEST_F(GpioTest, setSpeed_Medium_Pin5)
{
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::MEDIUM);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), (1U << 10));
}

TEST_F(GpioTest, setSpeed_High_Pin5)
{
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), (2U << 10));
}

TEST_F(GpioTest, setSpeed_VeryHigh_Pin5)
{
    Gpio::setSpeed(GPIOA, 5U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 10), (3U << 10));
}

TEST_F(GpioTest, setSpeed_Low_Pin10)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 10U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 20), 0U);
}

TEST_F(GpioTest, setSpeed_Medium_Pin10)
{
    Gpio::setSpeed(GPIOA, 10U, GpioSpeed::MEDIUM);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 20), (1U << 20));
}

TEST_F(GpioTest, setSpeed_High_Pin10)
{
    Gpio::setSpeed(GPIOA, 10U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 20), (2U << 20));
}

TEST_F(GpioTest, setSpeed_VeryHigh_Pin10)
{
    Gpio::setSpeed(GPIOA, 10U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 20), (3U << 20));
}

TEST_F(GpioTest, setSpeed_Low_Pin15)
{
    fakeGpioA.OSPEEDR = 0xFFFFFFFFU;
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::LOW);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), 0U);
}

TEST_F(GpioTest, setSpeed_Medium_Pin15)
{
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::MEDIUM);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), (1U << 30));
}

TEST_F(GpioTest, setSpeed_High_Pin15)
{
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), (2U << 30));
}

TEST_F(GpioTest, setSpeed_VeryHigh_Pin15)
{
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), (3U << 30));
}

// =============================================================================
// setPull for all 3 configs on multiple pins (12 tests)
// =============================================================================

TEST_F(GpioTest, setPull_None_Pin0)
{
    fakeGpioA.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOA, 0U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 0), 0U);
}

TEST_F(GpioTest, setPull_Up_Pin0)
{
    Gpio::setPull(GPIOA, 0U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 0), (1U << 0));
}

TEST_F(GpioTest, setPull_Down_Pin0)
{
    Gpio::setPull(GPIOA, 0U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 0), (2U << 0));
}

TEST_F(GpioTest, setPull_None_Pin7)
{
    fakeGpioA.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOA, 7U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), 0U);
}

TEST_F(GpioTest, setPull_Up_Pin7)
{
    Gpio::setPull(GPIOA, 7U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), (1U << 14));
}

TEST_F(GpioTest, setPull_Down_Pin7)
{
    Gpio::setPull(GPIOA, 7U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 14), (2U << 14));
}

TEST_F(GpioTest, setPull_None_Pin12)
{
    fakeGpioB.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOB, 12U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioB.PUPDR & (3U << 24), 0U);
}

TEST_F(GpioTest, setPull_Up_Pin12)
{
    Gpio::setPull(GPIOB, 12U, GpioPull::UP);
    EXPECT_EQ(fakeGpioB.PUPDR & (3U << 24), (1U << 24));
}

TEST_F(GpioTest, setPull_Down_Pin12)
{
    Gpio::setPull(GPIOB, 12U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioB.PUPDR & (3U << 24), (2U << 24));
}

TEST_F(GpioTest, setPull_None_Pin15)
{
    fakeGpioA.PUPDR = 0xFFFFFFFFU;
    Gpio::setPull(GPIOA, 15U, GpioPull::NONE);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), 0U);
}

TEST_F(GpioTest, setPull_Up_Pin15)
{
    Gpio::setPull(GPIOA, 15U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), (1U << 30));
}

TEST_F(GpioTest, setPull_Down_Pin15)
{
    Gpio::setPull(GPIOA, 15U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), (2U << 30));
}

// =============================================================================
// setAlternateFunction: AF0-AF15 on low pins (AFR[0]) and high pins (AFR[1])
// =============================================================================

TEST_F(GpioTest, setAlternateFunction_Pin0_AF0)
{
    Gpio::setAlternateFunction(GPIOA, 0U, 0U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 0), (0U << 0));
}

TEST_F(GpioTest, setAlternateFunction_Pin0_AF7)
{
    Gpio::setAlternateFunction(GPIOA, 0U, 7U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 0), (7U << 0));
}

TEST_F(GpioTest, setAlternateFunction_Pin1_AF1)
{
    Gpio::setAlternateFunction(GPIOA, 1U, 1U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 4), (1U << 4));
}

TEST_F(GpioTest, setAlternateFunction_Pin2_AF5)
{
    Gpio::setAlternateFunction(GPIOA, 2U, 5U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 8), (5U << 8));
}

TEST_F(GpioTest, setAlternateFunction_Pin3_AF10)
{
    Gpio::setAlternateFunction(GPIOA, 3U, 10U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 12), (10U << 12));
}

TEST_F(GpioTest, setAlternateFunction_Pin4_AF15)
{
    Gpio::setAlternateFunction(GPIOA, 4U, 15U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 16), (15U << 16));
}

TEST_F(GpioTest, setAlternateFunction_Pin6_AF3)
{
    Gpio::setAlternateFunction(GPIOA, 6U, 3U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 24), (3U << 24));
}

TEST_F(GpioTest, setAlternateFunction_Pin7_AF12)
{
    Gpio::setAlternateFunction(GPIOA, 7U, 12U);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 28), (12U << 28));
}

TEST_F(GpioTest, setAlternateFunction_Pin8_AF0)
{
    Gpio::setAlternateFunction(GPIOA, 8U, 0U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 0), (0U << 0));
}

TEST_F(GpioTest, setAlternateFunction_Pin9_AF8)
{
    Gpio::setAlternateFunction(GPIOA, 9U, 8U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 4), (8U << 4));
}

TEST_F(GpioTest, setAlternateFunction_Pin10_AF2)
{
    Gpio::setAlternateFunction(GPIOA, 10U, 2U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 8), (2U << 8));
}

TEST_F(GpioTest, setAlternateFunction_Pin11_AF14)
{
    Gpio::setAlternateFunction(GPIOA, 11U, 14U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 12), (14U << 12));
}

TEST_F(GpioTest, setAlternateFunction_Pin12_AF6)
{
    Gpio::setAlternateFunction(GPIOA, 12U, 6U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 16), (6U << 16));
}

TEST_F(GpioTest, setAlternateFunction_Pin13_AF11)
{
    Gpio::setAlternateFunction(GPIOA, 13U, 11U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 20), (11U << 20));
}

TEST_F(GpioTest, setAlternateFunction_Pin14_AF4)
{
    Gpio::setAlternateFunction(GPIOA, 14U, 4U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 24), (4U << 24));
}

TEST_F(GpioTest, setAlternateFunction_Pin15_AF13)
{
    Gpio::setAlternateFunction(GPIOA, 15U, 13U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 28), (13U << 28));
}

// =============================================================================
// readPin on all 16 pins
// =============================================================================

TEST_F(GpioTest, readPin_Pin0_High)
{
    fakeGpioA.IDR = (1U << 0);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 0U));
}

TEST_F(GpioTest, readPin_Pin0_Low)
{
    fakeGpioA.IDR = 0U;
    EXPECT_FALSE(Gpio::readPin(GPIOA, 0U));
}

TEST_F(GpioTest, readPin_Pin1_High)
{
    fakeGpioA.IDR = (1U << 1);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 1U));
}

TEST_F(GpioTest, readPin_Pin2_High)
{
    fakeGpioA.IDR = (1U << 2);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 2U));
}

TEST_F(GpioTest, readPin_Pin3_High)
{
    fakeGpioA.IDR = (1U << 3);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 3U));
}

TEST_F(GpioTest, readPin_Pin4_High)
{
    fakeGpioA.IDR = (1U << 4);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 4U));
}

TEST_F(GpioTest, readPin_Pin5_High)
{
    fakeGpioA.IDR = (1U << 5);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 5U));
}

TEST_F(GpioTest, readPin_Pin6_High)
{
    fakeGpioA.IDR = (1U << 6);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 6U));
}

TEST_F(GpioTest, readPin_Pin7_High)
{
    fakeGpioA.IDR = (1U << 7);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 7U));
}

TEST_F(GpioTest, readPin_Pin8_High)
{
    fakeGpioA.IDR = (1U << 8);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 8U));
}

TEST_F(GpioTest, readPin_Pin9_High)
{
    fakeGpioA.IDR = (1U << 9);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 9U));
}

TEST_F(GpioTest, readPin_Pin10_High)
{
    fakeGpioA.IDR = (1U << 10);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 10U));
}

TEST_F(GpioTest, readPin_Pin11_High)
{
    fakeGpioA.IDR = (1U << 11);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 11U));
}

TEST_F(GpioTest, readPin_Pin12_High)
{
    fakeGpioA.IDR = (1U << 12);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 12U));
}

TEST_F(GpioTest, readPin_Pin14_High)
{
    fakeGpioA.IDR = (1U << 14);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 14U));
}

TEST_F(GpioTest, readPin_Pin15_High)
{
    fakeGpioA.IDR = (1U << 15);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 15U));
}

// =============================================================================
// writePin high on all 16 pins
// =============================================================================

TEST_F(GpioTest, writePin_High_Pin0)
{
    Gpio::writePin(GPIOA, 0U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 0));
}

TEST_F(GpioTest, writePin_High_Pin1)
{
    Gpio::writePin(GPIOA, 1U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 1));
}

TEST_F(GpioTest, writePin_High_Pin2)
{
    Gpio::writePin(GPIOA, 2U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 2));
}

TEST_F(GpioTest, writePin_High_Pin3)
{
    Gpio::writePin(GPIOA, 3U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 3));
}

TEST_F(GpioTest, writePin_High_Pin4)
{
    Gpio::writePin(GPIOA, 4U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 4));
}

TEST_F(GpioTest, writePin_High_Pin6)
{
    Gpio::writePin(GPIOA, 6U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 6));
}

TEST_F(GpioTest, writePin_High_Pin7)
{
    Gpio::writePin(GPIOA, 7U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 7));
}

TEST_F(GpioTest, writePin_High_Pin8)
{
    Gpio::writePin(GPIOA, 8U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 8));
}

TEST_F(GpioTest, writePin_High_Pin9)
{
    Gpio::writePin(GPIOA, 9U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 9));
}

TEST_F(GpioTest, writePin_High_Pin10)
{
    Gpio::writePin(GPIOA, 10U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 10));
}

TEST_F(GpioTest, writePin_High_Pin11)
{
    Gpio::writePin(GPIOA, 11U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 11));
}

TEST_F(GpioTest, writePin_High_Pin12)
{
    Gpio::writePin(GPIOA, 12U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 12));
}

TEST_F(GpioTest, writePin_High_Pin13)
{
    Gpio::writePin(GPIOA, 13U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 13));
}

TEST_F(GpioTest, writePin_High_Pin14)
{
    Gpio::writePin(GPIOA, 14U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 14));
}

TEST_F(GpioTest, writePin_High_Pin15)
{
    Gpio::writePin(GPIOA, 15U, true);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 15));
}

// =============================================================================
// togglePin on all 16 pins
// =============================================================================

TEST_F(GpioTest, togglePin_Pin0)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 0U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 0), 0U);
    Gpio::togglePin(GPIOA, 0U);
    EXPECT_EQ(fakeGpioA.ODR & (1U << 0), 0U);
}

TEST_F(GpioTest, togglePin_Pin1)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 1U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 1), 0U);
    Gpio::togglePin(GPIOA, 1U);
    EXPECT_EQ(fakeGpioA.ODR & (1U << 1), 0U);
}

TEST_F(GpioTest, togglePin_Pin2)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 2U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 2), 0U);
}

TEST_F(GpioTest, togglePin_Pin3)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 3U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 3), 0U);
}

TEST_F(GpioTest, togglePin_Pin4)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 4U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 4), 0U);
}

TEST_F(GpioTest, togglePin_Pin6)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 6U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 6), 0U);
}

TEST_F(GpioTest, togglePin_Pin7)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 7U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 7), 0U);
}

TEST_F(GpioTest, togglePin_Pin8)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 8U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 8), 0U);
}

TEST_F(GpioTest, togglePin_Pin9)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 9U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 9), 0U);
}

TEST_F(GpioTest, togglePin_Pin10)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 10U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 10), 0U);
}

TEST_F(GpioTest, togglePin_Pin11)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 11U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 11), 0U);
}

TEST_F(GpioTest, togglePin_Pin12)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 12U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 12), 0U);
}

TEST_F(GpioTest, togglePin_Pin13)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 13U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 13), 0U);
}

TEST_F(GpioTest, togglePin_Pin14)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 14U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 14), 0U);
}

TEST_F(GpioTest, togglePin_Pin15)
{
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 15U);
    EXPECT_NE(fakeGpioA.ODR & (1U << 15), 0U);
}

// =============================================================================
// enablePortClock tests
// NOTE: enablePortClock is a no-op in tests because neither STM32G474xx nor
// STM32F413xx is defined (the fake register stubs don't need chip selection).
// We verify that calling it doesn't crash and that configure() calls it safely.
// =============================================================================

TEST_F(GpioTest, enablePortClock_GPIOA)
{
    Gpio::enablePortClock(GPIOA);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOAEN, 0U);
}

TEST_F(GpioTest, enablePortClock_GPIOB)
{
    Gpio::enablePortClock(GPIOB);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOBEN, 0U);
}

TEST_F(GpioTest, enablePortClock_GPIOC)
{
    Gpio::enablePortClock(GPIOC);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOCEN, 0U);
}

TEST_F(GpioTest, enablePortClock_GPIOA_DoesNotAffectOthers)
{
    Gpio::enablePortClock(GPIOA);
    EXPECT_EQ(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOBEN, 0U);
    EXPECT_EQ(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOCEN, 0U);
}

TEST_F(GpioTest, enablePortClock_AllPorts)
{
    Gpio::enablePortClock(GPIOA);
    Gpio::enablePortClock(GPIOB);
    Gpio::enablePortClock(GPIOC);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOAEN, 0U);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOBEN, 0U);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOCEN, 0U);
}

// =============================================================================
// configure() with various GpioConfig combinations (10 tests)
// =============================================================================

TEST_F(GpioTest, configure_InputPullUp)
{
    GpioConfig const cfg = {GPIOA, 0U, GpioMode::INPUT, GpioOutputType::PUSH_PULL,
                            GpioSpeed::LOW, GpioPull::UP, 0U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), 0U);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 0), (1U << 0));
}

TEST_F(GpioTest, configure_InputPullDown)
{
    GpioConfig const cfg = {GPIOB, 3U, GpioMode::INPUT, GpioOutputType::PUSH_PULL,
                            GpioSpeed::LOW, GpioPull::DOWN, 0U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 6), 0U);
    EXPECT_EQ(fakeGpioB.PUPDR & (3U << 6), (2U << 6));
}

TEST_F(GpioTest, configure_OutputOpenDrainHighSpeed)
{
    GpioConfig const cfg = {GPIOA, 8U, GpioMode::OUTPUT, GpioOutputType::OPEN_DRAIN,
                            GpioSpeed::HIGH, GpioPull::NONE, 0U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 16), (1U << 16));
    EXPECT_NE(fakeGpioA.OTYPER & (1U << 8), 0U);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 16), (2U << 16));
}

TEST_F(GpioTest, configure_AnalogPin)
{
    GpioConfig const cfg = {GPIOA, 1U, GpioMode::ANALOG, GpioOutputType::PUSH_PULL,
                            GpioSpeed::LOW, GpioPull::NONE, 0U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 2), (3U << 2));
}

TEST_F(GpioTest, configure_AF_Pin0_AF1)
{
    GpioConfig const cfg = {GPIOA, 0U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL,
                            GpioSpeed::VERY_HIGH, GpioPull::NONE, 1U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 0), (2U << 0));
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 0), (1U << 0));
}

TEST_F(GpioTest, configure_AF_Pin15_AF9)
{
    GpioConfig const cfg = {GPIOA, 15U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL,
                            GpioSpeed::MEDIUM, GpioPull::UP, 9U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (2U << 30));
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 28), (9U << 28));
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), (1U << 30));
}

TEST_F(GpioTest, configure_OutputPushPullMediumPullDown)
{
    GpioConfig const cfg = {GPIOC, 10U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL,
                            GpioSpeed::MEDIUM, GpioPull::DOWN, 0U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioC.MODER & (3U << 20), (1U << 20));
    EXPECT_EQ(fakeGpioC.OTYPER & (1U << 10), 0U);
    EXPECT_EQ(fakeGpioC.OSPEEDR & (3U << 20), (1U << 20));
    EXPECT_EQ(fakeGpioC.PUPDR & (3U << 20), (2U << 20));
}

TEST_F(GpioTest, configure_AF_SetsClockEnableB)
{
    GpioConfig const cfg = {GPIOB, 7U, GpioMode::ALTERNATE, GpioOutputType::OPEN_DRAIN,
                            GpioSpeed::LOW, GpioPull::UP, 4U};
    Gpio::configure(cfg);
    EXPECT_NE(fakeRcc.AHB2ENR & RCC_AHB2ENR_GPIOBEN, 0U);
    EXPECT_EQ(fakeGpioB.MODER & (3U << 14), (2U << 14));
    EXPECT_NE(fakeGpioB.OTYPER & (1U << 7), 0U);
    EXPECT_EQ(fakeGpioB.AFR[0] & (0xFU << 28), (4U << 28));
}

TEST_F(GpioTest, configure_OutputModeDontSetAF)
{
    // When mode is OUTPUT (not ALTERNATE), AF should not be written
    GpioConfig const cfg = {GPIOA, 5U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL,
                            GpioSpeed::LOW, GpioPull::NONE, 7U};
    Gpio::configure(cfg);
    // AFR should remain 0 since mode != ALTERNATE
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 20), 0U);
}

TEST_F(GpioTest, configure_InputModeDontSetAF)
{
    GpioConfig const cfg = {GPIOA, 3U, GpioMode::INPUT, GpioOutputType::PUSH_PULL,
                            GpioSpeed::LOW, GpioPull::NONE, 9U};
    Gpio::configure(cfg);
    EXPECT_EQ(fakeGpioA.AFR[0] & (0xFU << 12), 0U);
}

// =============================================================================
// Edge cases: pin 0 and pin 15 boundaries
// =============================================================================

TEST_F(GpioTest, edgeCase_Pin0_AllFunctions)
{
    Gpio::setMode(GPIOA, 0U, GpioMode::OUTPUT);
    EXPECT_EQ(fakeGpioA.MODER & 3U, 1U);
    Gpio::setOutputType(GPIOA, 0U, GpioOutputType::OPEN_DRAIN);
    EXPECT_EQ(fakeGpioA.OTYPER & 1U, 1U);
    Gpio::setSpeed(GPIOA, 0U, GpioSpeed::VERY_HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & 3U, 3U);
    Gpio::setPull(GPIOA, 0U, GpioPull::UP);
    EXPECT_EQ(fakeGpioA.PUPDR & 3U, 1U);
    Gpio::setAlternateFunction(GPIOA, 0U, 15U);
    EXPECT_EQ(fakeGpioA.AFR[0] & 0xFU, 15U);
}

TEST_F(GpioTest, edgeCase_Pin15_AllFunctions)
{
    Gpio::setMode(GPIOA, 15U, GpioMode::ALTERNATE);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 30), (2U << 30));
    Gpio::setOutputType(GPIOA, 15U, GpioOutputType::OPEN_DRAIN);
    EXPECT_EQ(fakeGpioA.OTYPER & (1U << 15), (1U << 15));
    Gpio::setSpeed(GPIOA, 15U, GpioSpeed::HIGH);
    EXPECT_EQ(fakeGpioA.OSPEEDR & (3U << 30), (2U << 30));
    Gpio::setPull(GPIOA, 15U, GpioPull::DOWN);
    EXPECT_EQ(fakeGpioA.PUPDR & (3U << 30), (2U << 30));
    Gpio::setAlternateFunction(GPIOA, 15U, 0U);
    EXPECT_EQ(fakeGpioA.AFR[1] & (0xFU << 28), 0U);
}

TEST_F(GpioTest, edgeCase_Pin0_ReadWriteToggle)
{
    fakeGpioA.IDR = 1U;
    EXPECT_TRUE(Gpio::readPin(GPIOA, 0U));
    Gpio::writePin(GPIOA, 0U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 16));
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 0U);
    EXPECT_EQ(fakeGpioA.ODR, 1U);
}

TEST_F(GpioTest, edgeCase_Pin15_ReadWriteToggle)
{
    fakeGpioA.IDR = (1U << 15);
    EXPECT_TRUE(Gpio::readPin(GPIOA, 15U));
    Gpio::writePin(GPIOA, 15U, false);
    EXPECT_EQ(fakeGpioA.BSRR, (1U << 31));
    fakeGpioA.ODR = 0U;
    Gpio::togglePin(GPIOA, 15U);
    EXPECT_EQ(fakeGpioA.ODR, (1U << 15));
}

TEST_F(GpioTest, edgeCase_MultiplePortsIndependent)
{
    Gpio::setMode(GPIOA, 5U, GpioMode::OUTPUT);
    Gpio::setMode(GPIOB, 5U, GpioMode::INPUT);
    Gpio::setMode(GPIOC, 5U, GpioMode::ANALOG);
    EXPECT_EQ(fakeGpioA.MODER & (3U << 10), (1U << 10));
    EXPECT_EQ(fakeGpioB.MODER & (3U << 10), (0U << 10));
    EXPECT_EQ(fakeGpioC.MODER & (3U << 10), (3U << 10));
}
