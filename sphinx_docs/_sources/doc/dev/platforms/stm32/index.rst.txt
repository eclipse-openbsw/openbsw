..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _stm32_overview:

STM32 Nucleo Boards
===================

Overview
--------

- The STM32 platform brings Eclipse OpenBSW to STMicroelectronics Nucleo development
  boards: low-cost, widely available hardware with an on-board ST-LINK
  debugger/programmer, so no external debug probe is needed.
- Eclipse OpenBSW provides a reference application for these boards which can be built
  out-of-the-box and flashed onto the board, providing an immediate starting point for
  further development and learning.

Supported boards
----------------

.. csv-table:: STM32 boards
   :header: "Board", "MCU", "Core", "Flash / RAM", "CAN peripheral", "Form factor"
   :widths: 18, 14, 16, 14, 14, 12

   "NUCLEO-G474RE", "STM32G474RE", "Cortex-M4F, 170 MHz", "512 KB / 128 KB", "FDCAN", "Nucleo-64"
   "NUCLEO-F413ZH", "STM32F413ZH", "Cortex-M4F, 100 MHz", "1.5 MB / 320 KB", "bxCAN", "Nucleo-144"

Scope
-----

The reference application on the STM32 boards provides:

- lifecycle management and the serial console over the ST-LINK virtual COM port,
- CAN communication (FDCAN on the STM32G474RE, bxCAN on the STM32F413ZH),
- DoCAN transport and UDS diagnostics (see :ref:`feature_diagnostics`),
- a choice of RTOS: FreeRTOS (default) or ThreadX, selected via the
  ``BUILD_TARGET_RTOS`` CMake option.

The platform options enabled per board are defined in
``executables/referenceApp/platforms/<board_name>/Options.cmake``.

Reference Links
---------------

1. **NUCLEO-G474RE**:
    - `NUCLEO-G474RE Product Page <https://www.st.com/en/evaluation-tools/nucleo-g474re.html>`_
    - `STM32G474RE MCU Page <https://www.st.com/en/microcontrollers-microprocessors/stm32g474re.html>`_

2. **NUCLEO-F413ZH**:
    - `NUCLEO-F413ZH Product Page <https://www.st.com/en/evaluation-tools/nucleo-f413zh.html>`_
    - `STM32F413ZH MCU Page <https://www.st.com/en/microcontrollers-microprocessors/stm32f413zh.html>`_

3. **Software and Tools**:
    - `STM32CubeProgrammer <https://www.st.com/en/development-tools/stm32cubeprog.html>`_
    - `OpenOCD <https://openocd.org/>`_

Availability
------------

Both boards are active STMicroelectronics products, available at low cost from the
usual electronics distributors.

Build environment
-----------------

For instructions on building for this platform see :ref:`learning_setup`
