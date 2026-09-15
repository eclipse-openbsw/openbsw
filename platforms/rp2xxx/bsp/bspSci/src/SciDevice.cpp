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
void sciInit(uint8_t const speedMode)
{
#if 0
    (void)speedMode;

    (void)bios::Io::setDefaultConfiguration(sciConfiguration.txPin);
    (void)bios::Io::setDefaultConfiguration(sciConfiguration.rxPin);
    sciConfiguration.sci->GLOBAL = 0;
    sciConfiguration.sci->CTRL   = 0;

    sciConfiguration.sci->PINCFG = 0;
    sciConfiguration.sci->BAUD   = 0;
    sciConfiguration.sci->BAUD   = sciConfiguration.fpBaudRate[0].BAUD;
    sciConfiguration.sci->STAT   = 0xFFFFFFFFU;
    sciConfiguration.sci->STAT   = 0;
    sciConfiguration.sci->MODIR  = 0;
    sciConfiguration.sci->FIFO   = 0;
    sciConfiguration.sci->WATER  = 0;
    // Last
    sciConfiguration.sci->CTRL   = LPUART_CTRL_RE(1U) + LPUART_CTRL_TE(1U);
#endif
}

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
