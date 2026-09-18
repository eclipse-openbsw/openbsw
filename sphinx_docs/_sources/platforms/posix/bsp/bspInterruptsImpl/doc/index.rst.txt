..
   *******************************************************************************
   Copyright (c) 2024 Accenture

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

bspInterruptsImpl
=================


Module Overview
---------------

The module ``bspInterruptsImpl`` implements the functions declared in ``bspInterrupts``.

``bspInterruptsImpl`` manages interrupt-related operations, including the suspension and resumption
of interrupts. When suspending interrupts, it halts all interrupt processes and retrieves the
previous interrupt status. Later, it resumes the previously suspended interrupts.