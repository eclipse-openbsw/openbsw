hardFaultHandler
================

Overview
--------

The ``hardFaultHandler`` module provides a Cortex-M4 hard fault handler that captures all
relevant CPU registers (R0-R12, LR, PC, xPSR, CFSR, HFSR, MMFAR, BFAR) into a NO_INIT RAM
section. This preserved register dump survives a software reset, allowing post-mortem analysis
of the fault cause after reboot.

The handler is written in assembly to guarantee correct stack frame extraction from either
MSP or PSP, depending on which stack was active at the time of the fault.
