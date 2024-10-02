schedule/scheduleAtFixedRate
============================

Overview
--------

The following sequences show the expected behaviour of calls to ``schedule``
or ``scheduleAtFixedRate`` from different contexts.

 * Schedule in higher prio context. It is expected that timers of the higher-prio task are
   handled immediately by pre-empting the lower-prio task.

 * Schedule in same context. It is expected that timers of the context are handled after
   the current ``Runnable``, avoiding immediate execution in this special case.

 * Schedule in lower prio context. It is expected that the timer handling is only triggered
   within the lower-prio context once after all higher-prio handling is terminated.

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