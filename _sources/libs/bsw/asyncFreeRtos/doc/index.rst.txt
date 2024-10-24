asyncFreeRtos
=============

Overview
--------

The module implements the generic ``async`` API for FreeRTOS.

The main features of this implementation are:

* A configurable maximum number of tasks can be configured (as contexts) using simple
  initialization types that also allocate tasks stacks statically
* A context maps to a FreeRTOS task
* A context can be represented by a number (starting with 0). A higher context number
  indicates a higher prio of the corresponding task.
* The idle task has the reserved value (and priority) 0.
* The timer task is the task with the highest priority in the system.
* All other tasks priorities are between idle and timer task priority.
* All tasks can be configured by simple configuration objects that also statically
  allocate the required stacks.
* A hook concept for registering to task switch events is available
* A simple interface for synchronizing FreeRTOS with interrupts is available
* Implementations of legacy interfaces `looper` and `ITimeoutManager2` are available
  for each configured task (they can be optionally disabled)

.. toctree::
    :maxdepth: 1
    :glob:
    :hidden:

    user/adapter_api
    user/async_api
    user/scenarios/index
    user/static_config

