..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

SafeWatchdog
============

The ``SafeWatchdog`` manages the watchdog by handling its initialization and servicing.
It also adds safety checks before each watchdog trigger, these checks include validating
the watchdog's configuration and timeout value, as well as verifying that it is not serviced more
frequently than allowed. If any of these checks fail, it initiates a software system reset via
safeMonitor.

Public API
----------

The public API of `SafeWatchdog` consists of a constructor and following methods:

.. sourceinclude:: include/safeWatchdog/SafeWatchdog.h
    :start-after: PUBLIC_API_START
    :end-before: PUBLIC_API_END
    :language: c++
    :dedent: 0
