Future support
==============

Overview
--------

The following sequences shows how futures (which are deprecated) are
implemented in order to provide blocking behaviour for a given task.

 * Synchronous wait on event set by higher prio context. It is expected that the
   waiting task is suspended (using a ``FutureSupport`` object) and giving all
   other tasks the chance to process events. Here the context that signals the
   event is of higher-prio therefore the waiting task will only be executed
   after the higher-prio runnable has terminated its work.

 * Synchronous wait on event set by lower prio context. It is expected that the
   waiting task is suspended (using a ``FutureSupport`` object) and giving all
   other tasks the chance to process events. Here the context that signals the
   event is of lower-prio therefore it will be pre-empted on setting the signal
   and continues after the higher-prio task has finished its work.

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

Should be synced together with a specific implementation ``asyncFreeRtos`` module.