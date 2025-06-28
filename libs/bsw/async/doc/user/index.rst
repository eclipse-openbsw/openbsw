User documentation
==================

Overview
--------
The ``async`` module defines an interface for asynchronous execution of runnable objects in specific execution contexts.

To be able to run code on arbitrary platforms in a generic way, this module provides an abstraction on top of
OS specific primitives. ``async`` uses the underlying ``async::AsyncBinding`` type to access to specific OS
implementations, for instance via ``async::FreeRtosAdapter`` for **FreeRTOS** OS.

Features
--------
Three asynchronous (non-blocking) functions are declared by async in ``Async.h``:

===================   ==================================================================================
execute               Allows to execute a ``async::RunnableType`` immediately in an execution ``async::ContextType``
schedule              Allows to schedule a ``async::RunnableType`` to be run after a delay in a ``async::ContextType``
scheduleAtFixedRate   Allows to schedule a ``async::RunnableType`` to be run periodically in a ``async::ContextType``
===================   ==================================================================================

The ``util/Call.h`` declares a runnable class that allows customized implementation on execution
by providing callable object (with *function call* operator ``()``).
For example the predefined ``async::Function`` type is declaring ``::etl::delegate`` as callable type.

The ``util/MemberCall.h`` provides a ``async::MemberCall`` class, which allows to create a ``async::RunnableType``
from a member function of a non-runnable class.

.. _asyncex:

Examples
--------

Making runnable class and scheduling it
+++++++++++++++++++++++++++++++++++++++

Define a class that implements the ``async::IRunnable`` interface:

.. code-block:: cpp

    #include <async/Async.h>

    class RunnableImpl : public ::async::IRunnable
    {
    public:
        void execute() override
        {
            fprintf(stdout, "Executing user logic!\n");
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

.. _asynCall:

Use ``async::Function`` to provide customized implementation on execution
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Allocate the timeout variables and define execution callback functions:

.. code-block:: cpp

    static constexpr ::async::TimeUnitType PERIOD_IN_S = 1;
    static ::async::TimeoutType runTimeout;
    static ::async::TimeoutType cancelTimeout;

    void runModule()
    {
        fprintf(stdout, "running!\n");
    }

    void stopModule()
    {
        runTimeout.cancel();
        fprintf(stdout, "stopped!\n");
    }

In client code specify the ``async::Function`` objects and schedule them:

.. code-block:: cpp

    void startStopModule(::async::ContextType& context)
    {
        // execute runModule every PERIOD_IN_S:
        ::async::Function moduleRunnable(runModule);
        ::async::scheduleAtFixedRate(context, moduleRunnable, runTimeout, PERIOD_IN_S, ::async::TimeUnit::SECONDS);

        // execute stopModule after 2*PERIOD_IN_S:
        ::async::Function cancelRunnable(stopModule);
        ::async::schedule(context, cancelRunnable, cancelTimeout, 2*PERIOD_IN_S, ::async::TimeUnit::SECONDS);
    }

This example could produce the following output, if ``startStopModule()`` is called:

.. code-block::

    running!
    running!
    stopped!

.. _asynMemberCall:

Use ``async::MemberCall`` to force non-runnable class behave like runnable
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Declare a non-runnable class:

.. code-block:: cpp

    #include <async/Async.h>
    #include <async/util/MemberCall.h>

    class NonRunnable // the class is NOT inheriting from Runnable!
    {
    public:
        void run()
        {
            fprintf(stderr, "running!\n");
        }
        // must not necessarily be member of the class:
        ::async::MemberCall<NonRunnable> memberCall{this, &NonRunnable::run};
    };

In client code, use the ``async::MemberCall`` object to schedule the execution:

.. code-block:: cpp

    void executeNonRunnable(NonRunnable & nonRunnable, ::async::ContextType& context)
    {
        // asynchronously call the run method of the NonRunnable object:
        ::async::execute(nonRunnable.memberCall, 0);
        // asynchronously call the run method with delay 1 second:
        ::async::schedule(context, nonRunnable.memberCall, timeout, 1, ::async::TimeUnit::SECONDS);
        // asynchronously call the run method periodically every 2 seconds:
        ::async::scheduleAtFixedRate(context, nonRunnable.memberCall, timeout, 2, ::async::TimeUnit::SECONDS);
    }

.. _RelevantTypes:

Relevant types
--------------

Context
+++++++

``async::ContextType`` represents an execution context. All functions that run in the same context are guaranteed
to be run sequentially, allowing safe access to shared resources.

It can be created from a ``uint8_t`` and is copyable, assignable, and comparable, with a defined invalid value (``CONTEXT_INVALID``).

Runnable
++++++++

``async::RunnableType`` is an object that defines a ``void execute(void)`` method, allowing it to be executed.
The ``async::IRunnable`` interface in ``async::AsyncImpl`` is one example of a ``async::RunnableType``, used by certain implementations.

Locks
+++++

``async::LockType`` and ``async::ModifiableLockType`` are scoped locks. The modifiable counterpart can be unlocked
and locked manually.

.. warning::
    The functional and non-functional semantics of these lock types can differ between implementations of this module
    for different target platforms. In some cases, platform specific usage invariants may apply. Using these lock types
    will impact the software's real-time performance and should only be employed when absolutely necessary
    and with a clear understanding of their broader impact.

Time units
++++++++++

``async::TimeUnitType`` is an alias of ``TimeUnit`` type providing a definition of micro-, milli-, and full seconds.
The following values are available within the ``async::TimeUnit`` scope:

.. code-block:: cpp

    TimeUnit::MICROSECONDS
    TimeUnit::MILLISECONDS
    TimeUnit::SECONDS

Timeout
+++++++

``async::TimeoutType`` provides the necessary memory to support the use of ``schedule()`` and ``scheduleAtFixedRate()``.
It can be cancelled using its ``cancel()`` method.

Integration
-----------

If a module uses ``async``, it *must* verify the correct usage of types by writing a unit test with the ``async`` mocks provided in the ``mock/gmock`` subfolder.

The following mocks are available, each of which mocks all ``async`` methods:

 - AsyncMock
 - LockMock
 - TimeoutMock

When mocking all of async is not desired, ``TestContext.h`` and ``.cpp`` can be used.
They provide a way to emulate async behaviour, that can be triggered manually for a specific context.

Porting to a new platform
-------------------------

To port async to a new platform, one must start by providing corresponding AdapterType, mapping the concepts to the
target platform.
