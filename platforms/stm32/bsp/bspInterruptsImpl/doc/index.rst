bspInterruptsImpl
=================

Overview
--------

The ``bspInterruptsImpl`` module provides interrupt management for STM32
Cortex-M4 targets. It implements global interrupt disable/enable primitives and
integrates with FreeRTOS for context-safe critical sections.

Global Interrupt Disable / Enable
----------------------------------

The header ``interrupts/disableEnableAllInterrupts.h`` provides two inline
assembly functions:

``disableAllInterrupts()``
    Executes ``CPSID I`` to set the ``PRIMASK`` register, masking all
    interrupts except NMI and HardFault. Followed by ``ISB``, ``DSB``, and
    ``DMB`` barrier instructions to ensure the mask takes effect before any
    subsequent memory access or instruction fetch.

``enableAllInterrupts()``
    Executes ``ISB``, ``DSB``, and ``DMB`` barriers first, then ``CPSIE I`` to
    clear ``PRIMASK`` and re-enable interrupts. The barrier-before-enable
    ordering ensures all pending memory operations complete before interrupts
    can fire.

Both functions are declared ``static inline`` with
``__attribute__((always_inline))`` to guarantee zero-overhead inlining at every
call site.

PRIMASK vs BASEPRI
------------------

The Cortex-M4 offers two mechanisms for interrupt masking:

**PRIMASK (bare-metal variant)**
    Setting PRIMASK to 1 disables all configurable interrupts (priority levels
    0--15). Only NMI (priority -2) and HardFault (priority -1) remain active.
    This is the approach used by ``disableAllInterrupts()`` /
    ``enableAllInterrupts()`` and is suitable for bare-metal critical sections
    where no RTOS is running.

**BASEPRI (FreeRTOS variant)**
    FreeRTOS uses ``BASEPRI`` instead of ``PRIMASK`` for its critical sections.
    Setting ``BASEPRI`` to ``configMAX_SYSCALL_INTERRUPT_PRIORITY`` masks only
    interrupts at or below that priority threshold, leaving higher-priority
    (lower numeric value) interrupts unmasked. This allows time-critical ISRs
    (e.g., motor control, safety watchdog) to preempt FreeRTOS critical
    sections.

    FreeRTOS-aware critical section macros (``taskENTER_CRITICAL`` /
    ``taskEXIT_CRITICAL``) use this BASEPRI approach internally. The
    ``bspInterruptsImpl`` module provides the ``SuspendResumeAllInterruptsScopedLock``
    RAII wrapper for C++ code that needs interrupt-safe sections compatible with
    both variants.

NVIC Priority Configuration
----------------------------

The STM32 Cortex-M4 implements 4-bit priority grouping (``FEATURE_NVIC_PRIO_BITS = 4``
in ``mcu/mcu.h``), yielding 16 priority levels (0 = highest, 15 = lowest).

Priority assignment guidelines:

- Priorities 0--3: Reserved for time-critical hardware ISRs that must not be
  blocked by FreeRTOS critical sections (above
  ``configMAX_SYSCALL_INTERRUPT_PRIORITY``).
- Priorities 4--15: Available for ISRs that may call FreeRTOS API functions
  (``xSemaphoreGiveFromISR``, ``xQueueSendFromISR``, etc.).

The ``ENABLE_INTERRUPTS()`` and ``DISABLE_INTERRUPTS()`` macros in ``mcu/mcu.h``
map directly to ``__enable_irq()`` / ``__disable_irq()`` CMSIS intrinsics for
use by the FreeRTOS Cortex-M4 port layer.

Scoped Lock
-----------

The ``SuspendResumeAllInterruptsScopedLock`` class (from the ``interrupts``
namespace) provides RAII-style interrupt locking:

.. code-block:: cpp

   {
       const ESR_UNUSED interrupts::SuspendResumeAllInterruptsScopedLock lock;
       // Critical section -- interrupts disabled
   }
   // Interrupts restored to previous state

This pattern is used throughout the BSP (e.g., ``bspTimer`` tick accumulation)
to ensure interrupt state is always restored, even if an early return or
exception occurs.

Dependencies
------------

- ARM Cortex-M4 ``PRIMASK`` and ``BASEPRI`` special registers
- CMSIS ``__enable_irq()`` / ``__disable_irq()`` intrinsics
- FreeRTOS ``configMAX_SYSCALL_INTERRUPT_PRIORITY`` (when FreeRTOS is active)
