..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_f413zh_main:

main - NUCLEO-F413ZH
====================

Overview
--------

The ``main`` module contains the platform-specific entry point, startup code, linker
script, and lifecycle systems for the NUCLEO-F413ZH evaluation board:

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32F413ZH - Arm Cortex-M4, single-precision FPU"
   "Core clock", "96 MHz (HSI with PLL)"
   "Flash", "1.5 MB"
   "SRAM", "320 KB (SRAM1 256 KB + SRAM2 64 KB)"
   "CAN", "bxCAN CAN1 - PD0 (RX) / PD1 (TX), AF9, 500 kbit/s"
   "Debug UART", "USART3 - PD8 TX (AF7), 115200 baud, routed to ST-LINK VCP"
   "User LED", "LD1 (green) on PB0 (active-high)"
   "Debug interface", "On-board ST-LINK/V2-1 (SWD)"

See :ref:`bspconfig_nucleo_f413zh` for the UART and standard I/O configuration, and the
user documentation for the sub-modules:

.. toctree::
   user/index
