/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <io/Gpio.h>

namespace bios
{
namespace pins
{

#if defined(STM32F413xx)
// NUCLEO-F413ZH board pins

// LD1 (green LED)
static constexpr GpioConfig LED1
    = {GPIOB, 0U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

// LD2 (blue LED)
static constexpr GpioConfig LED2
    = {GPIOB, 7U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

// LD3 (red LED)
static constexpr GpioConfig LED3
    = {GPIOB, 14U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

// User button B1 (external pull-down on board)
static constexpr GpioConfig USER_BUTTON
    = {GPIOC, 13U, GpioMode::INPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

static constexpr GpioConfig CAN1_RX = {
    GPIOD, 0U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL, GpioSpeed::HIGH, GpioPull::UP, 9U};

static constexpr GpioConfig CAN1_TX
    = {GPIOD,
       1U,
       GpioMode::ALTERNATE,
       GpioOutputType::PUSH_PULL,
       GpioSpeed::VERY_HIGH,
       GpioPull::NONE,
       9U};

#elif defined(STM32G474xx)
// NUCLEO-G474RE board pins

// LD2 (green LED)
static constexpr GpioConfig LED2
    = {GPIOA, 5U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

// User button B1 (external pull-up on board)
static constexpr GpioConfig USER_BUTTON
    = {GPIOC, 13U, GpioMode::INPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

static constexpr GpioConfig FDCAN1_RX = {
    GPIOA, 11U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL, GpioSpeed::HIGH, GpioPull::UP, 9U};

static constexpr GpioConfig FDCAN1_TX
    = {GPIOA,
       12U,
       GpioMode::ALTERNATE,
       GpioOutputType::PUSH_PULL,
       GpioSpeed::VERY_HIGH,
       GpioPull::NONE,
       9U};

#endif

} // namespace pins
} // namespace bios
