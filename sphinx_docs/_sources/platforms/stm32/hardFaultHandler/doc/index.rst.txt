..
   *******************************************************************************
   Copyright (c) 2026 An Dao

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

hardFaultHandler
================

Overview
--------

The ``hardFaultHandler`` module provides a Cortex-M4 hard fault handler that saves a register
dump to a no-init RAM region. The dump survives a software reset, allowing post-mortem
analysis of the fault cause after reboot.

The following data is saved to the dump:

- CPU registers of the faulting context (R0-R12, SP, LR, PC, xPSR)
- LR and PSR at the entry of the exception handler
- stack contents (as much as possible)
- checksum of the dump

The handler is written in assembly to guarantee correct stack frame extraction from either
MSP or PSP, depending on which stack was active at the time of the fault.

Integration
-----------

The board integration must branch the HardFault vector to ``customHardFaultHandler`` and
provide ``HardFault_Handler_Final``. The dump region must be reserved by the board linker
script.
