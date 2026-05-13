User documentation
==================

Overview
--------

``bspInterrupts`` provides lock objects to suspend and resume all interrupts.
There are 2 types of locks provided: RAII and non-RAII.
The ECU specific implementation of interrupt locks are done in the module ``bspInterruptsImpl``.

Public API
----------

Class ``SuspendResumeAllInterruptsLock`` provides non-RAII lock object for suspending/resuming interrupts.

.. sourceinclude:: include/interrupts/SuspendResumeAllInterruptsLock.h
    :start-after: PUBLICAPI_START
    :end-before: PUBLICAPI_END
    :language: c++
    :dedent: 0

Class ``SuspendResumeAllInterruptsScopedLock`` provides RAII lock object for suspending/resuming interrupts.

.. sourceinclude:: include/interrupts/SuspendResumeAllInterruptsScopedLock.h
    :start-after: PUBLICAPI_START
    :end-before: PUBLICAPI_END
    :language: c++
    :dedent: 0


Code generation
---------------

Not applicable

Configuration
-------------

Not applicable

Calibration
-----------

Not applicable

Usage Examples and Integration
------------------------------

It should be synced together with a specific implementation of ``bspInterruptsImpl`` module.