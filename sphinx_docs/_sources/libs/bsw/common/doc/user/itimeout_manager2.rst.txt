TimeoutManager Interface
========================

Overview
--------

The ``common::ITimeoutManager2`` class provides the interface for setting and canceling timeouts.
The ``common::AsyncTimeoutManager`` class provides the implementation of the ``common::ITimeoutManager2hh`` interface.
The ``set()`` function sets the timeout for the given object, and the ``cancel()`` function cancels the timeout.
Primarily, the ``common::AsyncTimeoutManager`` acts as a wrapper around the ``::async::scheduleAtFixedRate()``
and ``::async::schedule()`` for timers as runnables.

Examples
--------

Introduce a class that implements the ``common::AbstractTimeout`` interface:

.. code-block:: cpp

    class AbstractTimeoutImpl : public ::common::AbstractTimeout
    {
    public:
        void expired(TimeoutExpiredActions actions) override
        {
            // printf("Timeout expired\n");
        }
    };


In the client code allocate the timeout object and set the timer:

.. code-block:: cpp

    static common::AsyncTimeoutManager timeManager;
    static common::AbstractTimeoutImpl timer;

    void somefunc()
    {
        timeManager.init();
        timeManager.set(timer, 1000U, false); // set timeout for 1 second
    }
