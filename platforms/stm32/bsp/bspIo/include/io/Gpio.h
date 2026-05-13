// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   Gpio.h
 * \brief  Low-level GPIO abstraction for STM32 Cortex-M4 platforms.
 *
 * Provides register-level control of STM32 GPIO pins: mode configuration,
 * output type, speed, pull-up/pull-down, alternate function selection,
 * digital read/write, and atomic set/reset via BSRR.
 *
 * Designed for use by higher-level bspIo (DigitalInput / DigitalOutput)
 * and BSP pin configuration modules.
 */

#pragma once

#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

/**
 * \brief STM32 GPIO pin mode (MODER register).
 */
enum class GpioMode : uint8_t
{
    INPUT     = 0U, ///< Input (reset state for most pins)
    OUTPUT    = 1U, ///< General-purpose output
    ALTERNATE = 2U, ///< Alternate function
    ANALOG    = 3U  ///< Analog (ADC/DAC)
};

/**
 * \brief STM32 GPIO output type (OTYPER register).
 */
enum class GpioOutputType : uint8_t
{
    PUSH_PULL  = 0U, ///< Push-pull (default)
    OPEN_DRAIN = 1U  ///< Open-drain
};

/**
 * \brief STM32 GPIO output speed (OSPEEDR register).
 */
enum class GpioSpeed : uint8_t
{
    LOW       = 0U, ///< Low speed
    MEDIUM    = 1U, ///< Medium speed
    HIGH      = 2U, ///< High speed
    VERY_HIGH = 3U  ///< Very high speed
};

/**
 * \brief STM32 GPIO pull-up/pull-down configuration (PUPDR register).
 */
enum class GpioPull : uint8_t
{
    NONE = 0U, ///< No pull-up/pull-down
    UP   = 1U, ///< Pull-up
    DOWN = 2U  ///< Pull-down
};

/**
 * \brief Complete pin configuration descriptor.
 */
struct GpioConfig
{
    GPIO_TypeDef* port;    ///< GPIO port base address (GPIOA, GPIOB, ...)
    uint8_t pin;           ///< Pin number (0-15)
    GpioMode mode;         ///< Pin mode
    GpioOutputType otype;  ///< Output type (only relevant for output/AF)
    GpioSpeed speed;       ///< Output speed
    GpioPull pull;         ///< Pull-up/pull-down
    uint8_t af;            ///< Alternate function number (0-15)
};

/**
 * \brief Low-level GPIO driver for STM32.
 *
 * All methods are static — no instance required.
 */
class Gpio
{
public:
    Gpio() = delete;

    /**
     * \brief Enable the clock for a GPIO port.
     * \param port  GPIO port base address (GPIOA..GPIOH).
     */
    static void enablePortClock(GPIO_TypeDef* port);

    /**
     * \brief Configure a pin according to the given descriptor.
     * \param cfg  Full pin configuration.
     */
    static void configure(GpioConfig const& cfg);

    /**
     * \brief Set a pin's mode (MODER register).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param mode  Desired mode.
     */
    static void setMode(GPIO_TypeDef* port, uint8_t pin, GpioMode mode);

    /**
     * \brief Set a pin's output type (OTYPER register).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param otype Output type.
     */
    static void setOutputType(GPIO_TypeDef* port, uint8_t pin, GpioOutputType otype);

    /**
     * \brief Set a pin's output speed (OSPEEDR register).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param speed Output speed.
     */
    static void setSpeed(GPIO_TypeDef* port, uint8_t pin, GpioSpeed speed);

    /**
     * \brief Set a pin's pull-up/pull-down (PUPDR register).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param pull  Pull configuration.
     */
    static void setPull(GPIO_TypeDef* port, uint8_t pin, GpioPull pull);

    /**
     * \brief Set a pin's alternate function (AFR[0] or AFR[1]).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param af    Alternate function number (0-15).
     */
    static void setAlternateFunction(GPIO_TypeDef* port, uint8_t pin, uint8_t af);

    /**
     * \brief Read a pin's input level.
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \return true if the pin reads high, false if low.
     */
    static bool readPin(GPIO_TypeDef* port, uint8_t pin);

    /**
     * \brief Write a pin's output level (atomic via BSRR).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     * \param high  true to set the pin high, false to set it low.
     */
    static void writePin(GPIO_TypeDef* port, uint8_t pin, bool high);

    /**
     * \brief Toggle a pin's output level (read-modify-write on ODR).
     * \param port  GPIO port base address.
     * \param pin   Pin number (0-15).
     */
    static void togglePin(GPIO_TypeDef* port, uint8_t pin);
};

} // namespace bios
