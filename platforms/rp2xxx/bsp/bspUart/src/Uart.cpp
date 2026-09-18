/********************************************************************************
 * Copyright (c) 2026 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/Uart.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"

namespace bsp
{
namespace
{
// The reference application uses the RP2040's default UART, which is routed to
// the debug header of the Pi Pico board.
uart_inst_t* const TERMINAL_UART      = uart0;
constexpr uint32_t TERMINAL_BAUD_RATE = PICO_DEFAULT_UART_BAUD_RATE;
constexpr uint TERMINAL_TX_PIN        = PICO_DEFAULT_UART_TX_PIN;
constexpr uint TERMINAL_RX_PIN        = PICO_DEFAULT_UART_RX_PIN;

} // namespace

Uart::Uart(Id const id) : _id(id) {}

void Uart::init()
{
    gpio_set_function(TERMINAL_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(TERMINAL_RX_PIN, GPIO_FUNC_UART);
    (void)uart_init(TERMINAL_UART, TERMINAL_BAUD_RATE);
    _initialized = true;
}

bool Uart::isInitialized() const { return _initialized; }

bool Uart::waitForTxReady()
{
    while (!uart_is_writable(TERMINAL_UART))
    {
        // busy wait until the transmit fifo has room
    }
    return true;
}

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    for (uint8_t const byte : data)
    {
        (void)waitForTxReady();
        uart_putc_raw(TERMINAL_UART, static_cast<char>(byte));
    }
    return data.size();
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytesRead = 0U;
    for (uint8_t& byte : data)
    {
        if (!uart_is_readable(TERMINAL_UART))
        {
            break;
        }
        byte = static_cast<uint8_t>(uart_getc(TERMINAL_UART));
        ++bytesRead;
    }
    return bytesRead;
}

Uart& Uart::getInstance(Id const id)
{
    static Uart terminal{Id::TERMINAL};
    (void)id;
    return terminal;
}

} // namespace bsp
