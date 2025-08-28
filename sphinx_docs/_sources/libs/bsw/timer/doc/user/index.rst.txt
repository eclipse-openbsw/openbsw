User Documentation
==================

The ``Timer`` essentially acts as a container (linked list) of timeouts, selecting due timeouts for execution.
The logic for selecting a due timeout from the ``Timer`` container is application-dependent.
In the simplest implementation, the ``Timer`` object could be polled in a busy-wait loop, executing due timeouts as they arise.
However, this is not the most efficient way to utilize the timer module, as a busy-wait loop consumes CPU time and blocks the thread.
In the ``asyncFreeRtos`` implementation, by contrast, the ``Timer`` object is polled when the FreeRTOS timer interrupt is triggered.
Below an example code for setting cyclic, single shot timeouts and for handling timer loop is presented.

Examples
--------

Introduce a runnable class to be executed on timeout. The class inherits from the ``Runnable`` interface and implements the ``execute()`` method.

.. code-block:: cpp

    class RunnableImpl : public Runnable
    {
    public:
        void execute() override
        {
            // Do something
        }
    };

Client code for setting timeouts.

.. code-block:: cpp

    // Assume the context IDs are defined in enumerator:
    enum
    {
        TASK_BSP,
        TASK_UDS,
        TASK_DEMO
    };

    // Allocate timer, timeout and runnable:
    timer::Timer<Lock> timer;
    async::TimeoutType timeout;
    RunnableImpl runnable;
    // Assign runnable to timeout:
    timeout._runnable = &runnable;
    timeout._context = TASK_DEMO;

    // set one shot timeout to execute in 1 second from now:
    timer.set(timeout, 1000000, getSystemTimeUs32Bit());

    // set cyclic timeout with 2 seconds period:
    timer.setCyclic(timeout, 2000000, getSystemTimeUs32Bit());

    // cancel timeout:
    timer.cancel(timeout);
