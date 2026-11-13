..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_g474re_main_entry:

main File
=========

The ``main.cpp`` module is the entry point of the NUCLEO-G474RE reference application.
``SystemInit()``, called from ``Reset_Handler``, configures the PLL to 170 MHz, switches
on the LD2 LED (PA5), and sets up early USART2 TX output for use before the BSP UART
driver is initialized.

``main()`` constructs the safety supervisor, initializes the static BSP (system timer
and UART terminal, see :ref:`nucleo_g474re_StaticBsp`) and calls ``app_main()``, which
starts the lifecycle manager and the FreeRTOS scheduler. The task contexts are defined
in ``async/Config.h`` of ``referenceApp/asyncCoreConfiguration``: ``TASK_BACKGROUND``,
``TASK_BSP``, ``TASK_UDS``, ``TASK_DEMO``, ``TASK_ETHERNET``, ``TASK_CAN``,
``TASK_SYSADMIN`` and ``TASK_SAFETY``.

``platformLifecycleAdd()`` registers the platform's ``CanSystem`` at runlevel 2 in the
``TASK_CAN`` context; the remaining components - including ``RuntimeSystem`` and
``SafetySystem`` at runlevel 1 - are registered by the shared application code.

``HardFault_Handler`` branches to ``customHardFaultHandler`` from the
``hardFaultHandler`` module, which dumps the registers to RAM and ends in
``HardFault_Handler_Final``, which performs a system reset.
