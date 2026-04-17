.. _nucleo_g474re_startup:

Startup Code
============

The startup code for the NUCLEO-G474RE platform provides the vector table, the
``Reset_Handler`` entry point, and the low-level initialization sequence that executes
before any C/C++ code runs.

Vector Table
------------

The vector table resides at the base of flash (0x0800_0000) and is placed by the linker
script into the ``.isr_vector`` section. It contains:

- **Initial stack pointer** — loaded from the first word of the table into SP on reset.
- **Reset vector** — address of ``Reset_Handler``, loaded into PC on reset.
- **Exception handlers** — NMI, HardFault, MemManage, BusFault, UsageFault, SVCall,
  PendSV, SysTick.
- **IRQ handlers** — device-specific interrupt vectors (FDCAN1_IT0, FDCAN1_IT1,
  USART2, etc.).

Unused vectors point to a default handler that enters an infinite loop, ensuring
spurious interrupts are trapped rather than causing undefined behavior.

Reset Handler
-------------

The ``Reset_Handler`` function is the first code to execute after a system reset. The
processor loads the initial stack pointer from the vector table and branches to
``Reset_Handler``. The handler performs the following sequence:

1. **Disable interrupts** — ``CPSID I`` masks all configurable interrupts to prevent
   premature ISR entry during initialization.

2. **FPU enable** — The Cortex-M4F single-precision FPU is enabled by setting CP10 and
   CP11 to full access in ``SCB->CPACR``:

   .. code-block:: c

      SCB->CPACR |= (0xF << 20);  /* CP10 and CP11 = Full Access */
      __DSB();
      __ISB();

   The DSB/ISB barrier instructions ensure the FPU is accessible before any subsequent
   floating-point instruction executes. Without this step, any FP operation would
   trigger a UsageFault (NOCP).

3. **Copy .data section** — Initialized global and static variables are copied from
   their load address in flash (``_sidata``) to their runtime address in SRAM
   (``_sdata`` to ``_edata``).

4. **Zero .bss section** — Uninitialized global and static variables (``_sbss`` to
   ``_ebss``) are filled with zero.

5. **Call __libc_init_array** — Invokes constructors registered in ``.preinit_array``
   and ``.init_array`` sections. This executes all C++ global/static object constructors
   and any ``__attribute__((constructor))`` functions.

6. **Call SystemInit** — Invokes ``SystemInit()`` (defined in ``main.cpp``) to configure
   the PLL to 170 MHz, enable the diagnostic LED, and set up early USART2 output.

7. **Call main** — Branches to ``main()`` with interrupts still masked. Interrupts are
   unmasked later by ``setupApplicationsIsr()`` after the async framework is ready.

.. note::

   The exact ordering of steps 5 and 6 may vary depending on the toolchain startup
   file. The STM32CubeG4 default startup calls ``SystemInit()`` before
   ``__libc_init_array``, while some toolchains reverse this order. The reference
   application relies on ``SystemInit()`` being called before ``main()`` but does not
   depend on its position relative to global constructors.

Memory Layout
-------------

The linker script for STM32G474RE defines the following memory regions:

.. csv-table::
   :header: "Region", "Start", "Size", "Description"
   :widths: 15, 20, 15, 50
   :width: 100%

   "FLASH", "0x0800_0000", "512 KB", "Program code, read-only data, vector table"
   "SRAM1", "0x2000_0000", "96 KB", "Main SRAM — stack, heap, .data, .bss"
   "SRAM2", "0x2001_8000", "32 KB", "Secondary SRAM — available for DMA buffers"
   "CCM", "0x1000_0000", "32 KB", "Core-coupled memory — zero wait-state, no DMA"

The stack is placed at the top of SRAM1 and grows downward. The heap (if used) grows
upward from the end of .bss.
