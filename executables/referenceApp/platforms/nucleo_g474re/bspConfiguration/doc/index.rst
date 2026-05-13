.. _bspconfig_nucleo_g474re:

bspConfiguration - NUCLEO-G474RE
================================

Overview
--------

The ``bspConfiguration`` module contains hardware-specific configuration for the
NUCLEO-G474RE evaluation board. Driver logic is separated from its configuration
so that users can customise a project for different boards or pin assignments
without modifying driver source code.

The STM32G474RE platform exposes a minimal set of BSP peripherals required for
the CAN reference application:

- **bspUart** — USART2 configuration (ST-LINK Virtual COM Port on PA2/PA3).
- **bspStdIo** — Standard I/O bridge that routes ``putByteToStdout`` /
  ``getByteFromStdin`` through the configured UART instance.

.. note::

   Unlike the S32K148EVB reference application, this platform does not include
   ADC, PWM, or digital I/O configuration modules because the CAN reference
   application does not require analogue inputs or GPIO-driven outputs.
   These modules can be added following the same configuration pattern if
   a future application requires them.

Hardware Summary
^^^^^^^^^^^^^^^^

.. csv-table::
   :widths: 30, 70
   :width: 100%

   "MCU", "STM32G474RE (Arm Cortex-M4F, 170 MHz, single-precision FPU)"
   "UART peripheral", "USART2 — routed to ST-LINK/V3 Virtual COM Port"
   "UART TX pin", "PA2 (AF7)"
   "UART RX pin", "PA3 (AF7)"
   "UART baud rate", "115 200 baud (BRR = 1476 at 170 MHz APB1)"
   "CAN peripheral", "FDCAN1 (configured in CanSystem, not bspConfiguration)"

Build Integration
^^^^^^^^^^^^^^^^^

The CMake target ``bspConfiguration`` is a static library that compiles:

- ``src/bsp/stdIo/stdIo.cpp``
- ``src/bsp/uart/UartConfig.cpp``

It publicly exposes its ``include/`` directory and links against ``bspUart``,
``bspMcu``, and ``platform``.

.. toctree::
   :hidden:

   user/index
