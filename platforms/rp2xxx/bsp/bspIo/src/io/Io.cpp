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

bsp::BspReturnCode Io::getConfiguration(uint16_t io, PinConfiguration& cfg)
{
#if 0
    if (io >= NUMBER_OF_IOS)
    {
        return BSP_NOT_SUPPORTED;
    }
    cfg.port                  = fPinConfiguration[io].port;
    cfg.pinNumber             = fPinConfiguration[io].pinNumber;
    GPIO_Type* const portBase = gpioPtrs[cfg.port];
    PORT_Type* const cfgBase  = gpioPrfCfgPtrs[cfg.port];
    cfg.pinCfg                = cfgBase->PCR[cfg.pinNumber];
    cfg.pinCfgExt             = 0;
    if (portBase->PIDR & (1U << cfg.pinNumber))
    {
        cfg.dir = _DISABLED;
    }
    else if (portBase->PDDR & (1U << cfg.pinNumber))
    {
        cfg.dir = _OUT;
    }
    else
    {
        cfg.dir = _IN;
        if (cfgBase->DFER & (1U << cfg.pinNumber))
        {
            cfg.pinCfgExt = FILTER_ACTIVE;
            cfg.pinCfgExt |= (cfgBase->DFCR) ? FILTER_CLOCK_LPO : 0;
            cfg.pinCfgExt |= cfgBase->DFWR & 0x1FU;
        }
    }
    return BSP_OK;
#endif
    return BSP_OK;
}

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

bool Io::getPinIrq(uint16_t io)
{
#if 0
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        PORT_Type* cfgBase = (PORT_Type*)(gpioPrfCfgPtrs[fPinConfiguration[io].port]);
        return ((cfgBase->PCR[fPinConfiguration[io].pinNumber] & clearISR) != 0);
    }
    else
    {
        return false;
    }
#endif
    return {};
}

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

bsp::BspReturnCode Io::resetConfig(uint16_t io)
{
#if 0
    if ((io < NUMBER_OF_IOS) && (fPinConfiguration[io].pinNumber < MAX_GPIO_NUMBER))
    {
        GPIO_Type* portBase = (GPIO_Type*)(gpioPtrs[fPinConfiguration[io].port]);
        PORT_Type* cfgBase  = (PORT_Type*)(gpioPrfCfgPtrs[fPinConfiguration[io].port]);
        // disable
        portBase->PIDR = portBase->PIDR | (1 << fPinConfiguration[io].pinNumber); // default High-Z
        portBase->PDDR = portBase->PDDR & ~(1 << fPinConfiguration[io].pinNumber);
        cfgBase->DFER  = cfgBase->DFER & ~(1 << fPinConfiguration[io].pinNumber);
        cfgBase->PCR[fPinConfiguration[io].pinNumber] = 0;
        return BSP_OK;
    }
    else
    {
        return BSP_NOT_SUPPORTED;
    }
#endif
    return BSP_OK;
}

} /* namespace bios */
