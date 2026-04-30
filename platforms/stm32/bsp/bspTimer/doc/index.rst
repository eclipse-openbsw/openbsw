bspTimer
========

Overview
--------

The ``bspTimer`` module implements the system timer for STM32 Cortex-M4 targets.
It uses the ARM Data Watchpoint and Trace (DWT) cycle counter -- a core debug
unit register available on all Cortex-M4 devices -- rather than a chip-specific
peripheral timer. This makes the implementation portable across STM32 families.

DWT Cycle Counter
-----------------

The DWT ``CYCCNT`` register is a free-running 32-bit counter that increments at
the core clock frequency (HCLK). The module reads three memory-mapped registers
directly:

``DWT_CTRL`` (``0xE0001000``)
    Control register. Bit 0 (``CYCCNTENA``) enables the cycle counter.

``DWT_CYCCNT`` (``0xE0001004``)
    Current cycle count value (32-bit, wraps at 2^32).

``DEMCR`` (``0xE000EDFC``)
    Debug Exception and Monitor Control Register. Bit 24 (``TRCENA``) must be
    set to enable DWT and ITM access.

Initialisation (``initSystemTimer``)
------------------------------------

Called once during BSP bring-up with interrupts locked:

1. Set ``TRCENA`` in ``DEMCR`` to enable the trace unit.
2. Reset ``DWT_CYCCNT`` to zero.
3. Set ``CYCCNTENA`` in ``DWT_CTRL`` to start counting.
4. Clear the internal tick accumulator and last-DWT snapshot.

Tick Accumulation
-----------------

The internal ``updateTicks()`` function extends the 32-bit ``CYCCNT`` to a
64-bit monotonic tick counter. Each tick represents 1 microsecond:

.. code-block:: text

   elapsed_cycles = CYCCNT_now - CYCCNT_last
   elapsed_us     = elapsed_cycles / DWT_FREQ_MHZ

Where ``DWT_FREQ_MHZ`` is:

- **96** for STM32F4 (96 MHz HCLK)
- **170** for STM32G4 (170 MHz HCLK)

The family is selected at compile time via ``STM32_FAMILY_F4`` or
``STM32_FAMILY_G4``. ``updateTicks()`` runs under
``SuspendResumeAllInterruptsScopedLock`` to prevent CYCCNT from advancing
between the read and the delta calculation.

Public API
----------

All functions are declared ``extern "C"`` for use from both C and C++ code:

``uint64_t getSystemTicks(void)``
    Returns the 64-bit monotonic tick count (1 tick = 1 us).

``uint32_t getSystemTicks32Bit(void)``
    Lower 32 bits of the tick count. Wraps after ~71.6 minutes.

``uint64_t getSystemTimeNs(void)``
    System time in nanoseconds (``ticks * 1000 / TICK_FREQ_MHZ``).

``uint64_t getSystemTimeUs(void)``
    System time in microseconds.

``uint32_t getSystemTimeUs32Bit(void)``
    Lower 32 bits of microsecond time.

``uint64_t getSystemTimeMs(void)``
    System time in milliseconds.

``uint32_t getSystemTimeMs32Bit(void)``
    Lower 32 bits of millisecond time.

``uint64_t systemTicksToTimeUs(uint64_t ticks)``
    Convert tick count to microseconds.

``uint64_t systemTicksToTimeNs(uint64_t ticks)``
    Convert tick count to nanoseconds.

``uint32_t getFastTicks(void)``
    Raw ``DWT_CYCCNT`` value (no accumulation). Useful for sub-microsecond
    profiling.

``uint32_t getFastTicksPerSecond(void)``
    Returns ``DWT_FREQ_MHZ * 1000000`` (clock frequency in Hz).

``void sysDelayUs(uint32_t delay)``
    Busy-wait delay for the specified number of microseconds. Uses
    ``getSystemTimeUs()`` polling -- not interrupt-driven.

FreeRTOS Integration
--------------------

The DWT timer is independent of the SysTick peripheral. FreeRTOS uses SysTick
for its own tick interrupt (``configTICK_RATE_HZ``), while the BSP system timer
uses DWT for high-resolution timestamps. Both can coexist without conflict.

Dependencies
------------

- ARM DWT and CoreDebug registers (ARMv7-M architecture)
- ``mcu/mcu.h`` for ``STM32_FAMILY_F4`` / ``STM32_FAMILY_G4`` defines
- ``interrupts/SuspendResumeAllInterruptsScopedLock.h`` for atomic tick updates
