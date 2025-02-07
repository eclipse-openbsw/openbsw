.. _synch_mechanisms:

Synchronization mechanisms
==========================

The synchronization mechanisms allow synchronization of threads and ensure secure sections.

Related classes

* ``async::Lock``
* ``async::ModifiableLock``
* ``async::FutureSupport``

Examples
--------

Lock
++++

The ``async::Lock`` class implements the `RAII` idiom, meaning that declaring a ``async::Lock`` object in
the code blocks other threads until execution exits the scope of the ``async::Lock`` object. In the
example below, a global **counter** is presumably being incremented by two or more threads. To prevent
simultaneous modification of the counter, the `lock` object ensures that the increment operation is
performed by only one thread at a time, while others are paused and wait.

.. code-block:: cpp

    uint32_t counter = 0; // global (shared) resource

    void callback()
    {
        // .. some code here

        { // visibility scope of the lock starts here
            ::async::Lock const lock;
            ++counter;
        } // visibility scope of the lock ends here

        // .. some code here
    }

ModifiableLock
++++++++++++++

``async::ModifiableLock`` provides a secure section similar to ``async::Lock`` and is also an `RAII` object.
However, it additionally allows the client to unlock it on demand using the unlock function.

.. code-block:: cpp

    uint32_t counter = 0; // global (shared) resource

    void callback()
    {
        // .. some code here

        ::async::ModifiableLock const lock;
        ++counter;
        // .. some code we want to be inside the secure section

        lock.unlock();

        // .. some code outside the secure section
    }

FutureSupport
+++++++++++++

The ``async::FutureSupport`` class implements the ``async::IFutureSupport`` interface.
Calling the ``async::FutureSupport::wait()`` method pauses the current thread until the `future` object is ready.
This class synchronizes the main thread with the worker thread and can be compared to `barriers` or
`condition variables`. The ``async::FutureSupport::notify()`` function sets the bits of the `future` object,
allowing execution to return to the main thread.

Let's introduce a runnable class AsyncCommandExecutor:

.. code-block:: cpp

    #include "async/FutureSupport.h"

    // Asynchronous command executor
    class AsyncCommandExecutor : private ::async::RunnableType
    {
    public:
        AsyncCommandExecutor() : _future(0) {} // assume context ID is 0
        async::FutureSupport _future;
    }

Execute asynchronously, wait until its execution is complete and proceed.

.. code-block:: cpp

    // synchronization of asynchronous execution
    void AsyncCommandExecutor::execWithWait
    {
        ::async::execute(context, *this); // enqueue async execution
        _future.wait(); // wait until the execution is done

        // .. some code that may only be processes after "execute" is finished
    }

    void AsyncCommandExecutor::execute()
    {
        // Do something and notify when complete
        _future.notify();
    }
