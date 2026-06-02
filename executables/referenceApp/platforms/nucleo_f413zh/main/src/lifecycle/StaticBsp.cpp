/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "lifecycle/StaticBsp.h"

#include "bsp/timer/SystemTimer.h"
#include "bsp/uart/UartConfig.h"
#include "mcu/mcu.h"

namespace platform
{
namespace
{
constexpr uint32_t UART_IRQ_PRIORITY = 6U;
} // namespace

void StaticBsp::init()
{
    initSystemTimer();
    bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL).init();
    // Console RX is interrupt-driven (see bspConfiguration stdIo): the RXNE
    // interrupt drains the single-byte data register into a ring buffer so
    // rapid input cannot overrun between console task polls.
    USART3->CR1 |= USART_CR1_RXNEIE;
    NVIC_SetPriority(USART3_IRQn, UART_IRQ_PRIORITY);
    NVIC_EnableIRQ(USART3_IRQn);
}

} // namespace platform
