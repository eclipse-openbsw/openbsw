.. _nucleo_g474re_main:

NUCLEO-G474RE Reference Application
====================================

Overview
--------

The ``main`` module contains the platform-specific entry point, startup code, and
lifecycle systems for the **NUCLEO-G474RE** evaluation board.

Hardware Summary
^^^^^^^^^^^^^^^^

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32G474RE — Arm Cortex-M4F, single-precision FPU"
   "Core clock", "170 MHz (HSI16 with PLL, boost mode enabled)"
   "Flash", "512 KB"
   "SRAM", "128 KB (96 KB SRAM1 + 32 KB SRAM2)"
   "CAN peripheral", "FDCAN1 (ISO 11898-1:2015 CAN 2.0B / CAN FD capable)"
   "CAN pins", "PA11 (RX) / PA12 (TX), AF9"
   "CAN bit rate", "500 kbit/s nominal"
   "Debug UART", "USART2 — PA2 TX (AF7), 115200 baud, routed to ST-LINK VCP"
   "User LED", "LD2 on PA5 (active-high)"
   "Debug interface", "On-board ST-LINK/V3 (SWD)"

Module Contents
^^^^^^^^^^^^^^^

The ``main`` module is organized into the following sub-modules:

- **main.cpp** — C entry point (``main()``), ``SystemInit()``, ``HardFault_Handler``,
  and lifecycle registration via ``platformLifecycleAdd()``.
- **Startup code** — ``Reset_Handler`` assembly: vector table, ``.data`` copy, ``.bss``
  zero-fill, FPU enable, ``__libc_init_array``, branch to ``main()``.
- **Linker script** — Memory layout for STM32G474RE (512 KB flash at 0x0800_0000,
  128 KB SRAM at 0x2000_0000), section placement, stack/heap sizing.
- **StaticBsp** — Pre-lifecycle BSP initialization (system timer, UART).
- **CanSystem** — FDCAN1 lifecycle component with ISR trampolines and frame dispatch.

The reference application boots through the openBSW lifecycle manager with the following
runlevel progression:

1. **Level 1** — ``RuntimeSystem``: OS primitives, async framework, system tick.
2. **Level 2** — ``CanSystem``: FDCAN1 peripheral initialization, NVIC, ISR enable.
3. **Level 5** — ``DemoSystem``: Application-level demonstration tasks.

Related Platform Modules
^^^^^^^^^^^^^^^^^^^^^^^^

The NUCLEO-G474RE platform includes additional configuration modules alongside
``main``:

- :ref:`bspconfig_nucleo_g474re` — UART pin mapping, baud-rate configuration,
  and standard I/O bridge for the ST-LINK VCP.

User Documentation
^^^^^^^^^^^^^^^^^^

See the user documentation for detailed information on each sub-module:

.. toctree::
   user/index
