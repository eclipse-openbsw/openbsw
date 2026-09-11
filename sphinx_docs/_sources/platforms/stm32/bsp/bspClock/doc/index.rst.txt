..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspClock
========

Overview
--------

The ``bspClock`` module configures the system clock PLL for STM32F4 and STM32G4
targets. Each family has its own translation unit (``clockConfig_f4.cpp``,
``clockConfig_g4.cpp``); the public API is a single ``extern "C"`` function
``configurePll()`` declared in ``clock/clockConfig.h``. It is called from the
startup code before ``main()`` and must not use heap, RTOS primitives, or BSW
services. On a PLL or oscillator timeout the system stays on HSI 16 MHz and
``SystemCoreClock`` is not updated.

STM32F4 (STM32F413ZH) -- 96 MHz
-------------------------------

- Clock source: HSE 8 MHz bypass (ST-LINK MCO on NUCLEO-F413ZH)
- PLL: M = 8, N = 384, P = 4 -> SYSCLK = 96 MHz
- AHB = 96 MHz, APB1 = 48 MHz (max 50 MHz per datasheet), APB2 = 96 MHz
- Flash latency: 3 wait states at 3.3 V, prefetch and caches enabled

STM32G4 (STM32G474RE) -- 170 MHz
--------------------------------

- Clock source: HSI 16 MHz. HSE is not used because solder bridge SB15 may
  not route the ST-LINK V3 MCO to PH0/OSC_IN on all NUCLEO-G474RE revisions.
- PLL: M = 4, N = 85, R = 2 -> SYSCLK = 170 MHz (voltage Range 1 boost mode)
- AHB = APB1 = APB2 = 170 MHz
- Flash latency: 4 wait states; ``FLASH_ACR`` is written read-modify-write to
  preserve ``DBG_SWEN``. The switch above 150 MHz follows the AHB prescaler
  transition sequence from RM0440 section 6.1.5.
