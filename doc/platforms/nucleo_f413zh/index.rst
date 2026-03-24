.. _nucleo_f413zh_overview:

NUCLEO-F413ZH (STM32F413ZH)
============================

Overview
--------

- The NUCLEO-F413ZH is a development board from STMicroelectronics featuring the STM32F413ZH
  microcontroller (ARM Cortex-M4, 100 MHz, 1.5 MB Flash, 320 KB SRAM) in an LQFP144 package.
- It includes an on-board ST-LINK/V2-1 debugger/programmer, Arduino Uno V3 and ST morpho
  connectors, and requires no separate probe for development.
- Eclipse OpenBSW provides a reference application for this platform demonstrating UART console
  output and bxCAN communication, enabling immediate evaluation of the BSW stack on affordable
  STM32 hardware.

Reference Links
---------------

1. **NUCLEO-F413ZH Product Page**:
    - `NUCLEO-F413ZH <https://www.st.com/en/evaluation-tools/nucleo-f413zh.html>`_

2. **NUCLEO-F413ZH Documentation**:
    - `User Manual: UM1974 STM32 Nucleo-144 boards <https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf>`_
    - `Schematic <https://www.st.com/resource/en/schematic_pack/nucleo_144pins_sch.zip>`_

3. **STM32F413ZH MCU Documentation**:
    - `Reference Manual: RM0430 <https://www.st.com/resource/en/reference_manual/rm0430-stm32f413423-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`_
    - `Data Sheet: DS11581 <https://www.st.com/resource/en/datasheet/stm32f413zh.pdf>`_

4. **Software and Tools**:
    - `STM32CubeProgrammer <https://www.st.com/en/development-tools/stm32cubeprog.html>`_
    - `ARM GNU Toolchain <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`_

Build environment
-----------------
For instructions on building for this platform see :ref:`learning_setup`

Quick build::

    cmake --preset nucleo-f413zh-freertos-gcc
    cmake --build --preset nucleo-f413zh-freertos-gcc

Flash using STM32CubeProgrammer::

    STM32_Programmer_CLI -c port=SWD -d build/nucleo-f413zh-freertos-gcc/RelWithDebInfo/referenceApp.elf -v -rst

Connections
-----------

.. csv-table:: Pin Configuration
   :header: "Name", "State and Function", "Pin ID", "Usage"
   :widths: 10, 20, 15, 20

   "PD0", "IN, AF9", "CAN1_RX", "bxCAN1 Receive"
   "PD1", "OUT, AF9", "CAN1_TX", "bxCAN1 Transmit"
   "PD8", "OUT, AF7", "USART3_TX", "ST-LINK VCP TX"
   "PD9", "IN, AF7", "USART3_RX", "ST-LINK VCP RX"
   "PB0", "OUT, GPIO", "LD1_GREEN", "User LED Green"
   "PB7", "OUT, GPIO", "LD2_BLUE", "User LED Blue"
   "PB14", "OUT, GPIO", "LD3_RED", "User LED Red"
   "PC13", "IN, GPIO", "B1_USER", "User Button"

Hardware Summary
----------------

.. csv-table::
   :header: "Parameter", "Value"
   :widths: 30, 30

   "MCU", "STM32F413ZH (ARM Cortex-M4)"
   "Clock", "96 MHz (HSI with PLL)"
   "Flash", "1.5 MB"
   "SRAM", "320 KB (256 KB SRAM1 + 64 KB SRAM2)"
   "CAN", "bxCAN (CAN 2.0B)"
   "UART", "USART3 via ST-LINK VCP (115200 baud)"
   "Debug", "ST-LINK/V2-1 (SWD)"
   "Package", "LQFP144"
