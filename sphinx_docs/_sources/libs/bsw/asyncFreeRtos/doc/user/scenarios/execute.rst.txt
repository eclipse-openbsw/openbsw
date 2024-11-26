execute
=======

Overview
--------

The following sequences show the expected behavior of calls to ``execute``
from different contexts.

  * Execute in higher prio context than the current context. It is expected that the
    current task is immediately pre-empted in order to execute the higher-prio Runnable.

  * Execute in same context as the current context. It is important to note that also in this
    case it is an asynchronous operation and the Runnable is expected to be
    executed after complete execution of the current Runnable.

  * Execute in lower prio context than the current context. It is expected that the lower-prio
    Runnable is first executed after all execution of higher-prio Runnables is terminated.

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
