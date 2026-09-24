/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "sci/SciDevice.h"

#include "bsp/clock/boardClock.h"
#include "bsp/sci/sciConfiguration.h"
#include "hardware/uart.h"
#include "mcu/mcu.h"

extern "C"
{
void sciInit(uint8_t const speedMode) {}

uint8_t sciGetRxReady() { return uart_is_readable(uart0) ? 1 : 0; }

uint8_t sciGeth()
{
    //    return static_cast<uint8_t>(sciConfiguration.sci->DATA & 0xFFU);
    return uart_getc(uart0);
}

uint8_t sciGetTxNotReady() { return uart_is_writable(uart0) ? 0 : 1; }

void sciPuth(int const c) { uart_putc_raw(uart0, c); }

uint8_t sciGetInitState()
{
    return 1; // initialized
}

} // extern "C"
