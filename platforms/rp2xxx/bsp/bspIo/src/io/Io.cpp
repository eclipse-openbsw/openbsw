/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "io/Io.h"

#include "mcu/mcu.h"

#include <pico/stdlib.h>

#include <pico.h>

using namespace bsp;

namespace bios
{

#undef BSP_IO_PIN_CONFIGURATION
#define BSP_IO_PIN_CONFIGURATION 1
#include "bsp/io/io/ioConfiguration.h"

// GPIO_Type* const Io::gpioPtrs[_PORTS_MAX_]       = GPIO_BASE_PTRS;
// PORT_Type* const Io::gpioPrfCfgPtrs[_PORTS_MAX_] = PORT_BASE_PTRS;

BspReturnCode Io::setDefaultConfiguration(uint16_t io)
{
    if (io < NUMBER_OF_IOS)
    {
        return setConfiguration(io, fPinConfiguration[io]);
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

BspReturnCode Io::setConfiguration(uint16_t io, PinConfiguration const& cfg)
{
    if (io < NUMBER_OF_IOS)
    {
        // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
        // so we can use normal GPIO functionality to turn the led on and off
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

bsp::BspReturnCode Io::getConfiguration(uint16_t io, PinConfiguration& cfg) { return BSP_OK; }

uint32_t Io::getPinNumber(uint16_t io)
{
    // return fPinConfiguration[io].pinNumber;
    return {};
}

/**
 * @disc: get pin
 * @par : pin from configuration enum
 * @ret : true - 1, false 0
 */
bool Io::getPin(uint16_t io)
{
    if (io < NUMBER_OF_IOS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Io::getPinIrq(uint16_t io) { return {}; }

BspReturnCode Io::setPin(uint16_t io, bool level)
{
    if (io < NUMBER_OF_IOS)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, level);
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
}

bsp::BspReturnCode Io::resetConfig(uint16_t io) { return BSP_OK; }

} /* namespace bios */
