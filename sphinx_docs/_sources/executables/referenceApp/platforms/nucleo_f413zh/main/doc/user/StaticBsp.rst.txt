..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_f413zh_StaticBsp:

Static BSP
==========

Overview
--------

The ``StaticBsp`` class aggregates the BSP peripherals that must be operational before
the lifecycle starts ("static"): the system timer (DWT cycle counter based) and the
UART terminal (USART3, see :ref:`bspConfig_Uart_F413ZH`). It is instantiated in
``main.cpp`` and initialized from ``main()`` before ``app_main()``, so timing services
and serial output are available during the early lifecycle phases.
