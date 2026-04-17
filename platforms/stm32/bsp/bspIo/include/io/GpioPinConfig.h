// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   GpioPinConfig.h
 * \brief  Board-level GPIO pin definitions for STM32 NUCLEO boards.
 *
 * Defines named pin configurations for the NUCLEO-F413ZH and NUCLEO-G474RE
 * boards.  Each entry is a GpioConfig struct ready for Gpio::configure().
 */

#pragma once

#include <io/Gpio.h>

namespace bios
{
namespace pins
{

#if defined(STM32F413xx)
// ---- NUCLEO-F413ZH board pins ----

/// LD1 (green LED) — PB0, push-pull output
static constexpr GpioConfig LED1 = {
    GPIOB, 0U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// LD2 (blue LED) — PB7, push-pull output
static constexpr GpioConfig LED2 = {
    GPIOB, 7U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// LD3 (red LED) — PB14, push-pull output
static constexpr GpioConfig LED3 = {
    GPIOB, 14U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// User button (B1) — PC13, input with no pull (external pull-down)
static constexpr GpioConfig USER_BUTTON = {
    GPIOC, 13U, GpioMode::INPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// CAN1 RX — PD0, AF9
static constexpr GpioConfig CAN1_RX = {
    GPIOD, 0U, GpioMode::ALTERNATE, GpioOutputType::PUSH_PULL, GpioSpeed::HIGH, GpioPull::UP, 9U};

/// CAN1 TX — PD1, AF9
static constexpr GpioConfig CAN1_TX = {
    GPIOD,
    1U,
    GpioMode::ALTERNATE,
    GpioOutputType::PUSH_PULL,
    GpioSpeed::VERY_HIGH,
    GpioPull::NONE,
    9U};

#elif defined(STM32G474xx)
// ---- NUCLEO-G474RE board pins ----

/// LD2 (green LED) — PA5, push-pull output
static constexpr GpioConfig LED2 = {
    GPIOA, 5U, GpioMode::OUTPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// User button (B1) — PC13, input with no pull (external pull-up on NUCLEO)
static constexpr GpioConfig USER_BUTTON = {
    GPIOC, 13U, GpioMode::INPUT, GpioOutputType::PUSH_PULL, GpioSpeed::LOW, GpioPull::NONE, 0U};

/// FDCAN1 RX — PA11, AF9
static constexpr GpioConfig FDCAN1_RX = {
    GPIOA,
    11U,
    GpioMode::ALTERNATE,
    GpioOutputType::PUSH_PULL,
    GpioSpeed::HIGH,
    GpioPull::UP,
    9U};

/// FDCAN1 TX — PA12, AF9
static constexpr GpioConfig FDCAN1_TX = {
    GPIOA,
    12U,
    GpioMode::ALTERNATE,
    GpioOutputType::PUSH_PULL,
    GpioSpeed::VERY_HIGH,
    GpioPull::NONE,
    9U};

#endif

} // namespace pins
} // namespace bios
