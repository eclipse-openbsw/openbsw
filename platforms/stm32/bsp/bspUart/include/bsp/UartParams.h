// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

#pragma once

#include "bsp/Uart.h"

#include <mcu/mcu.h>

namespace bsp
{

struct Uart::UartConfig
{
    USART_TypeDef* usart;
    GPIO_TypeDef* gpioPort;
    uint8_t txPin;
    uint8_t rxPin;
    uint8_t af;
    uint32_t brr;
    uint32_t rccGpioEnBit;
    uint32_t rccUsartEnBit;
    uint32_t volatile* rccGpioEnReg;
    uint32_t volatile* rccUsartEnReg;
};

} // namespace bsp
