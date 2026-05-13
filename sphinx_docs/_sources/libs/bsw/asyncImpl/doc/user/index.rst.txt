User documentation
==================

Overview
--------
The ``asyncImpl`` module defines generic event-driven mechanisms. It is currently used in the ``asyncFreeRtos`` implementation of the ``async`` module.
It implements ``Runnable`` aspect of ``async``. Refer to ``asyncFreeRtos/include/async/TaskContext.h`` in ``asyncFreeRtos`` for usage examples.

Module consists of classes:
 - ``async::EventDispatcher``
 - ``async::EventPolicy``
 - ``async::RunnableExecutor``
 - ``async::IRunnable``
 - ``async::Queue``

EventDispatcher
+++++++++++++++

The ``async::EventDispatcher`` allows registering or deregistering event handlers using the ``async::EventDispatcher::setEventHandler()`` and ``async::EventDispatcher::removeEventHandler()``.
The ``async::EventDispatcher::handleEvents()`` executes handlers based on the provided event bit mask.
The ``async::TaskContext`` from ``asyncFreeRtos`` module is ``async::EventDispatcher``.

EventPolicy
+++++++++++

``async::EventPolicy`` is a wrapper around ``async::EventDispatcher``, responsible for managing a specific event.
The main difference between ``async::EventPolicy`` and ``async::EventDispatcher`` lies in the method ``async::EventPolicy::setEvent()``. This method takes no arguments
and sets the event bit mask to include the event bit that ``async::EventPolicy`` is responsible for.

RunnableExecutor
++++++++++++++++

The ``async::RunnableExecutor`` implements the ``async::execute()`` from ``async`` module in ``asyncFreeRtos``.
When the ``async::TaskContext::execute()`` is called, the ``async::RunnableExecutor`` places the **Runnable** in the queue and sets the event it is responsible for.
Once the ``async::EventDispatcher::handleEvents()`` (from ``async::TaskContext``) is called, the ``async::EventDispatcher`` invokes the ``async::RunnableExecutor`` handler, which executes all enqueued **Runnables**.

IRunnable
+++++++++

``async::IRunnable`` is a simple interface with a single method: ``async::IRunnable::execute()``. For simplicity, both the interface and its implementations will be referred to as `runnables`.

Queue
+++++

The ``async::Queue`` is an implementation of simple queue, used in ``async::RunnableExecutor`` to hold `runnable` objects.

How ``asyncImpl`` is used in `async::TaskContext`
-------------------------------------------------

The ``async::asyncImpl`` classes are used in ``async/TaskContext.h`` of the ``asyncFreeRtos`` module.
``async::TaskContext`` acts as a wrapper for a **FreeRTOS** task and ``async::EventDispatcher`` that handles three events: timer, runnable, and stop events.
The main function of the underlying **FreeRTOS** task is implemented in ``async::TaskContext::dispatch()``.
This function blocks itself using ``FreeRTOS::xTaskNotifyWait()``, waiting for ``FreeRTOS::xTaskNotify()`` to be triggered via ``async::TaskContext::setEvents()`` with an event bit mask (refer to the **FreeRTOS** documentation for details).
Once unblocked, ``async::EventDispatcher::handleEvents()`` (from ``async::TaskContext``) is called, where the ``async::EventDispatcher`` invokes handlers for all active events.
When ``async::TaskContext::setEvents()`` is called with a bit mask of 1 << 2, corresponding to the `stop` event, the execution of ``async::TaskContext::dispatch()`` is terminated.
Correspondingly, bit mask of 1 << 0 sets the timer event and 1 << 1 leads to execution of enqueued **Runnables**.


Example
-------

The following example shows the sample class with event-driven mechanisms of ``asyncImpl``.
It could be used as a template for implementation of the ``async::TaskContext`` for the new platforms.

To port ``async`` to a new platform, one must start by providing an adequate ``Types.h``, mapping the key definitions of the ``async`` to the target platform:

.. sourceinclude:: examples/include/async/Types.h
   :start-after: EXAMPLE_BEGIN AsyncImplExample
   :end-before: EXAMPLE_END AsyncImplExample

Header of the sample class that handles 3 events: A, B and runnable:

.. sourceinclude:: examples/include/example.h
   :start-after: EXAMPLE_BEGIN AsyncImplExample
   :end-before: EXAMPLE_END AsyncImplExample

The key points of this implementation are methods ``AsyncImplExample::setEvents()`` and ``AsyncImplExample::dispatch()``.
In real life scenario, the logic to wakeup a task/context, which calls ``AsyncImplExample::dispatch()`` could be placed
inside ``AsyncImplExample::setEvents()`` and ``AsyncImplExample::dispatch()`` in the main body of task/context:

.. sourceinclude:: examples/src/example.cpp
   :start-after: EXAMPLE_BEGIN AsyncImplExample
   :end-before: EXAMPLE_END AsyncImplExample

This example prints following:

.. code-block:: bash

    AsyncImplExample::setEventA() is called.
    AsyncImplExample::setEvents() is called with eventMask:0b0000001 new eventMask:0b0000001
    AsyncImplExample::setEventB() is called.
    AsyncImplExample::setEvents() is called with eventMask:0b0000010 new eventMask:0b0000011
    AsyncImplExample::execute() called, Runnable is prepared for execution
    AsyncImplExample::setEvents() is called with eventMask:0b0000100 new eventMask:0b0000111
    AsyncImplExample::execute() called, Runnable is prepared for execution
    AsyncImplExample::setEvents() is called with eventMask:0b0000100 new eventMask:0b0000111
    AsyncImplExample::dispatch() is called, eventMask:0b0000111
    AsyncImplExample::handlerEventA() is called.
    AsyncImplExample::handlerEventB() is called.
    exampleRunnableA is called.
    exampleRunnableB is called.
    AsyncImplExample::dispatch() reset eventMask, eventMask:0b0000000
