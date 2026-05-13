Synchronization Interface
=========================

Overview
--------

The ``os::IFutureSupport`` class provides interface for synchronization between the main thread and the worker thread.
The ``async::FutureSupport`` class provides implementation of the ``os::IFutureSupport`` interface.
Calling ``wait()`` method results in blocking the current thread until the future is ready.
This class is used to synchronize the main thread with the worker thread and can be
compared to barriers or conditionals. The ``notify()`` function will set the bits of the future object
allowing execution to return to the main thread.

Examples
--------

.. code-block:: cpp

    #include "async/FutureSupport.h"

    // Asynchronous command executor
    class AsyncCommandExecutor : private ::async::RunnableType
    {
    public:
        AsyncCommandExecutor() : _future(0) {} // assume context ID is 0
        async::FutureSupport _future;
    }

    // synchronization of asynchronous execution
    void AsyncCommandExecutor::execWithWait
    {
        ::async::execute(context, *this); // enqueue async execution
        _future.wait(); // wait until the execution is done
    }

    void AsyncCommandExecutor::execute()
    {
        // Do something and notify when complete
        _future.notify();
    }
