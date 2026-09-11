..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspIo Driver
============

Overview
--------

The ``bspIo`` module provides a register-level GPIO driver for STM32 targets.
The driver is the stateless ``bios::Gpio`` class, which wraps the GPIO
peripheral registers (``MODER``, ``OTYPER``, ``OSPEEDR``, ``PUPDR``, ``AFR``,
``IDR``, ``BSRR``, ``ODR``) behind typed accessors.

A pin is described by a ``GpioConfig`` struct: port, pin number, mode
(input / output / alternate / analog), output type, speed, pull resistor,
and alternate function number. ``Gpio::configure()`` applies a complete
``GpioConfig`` in one call and enables the port clock via
``Gpio::enablePortClock()``. Individual setters
(``setMode``, ``setOutputType``, ``setSpeed``, ``setPull``,
``setAlternateFunction``) and the pin accessors (``readPin``, ``writePin``,
``togglePin``) are available for dynamic use.

Pin assignments are a board decision and are supplied by the board
configuration, not by this module.
