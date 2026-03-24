.. _stm32_overview:

STM32 Platforms
===============

Overview
--------

- Eclipse OpenBSW supports two STM32 Nucleo development boards as a third platform alongside
  POSIX and S32K148EVB, bringing the stack to affordable, widely available ARM Cortex-M4 hardware
  from STMicroelectronics.
- All BSP drivers are register-level implementations with no vendor HAL dependency, following the
  same design philosophy as the S32K1xx platform.
- Two CAN controller families are supported: bxCAN (CAN 2.0B) on the F4 series and FDCAN
  (CAN FD) on the G4 series.

Supported Boards
----------------

.. csv-table::
   :header: "Board", "MCU", "Core", "Clock", "Flash / RAM", "CAN"
   :widths: 20, 20, 15, 10, 20, 15

   "NUCLEO-F413ZH", "STM32F413ZH", "Cortex-M4", "96 MHz", "1.5 MB / 320 KB", "bxCAN (CAN 2.0B)"
   "NUCLEO-G474RE", "STM32G474RE", "Cortex-M4F", "170 MHz", "512 KB / 128 KB", "FDCAN (CAN FD)"

For detailed pin configuration and board-specific information see:

- :ref:`nucleo_f413zh_overview`
- :ref:`nucleo_g474re_overview`

.. toctree::
   :maxdepth: 1
   :hidden:

   ../../../../doc/platforms/nucleo_f413zh/index
   ../../../../doc/platforms/nucleo_g474re/index

BSP Modules
-----------

.. csv-table::
   :header: "Module", "Description"
   :widths: 25, 50

   "bspCan", "Low-level CAN register access shared between bxCAN and FDCAN drivers"
   "bspClock", "PLL configuration for both chips (HSI/HSE to target frequency)"
   "bspInterruptsImpl", "Global interrupt masking (CPSID/CPSIE)"
   "bspMcu", "CMSIS headers, linker scripts, startup assembly, NVIC macros"
   "bspTimer", "DWT CYCCNT-based system timer (architecture-level, chip-agnostic)"
   "bspUart", "Polling UART TX/RX with dual register set support (F4 SR/DR vs G4 ISR/TDR)"
   "bxCanTransceiver", "OpenBSW ``AbstractCANTransceiver`` wrapper for bxCAN (F4)"
   "fdCanTransceiver", "OpenBSW ``AbstractCANTransceiver`` wrapper for FDCAN (G4)"

Reference Links
---------------

1. **STM32F413ZH**:
    - `Reference Manual: RM0430 <https://www.st.com/resource/en/reference_manual/rm0430-stm32f413423-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`_
    - `Data Sheet: DS11581 <https://www.st.com/resource/en/datasheet/stm32f413zh.pdf>`_

2. **STM32G474RE**:
    - `Reference Manual: RM0440 <https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf>`_
    - `Data Sheet: DS12288 <https://www.st.com/resource/en/datasheet/stm32g474re.pdf>`_

3. **Software and Tools**:
    - `STM32CubeProgrammer <https://www.st.com/en/development-tools/stm32cubeprog.html>`_
    - `ARM GNU Toolchain <https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>`_

Build environment
-----------------
For instructions on building for this platform see :ref:`learning_setup`

Validation
----------

- 46 unit tests pass (22 BxCanTransceiver + 23 FdCanTransceiver + 1 UART)
- CAN loopback self-test verified on both NUCLEO-G474RE (FDCAN) and NUCLEO-F413ZH (bxCAN)
- Validated against upstream OpenBSW with MinGW GCC 15.2.0
