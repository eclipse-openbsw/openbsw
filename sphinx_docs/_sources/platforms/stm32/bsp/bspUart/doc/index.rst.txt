..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspUart Driver
==============

Overview
--------

The ``bspUart`` module provides a polling-mode UART driver for debug console
output on STM32 targets. It is used by the logger subsystem and the command
shell for human-readable output over the ST-LINK Virtual COM Port (VCP). The
driver is the ``bsp::Uart`` class and satisfies the ``UartConcept``
compile-time interface check (``BSP_UART_CONCEPT_CHECKER``).

All I/O is synchronous polling; no DMA or interrupts are used, which avoids
NVIC configuration dependencies at the cost of blocking the caller during
transmission.

Configuration
-------------

Each UART instance is described by a ``UartConfig`` struct in a compile-time
table (``_uartConfigs[]``): USART peripheral, GPIO port and TX/RX pins,
alternate function number, and baud rate register value (typically 115200
baud for the debug console).

Board configuration:

- **NUCLEO-F413ZH** -- USART3 on PD8 (TX) / PD9 (RX), AF7. VCP via ST-LINK
  V2-1.
- **NUCLEO-G474RE** -- USART2 on PA2 (TX) / PA3 (RX), AF7. VCP via ST-LINK
  V3.
