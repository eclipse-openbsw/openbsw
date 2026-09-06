..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

.. _nucleo_f413zh_startup:

Startup Code
============

The startup code (``platforms/stm32/bsp/bspMcu/startup/startup_stm32f413xx.s``) provides
the vector table and the ``Reset_Handler`` for the STM32F413xx.

Vector Table
------------

The vector table is placed at the base of flash (``0x0800_0000``) in the
``.isr_vector`` section. It contains the initial stack pointer, the Cortex-M4 exception
vectors, and the 102 STM32F413xx interrupt vectors (RM0430). Every handler is a weak
alias of ``Default_Handler`` (an infinite loop) and is overridden by a strong definition
where the application implements one (e.g. ``HardFault_Handler``,
``CAN1_RX0_IRQHandler``).

Reset Handler
-------------

``Reset_Handler`` executes the following sequence:

1. Load the initial stack pointer (``_estack``).
2. Copy the ``.data`` section from flash to SRAM.
3. Zero-fill the ``.bss`` section.
4. Enable the FPU (CP10/CP11 full access in ``CPACR``, followed by ``dsb``/``isb``).
   This is required for ThreadX; the FreeRTOS port enables the FPU itself.
5. Call ``SystemInit()`` (PLL and early peripheral setup, defined in ``main.cpp``).
6. Call ``__libc_init_array`` (C++ global/static constructors).
7. Call ``main()``.

The startup code does not mask or unmask interrupts; interrupt control is left to the
RTOS port once the scheduler starts.

Memory Layout
-------------

The linker script places code in 1536 KB flash at ``0x0800_0000`` and data in the
320 KB of contiguous SRAM at ``0x2000_0000`` (SRAM1 256 KB + SRAM2 64 KB). The top
1 KB of SRAM (``0x2004_FC00``-``0x2005_0000``) is excluded from the ``RAM`` region and
reserved for the hard fault handler dump, so it survives reset. The stack grows down
from ``_estack`` at ``0x2004_FC00``.
