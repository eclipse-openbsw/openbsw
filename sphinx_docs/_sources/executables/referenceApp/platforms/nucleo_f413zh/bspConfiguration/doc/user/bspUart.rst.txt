..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspConfig_Uart_F413ZH:

bspUart Configuration
=====================

Overview
--------

The UART configuration defines the hardware parameters for the on-board
ST-LINK Virtual COM Port of the NUCLEO-F413ZH. A single UART instance
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

   "Peripheral", "USART3 (APB1, 48 MHz = 96 MHz SYSCLK / 2)"
   "TX pin", "PD8, alternate function AF7"
   "RX pin", "PD9, alternate function AF7"
   "Baud rate", "115 200 baud"
   "BRR", "48 000 000 / 115 200 = 416.67 -> 417"

The resulting actual baud rate is 48 MHz / 417 = 115 108 baud (0.080 % error),
well within the UART tolerance.

.. note::

   The UART driver uses polling mode (no DMA, no interrupts). For high-
   throughput or latency-sensitive applications, consider extending the
   ``bspUart`` BSP module with an interrupt-driven or DMA-backed backend.
