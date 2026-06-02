..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_f413zh_main_entry:

main File
=========

The ``main.cpp`` module is the entry point of the NUCLEO-F413ZH reference application.
``SystemInit()``, called from ``Reset_Handler``, configures the PLL to 96 MHz, switches
on the LD1 LED (PB0), and sets up early USART3 TX output for use before the BSP UART
driver is initialized.

``main()`` constructs the safety supervisor, initializes the static BSP (system timer
and UART terminal, see :ref:`nucleo_f413zh_StaticBsp`) and calls ``app_main()``, which
starts the lifecycle manager and the RTOS scheduler. The IWDG refresh writes in
``main()`` are no-ops until the SafetyManager enables the watchdog at runlevel 1. The
task contexts are defined in ``async/Config.h`` of ``referenceApp/asyncCoreConfiguration``:
``TASK_BACKGROUND``, ``TASK_BSP``, ``TASK_UDS``, ``TASK_DEMO``, ``TASK_ETHERNET``,
``TASK_CAN``, ``TASK_SYSADMIN`` and ``TASK_SAFETY``.

``platformLifecycleAdd()`` registers the platform's ``CanSystem`` at runlevel 2 in the
``TASK_CAN`` context; the remaining components - including ``RuntimeSystem`` and
``SafetySystem`` at runlevel 1 and, since the platform enables
``PLATFORM_SUPPORT_TRANSPORT`` and ``PLATFORM_SUPPORT_UDS``, the transport, DoCAN and
UDS systems - are registered by the shared application code.

``setupApplicationsIsr()`` is a no-op; CAN NVIC setup happens in ``CanSystem::run()``.

``HardFault_Handler`` branches to ``customHardFaultHandler`` from the
``hardFaultHandler`` module, which dumps the registers to RAM and ends in
``HardFault_Handler_Final``, which performs a system reset.
