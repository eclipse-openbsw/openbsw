.. _tasks_startup:

Startup Sequence
================

1. **Initialization**

   Application initializes the ``async::internal::Task`` task objects, the bare minimum are:

   - The idle task
   - The timer task

  required by **FreeRTOS**. However, typically more tasks will be needed in the application,
  e.g. communication (CAN, UDS, Ethernet), monitoring, SWC scheduler,
  as well as lifecycle task managing desired order of modules start up and shut down.

2. **Running the Adapter**

  Before performing a first call to one of the `async` functions the application needs to
  start the **FreeRTOS** adapter by calling ``async::FreeRtosAdapter::run()``. All required
  tasks are created and started by this call. The adapter will then start the **FreeRTOS** scheduler.
  As soon as the scheduler is running the adapter performs a call to the application-defined
  startApp function that was handed over on the call to ``async::FreeRtosAdapter::run()``.

  The `runnables` are executed according to their priority and designated time,
  whether immediate or scheduled.
  Refer to :ref:`async_execution` for more information about task priorities.

  If `runnables` are being executed or waiting for execution (scheduled `runnables`),
  the application can use the idle task to perform low-priority routines,
  such as monitoring console input/output.

Related types
-------------

* ``async::FreeRtosAdapter``
* ``async::TaskInitializer``
* ``async::StaticRunnable``

Examples
--------

Typical routine for creating, initializing, and starting a task:

.. code-block:: cpp

    using AsyncAdapter = ::async::AsyncBinding::AdapterType;

    // assume the enumerator is defining the available tasks of RTOS:
    enum
    {
        TASK_BSP,
        TASK_UDS,
        TASK_DEMO
    };

    // make decision about the stack size for the task:
    const uint32_t STACK_SIZE = 1024;

    AsyncAdapter::Task<AsyncAdapter, TASK_DEMO, STACK_SIZE> demoTask{"demo"};

    void startApp()
    {
        // application-defined callback that performs first calls to async functions
    }

    ...

    AsyncAdapter::run(AsyncAdapter::StartAppFunctionType::create<&startApp>());