..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_g474re_main:

main - NUCLEO-G474RE
====================

Overview
--------

The ``main`` module contains the platform-specific entry point, startup code, linker
script, and lifecycle systems for the NUCLEO-G474RE evaluation board:

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32G474RE - Arm Cortex-M4F, single-precision FPU"
   "Core clock", "170 MHz (HSI16 with PLL, boost mode enabled)"
   "Flash", "512 KB"
   "SRAM", "128 KB (SRAM1 80 KB + SRAM2 16 KB + CCM SRAM 32 KB)"
   "CAN", "FDCAN1 - PA11 (RX) / PA12 (TX), AF9, 500 kbit/s"
   "Debug UART", "USART2 - PA2 TX (AF7), 115200 baud, routed to ST-LINK VCP"
   "User LED", "LD2 on PA5 (active-high)"
   "Debug interface", "On-board ST-LINK/V3 (SWD)"

See :ref:`bspconfig_nucleo_g474re` for the UART and standard I/O configuration, and the
user documentation for the sub-modules:

.. toctree::
   user/index
