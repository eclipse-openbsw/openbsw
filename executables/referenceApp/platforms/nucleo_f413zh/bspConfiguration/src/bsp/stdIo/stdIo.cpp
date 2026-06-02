/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/uart/UartConfig.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

namespace
{
// USART3 has a single-byte receive data register. The console task polls
// stdin, so under bursts of input bytes arrive faster (~87 us at 115200)
// than any task-level poll can be scheduled and the register overruns.
// The RXNE interrupt therefore drains USART3 into this single-producer /
// single-consumer ring; getByteFromStdin consumes from the ring only.
constexpr size_t RX_RING_SIZE = 128U; // power of two
uint8_t volatile rxRing[RX_RING_SIZE];
size_t volatile rxHead = 0U; // written by ISR only
size_t volatile rxTail = 0U; // written by consumer only
} // namespace

extern "C" void putByteToStdout(uint8_t const byte)
{
    static bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
    uart.write(etl::span<uint8_t const>(&byte, 1U));
}

extern "C" void stdIoRxIsr()
{
    // Reading SR then DR clears RXNE and a pending overrun condition.
    while ((USART3->SR & USART_SR_RXNE) != 0U)
    {
        uint8_t const byte = static_cast<uint8_t>(USART3->DR & 0xFFU);
        size_t const next  = (rxHead + 1U) % RX_RING_SIZE;
        if (next != rxTail) // on overflow the newest byte is dropped
        {
            rxRing[rxHead] = byte;
            rxHead         = next;
        }
    }
}

extern "C" int32_t getByteFromStdin()
{
    // Uart::init() rewrites CR1 wholesale (TE|RE|UE) and may run again after
    // bring-up, silently dropping RXNEIE - re-assert it so the RX interrupt
    // survives any re-initialization of the UART.
    if ((USART3->CR1 & USART_CR1_RXNEIE) == 0U)
    {
        USART3->CR1 |= USART_CR1_RXNEIE;
    }
    if (rxTail == rxHead)
    {
        return -1;
    }
    uint8_t const byte = rxRing[rxTail];
    rxTail             = (rxTail + 1U) % RX_RING_SIZE;
    return byte;
}
