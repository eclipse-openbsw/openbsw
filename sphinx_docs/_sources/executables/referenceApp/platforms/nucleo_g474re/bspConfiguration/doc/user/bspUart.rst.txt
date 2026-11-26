..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspConfig_Uart_G474RE:

bspUart Configuration
=====================

Overview
--------

The UART configuration defines the hardware parameters for the on-board
ST-LINK Virtual COM Port of the NUCLEO-G474RE. A single UART instance
(``TERMINAL``) is configured for debug logging and interactive console use.

``UartConfig.h`` declares the ``Uart::Id`` enumeration of available UART
instances, and ``UartConfig.cpp`` maps each ``Uart::Id`` to its hardware
parameters and provides the singleton accessor
``bsp::Uart::getInstance(Uart::Id)``.

Configuration
-------------

.. csv-table::
   :header: "Parameter", "Value"
   :widths: 30, 70
   :width: 100%

   "Peripheral", "USART2 (APB1)"
   "TX pin", "PA2, alternate function AF7"
   "RX pin", "PA3, alternate function AF7"
   "Baud rate", "115 200 baud"
   "BRR", "170 000 000 / 115 200 = 1475.69 -> 1476"

The resulting actual baud rate is 170 MHz / 1476 = 115 176 baud (0.021 % error),
well within the UART tolerance.

.. note::

   The UART driver uses polling mode (no DMA, no interrupts). For high-
   throughput or latency-sensitive applications, consider extending the
   ``bspUart`` BSP module with an interrupt-driven or DMA-backed backend.
