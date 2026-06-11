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

#include <mcu/mcu.h>
#include <stdint.h>

namespace bios
{

// MODER register values
enum class GpioMode : uint8_t
{
    INPUT     = 0U,
    OUTPUT    = 1U,
    ALTERNATE = 2U,
    ANALOG    = 3U
};

// OTYPER register values
enum class GpioOutputType : uint8_t
{
    PUSH_PULL  = 0U,
    OPEN_DRAIN = 1U
};

// OSPEEDR register values
enum class GpioSpeed : uint8_t
{
    LOW       = 0U,
    MEDIUM    = 1U,
    HIGH      = 2U,
    VERY_HIGH = 3U
};

// PUPDR register values
enum class GpioPull : uint8_t
{
    NONE = 0U,
    UP   = 1U,
    DOWN = 2U
};

struct GpioConfig
{
    GPIO_TypeDef* port;
    uint8_t pin; // 0-15
    GpioMode mode;
    GpioOutputType otype;
    GpioSpeed speed;
    GpioPull pull;
    uint8_t af; // Alternate function number (0-15)
};

class Gpio
{
public:
    Gpio() = delete;

    static void enablePortClock(GPIO_TypeDef* port);
    static void configure(GpioConfig const& cfg);
    static void setMode(GPIO_TypeDef* port, uint8_t pin, GpioMode mode);
    static void setOutputType(GPIO_TypeDef* port, uint8_t pin, GpioOutputType otype);
    static void setSpeed(GPIO_TypeDef* port, uint8_t pin, GpioSpeed speed);
    static void setPull(GPIO_TypeDef* port, uint8_t pin, GpioPull pull);
    static void setAlternateFunction(GPIO_TypeDef* port, uint8_t pin, uint8_t af);
    static bool readPin(GPIO_TypeDef* port, uint8_t pin);
    static void writePin(GPIO_TypeDef* port, uint8_t pin, bool high);
    static void togglePin(GPIO_TypeDef* port, uint8_t pin);
};

} // namespace bios
