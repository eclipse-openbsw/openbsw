..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspIo - IO driver
=================

Overview
--------

This module is an IO driver that provides access to input/output functionality.
It consists of available port pins, which are represented as part of an enum.
Additionally, it contains enums for specifying whether a pin will be used as an input or output,
available interrupts, and different clock configurations. These enums can be collectively used
for ioConfigurations.