.. _bspconfig_nucleo_f413zh:

bspConfiguration - NUCLEO-F413ZH
================================

Overview
--------

The ``bspConfiguration`` module contains hardware-specific configuration for the
NUCLEO-F413ZH evaluation board. Driver logic is separated from its configuration
so that users can customise a project for different boards or pin assignments
without modifying driver source code.

The STM32F413ZH platform exposes a minimal set of BSP peripherals required for
the CAN reference application:

- **bspUart** — USART3 configuration (ST-LINK Virtual COM Port on PD8/PD9).
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

   "MCU", "STM32F413ZH (Arm Cortex-M4F, 96 MHz, single-precision FPU)"
   "UART peripheral", "USART3 — routed to ST-LINK/V2-1 Virtual COM Port"
   "UART TX pin", "PD8 (AF7)"
   "UART RX pin", "PD9 (AF7)"
   "UART baud rate", "115 200 baud (BRR = 417 at 48 MHz APB1)"
   "CAN peripheral", "CAN1 (bxCAN, configured in CanSystem, not bspConfiguration)"

.. important::

   On the STM32F413ZH the APB1 bus clock is **48 MHz** (half the 96 MHz
   system clock due to a maximum APB1 frequency of 50 MHz). The BRR value
   is calculated from the APB1 clock, not the system clock.

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
