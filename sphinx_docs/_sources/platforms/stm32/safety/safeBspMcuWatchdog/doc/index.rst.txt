..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

safeBspMcuWatchdog
==================

Overview
--------

The ``safeBspMcuWatchdog`` module provides the STM32 IWDG (independent
watchdog) driver behind the same ``safety::bsp::Watchdog`` API as the
S32K1xx module of the same name.

IWDG specifics:

- Clocked by the untrimmed LSI oscillator (~32 kHz nominal, 17--47 kHz
  across devices and conditions), independent of the system clock.
- 12-bit downcounter with a /4../256 prescaler; timeouts from 1 ms up to
  ~32 s at 32 kHz.
- Once started the watchdog cannot be stopped; ``disableWatchdog()`` is
  therefore a documented no-op.

``enableWatchdog()`` computes the smallest prescaler that fits the requested
timeout, writes the configuration with bounded waits on the ``PVU``/``RVU``
update flags, and starts the counter. ``checkWatchdogConfiguration()``
verifies the programmed prescaler and reload against an expected timeout.
``isResetFromWatchdog()`` and ``clearResetFlag()`` expose the
``RCC_CSR_IWDGRSTF`` reset-cause flag.
