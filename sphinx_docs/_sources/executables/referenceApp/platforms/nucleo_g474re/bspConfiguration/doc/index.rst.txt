..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspconfig_nucleo_g474re:

bspConfiguration - NUCLEO-G474RE
================================

Overview
--------

The ``bspConfiguration`` module contains hardware-specific configuration for the
NUCLEO-G474RE evaluation board. Driver logic is separated from its configuration
so that users can customise a project for different boards or pin assignments
without modifying driver source code.

The STM32G474RE platform currently exposes a minimal set of BSP peripherals:

- **bspUart** - USART2 configuration (ST-LINK Virtual COM Port on PA2/PA3).
- **bspStdIo** - Standard I/O bridge that routes ``putByteToStdout`` /
  ``getByteFromStdin`` through the configured UART instance.

.. note::

   The ADC, PWM, and digital I/O configuration modules demonstrated by the
   S32K148EVB reference application are not yet ported to this platform.
   They are demonstration features independent of CAN and should be added
   following the same configuration pattern in a future change.

Hardware Summary
++++++++++++++++

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32G474RE (Arm Cortex-M4F, 170 MHz, single-precision FPU)"
   "UART peripheral", "USART2 - routed to ST-LINK/V3 Virtual COM Port"
   "UART TX pin", "PA2 (AF7)"
   "UART RX pin", "PA3 (AF7)"
   "UART baud rate", "115 200 baud (BRR = 1476 at 170 MHz APB1)"
   "CAN peripheral", "FDCAN1 (configured in CanSystem, not bspConfiguration)"

.. toctree::
   :hidden:

   user/index
