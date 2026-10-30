..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspMcu
======

Overview
--------

The module ``bspMcu`` provides the MCU API for STM32F413xx and STM32G474xx.
``mcu/mcu.h`` selects the matching ST CMSIS device header from
``include/3rdparty/st`` and reuses the shared CMSIS core headers from
``libs/3rdparty/cmsis``. The module also contains the chip startup files and
a function for software system reset.
