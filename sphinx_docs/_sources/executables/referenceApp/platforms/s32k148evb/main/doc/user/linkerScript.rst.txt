.. _linkerScript:

Linker Script
=============

The file ``application_generated.dld`` contains the linker script of the executable

The linker script defines:

- Memory Layout of the microcontroller. Defines size and location of memory regions such as RAM,
  FLASH, etc.
- Sections of the program. Defines the location of the sections in the memory layout.
- Defines the location of the symbols in the memory layout.
- Entry Point Specification: It specifies the entry point of the program, which is the address
  at which execution begins.
