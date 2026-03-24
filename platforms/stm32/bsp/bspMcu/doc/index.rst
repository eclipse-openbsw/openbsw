bspMcu
======

Overview
--------

The ``bspMcu`` module is the MCU support package for the STM32 platform port. It
provides device headers, startup assembly, linker scripts, and the software
reset wrapper. Together these files bring the chip from power-on reset to the
point where C/C++ ``main()`` can execute.

Device Headers (``mcu/mcu.h``)
------------------------------

``mcu.h`` is the single include point for all chip-specific register
definitions. It selects the correct vendor CMSIS device header based on the
build-time preprocessor symbol:

- ``STM32F413xx`` -- includes ``stm32f413xx.h`` (STM32F413ZH, Cortex-M4F,
  1536 KB Flash, 320 KB SRAM)
- ``STM32G474xx`` -- includes ``stm32g474xx.h`` (STM32G474RE, Cortex-M4F,
  512 KB Flash, 128 KB SRAM)

If neither symbol is defined the build fails with a ``#error``.

Additional definitions in ``mcu.h``:

``INCLUDE_CORE_CM4_IN_MCU_H``
    Guard required by the OpenBSW-patched ``core_cm4.h``.

``ENABLE_INTERRUPTS()`` / ``DISABLE_INTERRUPTS()``
    Macros mapping to ``__enable_irq()`` / ``__disable_irq()`` intrinsics. Used
    by the FreeRTOS Cortex-M4 SysTick port.

``FEATURE_NVIC_PRIO_BITS``
    Set to ``4`` (16 priority levels). Matches the STM32 4-bit priority
    grouping, consistent with the S32K1xx platform port.

Vendor device headers (``stm32f413xx.h``, ``stm32g474xx.h``) and CMSIS
``core_cm4.h`` reside under the ``3rdparty/`` directory and are provided by ST
Microelectronics.

Startup Assembly
----------------

Each target has a dedicated startup file in ``startup/``:

- ``startup_stm32f413xx.s`` -- 102 interrupt vectors per RM0430 Table 40
- ``startup_stm32g474xx.s`` -- 102 interrupt vectors per RM0440 Table 97

Both files share the same structure:

``Reset_Handler``
    1. Load stack pointer from ``_estack`` (top of SRAM, defined by linker
       script).
    2. Copy ``.data`` section from Flash (``_sidata``) to SRAM
       (``_sdata`` .. ``_edata``).
    3. Zero-fill ``.bss`` section (``_sbss`` .. ``_ebss``).
    4. Call ``SystemInit`` (weak, defaults to no-op infinite loop if not
       overridden -- typically overridden by ``configurePll()`` via
       ``bspClock``).
    5. Call ``__libc_init_array`` (C++ static constructors and
       ``preinit_array`` / ``init_array``).
    6. Call ``main()``.

``Default_Handler``
    Infinite loop. All interrupt handlers are declared as weak aliases to
    ``Default_Handler`` so that unhandled interrupts trap deterministically.

``g_pfnVectors``
    The vector table placed in the ``.isr_vector`` section. Contains the
    initial stack pointer, 15 Cortex-M4 system exception entries, and all
    device-specific external interrupt entries.

Linker Scripts
--------------

Device-specific linker scripts reside in ``linker/``:

**STM32F413ZHxx_FLASH.ld**

    =========  ==============  ============
    Region     Origin          Size
    =========  ==============  ============
    FLASH      ``0x08000000``  1536 KB
    RAM        ``0x20000000``  320 KB
    =========  ==============  ============

**STM32G474RExx_FLASH.ld**

    =========  ==============  ============
    Region     Origin          Size
    =========  ==============  ============
    FLASH      ``0x08000000``  512 KB
    RAM        ``0x20000000``  128 KB
    =========  ==============  ============

Both linker scripts share the same section layout:

- ``.isr_vector`` -- interrupt vector table (Flash, ``KEEP``)
- ``.text`` -- code, ARM glue sections, ``.init`` / ``.fini`` (Flash)
- ``.rodata`` -- read-only data (Flash)
- ``.ARM.extab`` / ``.ARM`` -- C++ exception unwinding tables (Flash)
- ``.preinit_array`` / ``.init_array`` / ``.fini_array`` -- C++ constructor and
  destructor tables (Flash)
- ``.data`` -- initialised data (loaded from Flash into RAM at startup)
- ``.bss`` -- zero-initialised data (RAM)
- ``._user_heap_stack`` -- reserved space for heap (512 bytes min) and stack
  (1024 bytes min)

Entry point is ``Reset_Handler``. Stack grows downward from top of RAM
(``_estack = ORIGIN(RAM) + LENGTH(RAM)``).

Software Reset (``softwareSystemReset``)
----------------------------------------

``reset/softwareSystemReset.h`` declares a single ``extern "C"`` function:

.. code-block:: c

   void softwareSystemReset(void);

This wraps the CMSIS ``NVIC_SystemReset()`` call, which writes the
``SYSRESETREQ`` bit in the Cortex-M4 Application Interrupt and Reset Control
Register (``SCB->AIRCR``). The function does not return.

Dependencies
------------

- CMSIS ``core_cm4.h`` (ARM)
- ST vendor device headers under ``3rdparty/``
- ``mcu/typedefs.h`` for platform type aliases
