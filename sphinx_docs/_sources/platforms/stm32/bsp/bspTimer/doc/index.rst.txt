..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspTimer
========

Overview
--------

The ``bspTimer`` module implements the system timer for STM32 Cortex-M4
targets using the ARM Data Watchpoint and Trace (DWT) cycle counter
(``CYCCNT``) -- a core debug unit register available on all Cortex-M4 devices
-- so no chip-specific peripheral timer is required.

The 32-bit ``CYCCNT`` is extended to a 64-bit monotonic microsecond counter;
updates run under ``SuspendResumeAllInterruptsScopedLock``. The core clock
frequency used for the cycles-to-microseconds conversion is fixed at compile
time:

- 96 MHz for STM32F4 (STM32F413ZH)
- 170 MHz for STM32G4 (STM32G474RE)

The family is selected via the ``STM32_FAMILY_F4`` / ``STM32_FAMILY_G4``
compile definitions from the bspMcu CMake configuration.

The DWT timer is independent of SysTick: FreeRTOS uses SysTick for its tick
interrupt while the BSP system timer uses DWT for high-resolution timestamps.
