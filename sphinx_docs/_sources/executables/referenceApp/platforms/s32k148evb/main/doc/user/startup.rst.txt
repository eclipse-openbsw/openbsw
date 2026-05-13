.. _startUp:

Startup Code
============

The linker script contains the addresses of the interrupt handlers and the reset handler.
When a microcontroller is reset or an interrupt occurs, the microcontroller looks up the address
of the corresponding handler function in the vector table and jumps to that address to execute
the handler function.

Reset Handler
-------------
The ``Reset_Handler`` function is the entry point of program after the system resets.
This location is typically specified in the reset vector, which is part of the vector table at
the start of the system memory. The reset vector points to the Reset_Handler function.

The Reset_Handler function typically performs the following tasks:

- Initializes the system's RAM.
    - copying initial values for variables from non-volatile memory (like flash) to RAM.
- Calls system initialization functions.
    - setting up clocks
    - configuring peripherals
- Calls the main function of the program.

Order of activities in Reset_Handler function:

1. Mask interrupts
2. Disable interrupts and clear pending flags
3. clear enable IRQ registers
4. ISR relocation
5. enable fpu
6. Init the rest of the registers
7. Initialize the stack pointer
8. c code call stack init
9. call boardInit function
10. Unmask interrupts
11. Init ECC RAM
12. Loop to copy data from read only memory to RAM
13. Loop to clear BSS section
14. call global constructors
15. call main function

__libc_init_array is a function which calls the constructors in the .preinit_array and .init_array
sections.
