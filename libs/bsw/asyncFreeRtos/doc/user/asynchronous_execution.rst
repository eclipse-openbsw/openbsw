.. _async_execution:

Asynchronous execution
======================

The asynchronous execution family consists of three functions:

* ``async::execute()``
* ``async::schedule()``
* ``async::scheduleAtFixedRate()``

Each of the above function would call a corresponding non-blocking method from
``FreeRtosAdapter`` class. A more detailed description of the functions may be found in ``async`` module.

Features
--------

Execute
+++++++

The following sequences illustrate the expected behavior of calls to execute from different contexts:

* | **Execute in a higher-priority context than the current context.**
  | The current task is expected to be immediately preempted to execute the higher-priority `runnable`.

* | **Execute in the same context as the current context.**
  | Even in this case, it is an asynchronous operation. The `runnable` is expected to be
    executed only after the complete execution of the current `runnable`.

* | **Execute in a lower-priority context than the current context.**
  | The lower-priority `runnable` is expected to be executed only after the execution of all
    higher-priority `runnables` has completed.

Schedule
++++++++

The following sequences illustrate the expected behavior of calls to ``schedule`` or
``scheduleAtFixedRate`` from different contexts:

* | **Schedule in a higher-priority context.**
  | Timers of the higher-priority task are expected to be handled immediately, preempting the
    lower-priority task.

* | **Schedule in the same context.**
  | Timers of the current context are expected to be handled after the execution of the current
    `runnable`, avoiding immediate execution in this specific case.

* | **Schedule in a lower-priority context.**
  | Timer handling is expected to be triggered within the lower-priority context only after all
    higher-priority handling has been completed.

Related types
+++++++++++++

* ``async::FreeRtosAdapter``
* ``async::TimeoutType``

Examples
++++++++

Define a class that implements the ``IRunnable`` interface:

.. code-block:: cpp

    #include <async/Async.h>

    class RunnableImpl : public ::async::IRunnable
    {
    public:
        void execute() override
        {
            fprintf(stdout, "Executing TestRunnable!\n");
        }
    };

In client code allocate the runnable object and asynchronously execute it:

.. code-block:: cpp

    static RunnableImpl runnable;
    static ::async::TimeoutType timeout;

    void executeRunnable(::async::ContextType& context)
    {
        // asynchronously execute the runnable:
        ::async::execute(context, runnable);

        // schedule the runnable for single-time execution after 1 second from now:
        ::async::schedule(context, runnable, timeout, 1, ::async::TimeUnit::SECONDS);

        // schedule the runnable for cyclic execution with period 2 seconds:
        ::async::scheduleAtFixedRate(context, runnable, timeout, 2, ::async::TimeUnit::SECONDS);
    }
