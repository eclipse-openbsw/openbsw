.. _nucleo_f413zh_main:

NUCLEO-F413ZH Reference Application
====================================

Overview
--------

The ``main`` module contains the platform-specific entry point, startup code, and
lifecycle systems for the **NUCLEO-F413ZH** evaluation board.

Hardware Summary
^^^^^^^^^^^^^^^^

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32F413ZH — Arm Cortex-M4, single-precision FPU"
   "Core clock", "96 MHz (HSI with PLL)"
   "Flash", "1.5 MB"
   "SRAM", "320 KB"
   "CAN peripheral", "bxCAN — CAN1 (ISO 11898-1 CAN 2.0A/B)"
   "CAN pins", "PD0 (RX) / PD1 (TX), AF9"
   "CAN bit rate", "500 kbit/s nominal"
   "Debug UART", "USART3 — PD8 TX (AF7), 115200 baud, routed to ST-LINK VCP"
   "User LED", "LD1 (green) on PB0 (active-high)"
   "Debug interface", "On-board ST-LINK/V2-1 (SWD)"

Module Contents
^^^^^^^^^^^^^^^

The ``main`` module is organized into the following sub-modules:

- **main.cpp** — C entry point (``main()``), ``SystemInit()``, ``HardFault_Handler``,
  diagnostic output helpers, and lifecycle registration via ``platformLifecycleAdd()``.
- **Startup code** — ``Reset_Handler`` assembly: vector table, ``.data`` copy, ``.bss``
  zero-fill, FPU enable, ``__libc_init_array``, branch to ``main()``.
- **Linker script** — Memory layout for STM32F413ZH (1.5 MB flash at 0x0800_0000,
  320 KB SRAM at 0x2000_0000), section placement, stack/heap sizing.
- **StaticBsp** — Pre-lifecycle BSP initialization (system timer, UART).
- **CanSystem** — bxCAN1 lifecycle component with ISR trampolines and frame dispatch.

The reference application boots through the openBSW lifecycle manager with the following
runlevel progression:

1. **Level 1** — ``RuntimeSystem``: OS primitives, async framework, system tick.
2. **Level 2** — ``CanSystem``: bxCAN1 peripheral initialization, NVIC, ISR enable.
3. **Level 5** — ``DemoSystem``: Application-level demonstration tasks.

.. note::

   CAN support is conditionally compiled via the ``PLATFORM_SUPPORT_CAN`` preprocessor
   define. When this define is absent, the platform boots without CAN functionality,
   which is useful for UART-only debugging configurations.

Related Platform Modules
^^^^^^^^^^^^^^^^^^^^^^^^

The NUCLEO-F413ZH platform includes additional configuration modules alongside
``main``:

- :ref:`bspconfig_nucleo_f413zh` — UART pin mapping, baud-rate configuration,
  and standard I/O bridge for the ST-LINK VCP.

User Documentation
^^^^^^^^^^^^^^^^^^

See the user documentation for detailed information on each sub-module:

.. toctree::
   user/index
