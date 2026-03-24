.. _nucleo_g474re_overview:

NUCLEO-G474RE (STM32G474RE)
============================

Overview
--------

- The NUCLEO-G474RE is a development board from STMicroelectronics featuring the STM32G474RE
  microcontroller (ARM Cortex-M4F, 170 MHz, 512 KB Flash, 128 KB SRAM) in an LQFP64 package.
- It includes an on-board ST-LINK/V3E debugger/programmer, Arduino Uno V3 and ST morpho
  connectors, and requires no separate probe for development.
- Eclipse OpenBSW provides a reference application for this platform demonstrating UART console
  output and FDCAN communication, bringing CAN FD support to the OpenBSW ecosystem on affordable
  STM32 hardware.

Reference Links
---------------

1. **NUCLEO-G474RE Product Page**:
    - `NUCLEO-G474RE <https://www.st.com/en/evaluation-tools/nucleo-g474re.html>`_

2. **NUCLEO-G474RE Documentation**:
    - `User Manual: UM2505 STM32G4 Nucleo-64 boards <https://www.st.com/resource/en/user_manual/um2505-stm32g4-nucleo64-boards-mb1367-stmicroelectronics.pdf>`_
    - `Schematic <https://www.st.com/resource/en/schematic_pack/mb1367-g474re-c04_schematic.pdf>`_

3. **STM32G474RE MCU Documentation**:
    - `Reference Manual: RM0440 <https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`_
    - `Data Sheet: DS12288 <https://www.st.com/resource/en/datasheet/stm32g474re.pdf>`_

4. **Software and Tools**:
    - `STM32CubeProgrammer <https://www.st.com/en/development-tools/stm32cubeprog.html>`_
    - `ARM GNU Toolchain <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`_

Build environment
-----------------
For instructions on building for this platform see :ref:`learning_setup`

Quick build::

    cmake --preset nucleo-g474re-freertos-gcc
    cmake --build --preset nucleo-g474re-freertos-gcc

Flash using STM32CubeProgrammer::

    STM32_Programmer_CLI -c port=SWD -d build/nucleo-g474re-freertos-gcc/RelWithDebInfo/referenceApp.elf -v -rst

Connections
-----------

.. csv-table:: Pin Configuration
   :header: "Name", "State and Function", "Pin ID", "Usage"
   :widths: 10, 20, 15, 20

   "PA11", "IN, AF9", "FDCAN1_RX", "FDCAN1 Receive"
   "PA12", "OUT, AF9", "FDCAN1_TX", "FDCAN1 Transmit"
   "PA2", "OUT, AF7", "USART2_TX", "ST-LINK VCP TX"
   "PA3", "IN, AF7", "USART2_RX", "ST-LINK VCP RX"
   "PA5", "OUT, GPIO", "LD2_GREEN", "User LED Green"
   "PC13", "IN, GPIO", "B1_USER", "User Button"

Hardware Summary
----------------

.. csv-table::
   :header: "Parameter", "Value"
   :widths: 30, 30

   "MCU", "STM32G474RE (ARM Cortex-M4F)"
   "Clock", "170 MHz (HSI16 with PLL boost)"
   "Flash", "512 KB"
   "SRAM", "128 KB (96 KB SRAM1 + 32 KB SRAM2 + 32 KB CCM)"
   "CAN", "FDCAN (CAN FD + CAN 2.0B)"
   "UART", "USART2 via ST-LINK VCP (115200 baud)"
   "Debug", "ST-LINK/V3E (SWD)"
   "Package", "LQFP64"
