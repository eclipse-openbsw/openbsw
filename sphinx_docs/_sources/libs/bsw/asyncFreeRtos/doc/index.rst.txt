asyncFreeRtos
=============

This module provides a collection of classes that serve as an intermediate layer
between the asynchronous operations API and their implementation in **FreeRTOS**.
The supported asynchronous operations include:

* Non-blocking immediate execution
* Non-blocking scheduling of single-time execution
* Non-blocking scheduling of cyclic execution
* Thread synchronization using ``wait()`` and ``notify()`` mechanisms
* Secure sections with ``Lock``

The implementation of these asynchronous operations is achieved by wrapping native
**FreeRTOS** functions into classes like ``async::TaskContext`` and ``async::FutureSupport``.
Additional classes, such as ``async::FreeRtosAdapter`` and ``async::StaticRunnable``,
offer a public API for initializing tasks that can later be executed asynchronously.

The main features of this implementation are:

* A single ``async::Task`` instance corresponds to a single **FreeRTOS** task (via TaskHandle_t, a pointer to the **FreeRTOS** structure).
* The ``async::Task`` context is represented by an integer ID. A higher context ID indicates a higher priority for the corresponding task.
* The idle task has a reserved context ID and priority of 0.
* The timer task is assigned the highest priority in the system.
* All other task priorities are assigned values between the idle and timer task priorities.
* Task configuration specifies both the context ID and the required stack size.
* A Runnable is executed within the context of one of the initialized instances of ``async::Task``.
* A hook mechanism is available for registering task switch events.
* An interface is provided for synchronizing **FreeRTOS** with interrupts.

.. toctree::
    :maxdepth: 1
    :glob:

    */index

