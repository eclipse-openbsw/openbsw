..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _bspconfig_nucleo_f413zh:

bspConfiguration - NUCLEO-F413ZH
================================

Overview
--------

The ``bspConfiguration`` module contains hardware-specific configuration for the
NUCLEO-F413ZH evaluation board. Driver logic is separated from its configuration
so that users can customise a project for different boards or pin assignments
without modifying driver source code.

The STM32F413ZH platform currently exposes a minimal set of BSP peripherals:

- **bspUart** - USART3 configuration (ST-LINK Virtual COM Port on PD8/PD9).
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

   "MCU", "STM32F413ZH (Arm Cortex-M4F, 96 MHz, single-precision FPU)"
   "UART peripheral", "USART3 - routed to ST-LINK/V2-1 Virtual COM Port"
   "UART TX pin", "PD8 (AF7)"
   "UART RX pin", "PD9 (AF7)"
   "UART baud rate", "115 200 baud (BRR = 417 at 48 MHz APB1)"
   "CAN peripheral", "CAN1 (bxCAN, configured in CanSystem, not bspConfiguration)"

.. toctree::
   :hidden:

   user/index
