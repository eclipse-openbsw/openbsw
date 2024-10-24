.. _async:

async - Asynchronous Execution Interface
========================================

Overview
--------
The ``async`` module defines an interface for running runnable objects in specific execution contexts.

To be able to run code in arbitrary contexts in a generic way, this module provides an abstraction on top of
OS specific primitives, allowing to write easily-portable concurrent code.

An ``async`` implementation provides an appropriate ``Types.h``, defining the types used by async.
The reference ``Types.h`` can be found in ``async/mock/gmock/include/async/Types.h``.
It provides an example of the types that need to be defined by all ``async`` implementations.
For a description of the types used by async, see :ref:`Concepts`.

A number of implementations are available:
 - asyncFreeRtos
 - asyncPosix

Features
--------
Three functions are specified by async:

===================   ==================================================================================
execute               Allows to execute a ``RunnableType`` immediately in an execution ``ContextType``
schedule              Allows to schedule a ``RunnableType`` to be run after a delay in a ``ContextType``
scheduleAtFixedRate   Allows to schedule a ``RunnableType`` to be run periodically in a ``ContextType``
===================   ==================================================================================

Additionally, async defines in ``util/Call.h`` an interface to create a ``RunnableType`` from an
executable object (i.e. that defines the ``()`` operator).
A ``Function`` type alias implementing it for an ``::estd::function`` is provided.

.. _asyncex:

Example
-------

.. code-block:: cpp

    #include <async/Async.h>
    #include <async/util/Call.h>

    constexpr ::async::TimeUnitType PERIOD_IN_S = 1;

    ::async::TimeoutType runTimeout;
    ::async::TimeoutType cancelTimeout;

    void runModule()
    {
        fprintf(stderr, "running!\n");
    }

    void stopModule()
    {
        runTimeout.cancel();
    }

    void startModule(::async::ContextType& context)
    {
        ::async::Function moduleRunnable(runModule);
        ::async::scheduleAtFixedRate(context, moduleRunnable, runTimeout, PERIOD_IN_S, ::async::TimeUnit::SECONDS);

        ::async::Function cancelRunnable(stopModule);
        ::async::schedule(context, cancelRunnable, cancelTimeout, 2*PERIOD_IN_S, ::async::TimeUnit::SECONDS);
    }

This example could produce the following output, if ``startModule`` is called with async properly set-up (the
initialization is platform dependant):

.. code-block::

    running!
    # 1s gap
    running!
    # end

.. _Concepts:

Concepts
--------

Context
+++++++

``ContextType`` is an execution context. All functions which are run in the same context are guaranteed
to be run sequentially, allowing safe access to shared resources.

It can be created from a ``uint8_t`` and is copyable, assignable, comparable and has a defined invalid value (``CONTEXT_INVALID``).

Runnable
++++++++

``RunnableType`` is an object defining a ``void execute(void)`` method, allowing it to be executed.
The ``IRunnable`` interface in ``AsyncImpl`` is one such ``RunnableType``, used by some implementations.

Locks
+++++

``LockType`` and ``ModifiableLockType`` are scoped locks. The modifiable counterpart can be unlocked
and locked manually.

.. warning::
    The funcional and non-functional semantics of these lock types can differ between implementations of this module
    for different target platforms. In some cases, platform specific usage invariants may apply. Using these lock types
    will impact the software's real-time performance and should only be employed when absolutely necessary
    and with an understanding of the wider impact of their use.

Time units
++++++++++

``TimeUnitType`` is a type providing a definition of micro-, milli- and full seconds.
The following values are available from the ``TimeUnit`` scope:

.. code-block:: cpp

    TimeUnit::MICROSECONDS
    TimeUnit::MILLISECONDS
    TimeUnit::SECONDS

Timeout
+++++++

``TimeoutType`` is a type that provides the memory to enable the use of ``schedule`` and ``scheduleAtFixedRate``.
It can be cancelled with its ``cancel`` method.

Integration
-----------

If a module uses async, it *must* check for the correct usage of types by writing a unit test with the async mocks,
provided in the ``mock/gmock`` sub-folder.

The following mocks are available, that mock all async methods:

 - AsyncMock
 - LockMock
 - TimeoutMock

When mocking all of async is not desired, ``TestContext.h`` and ``.cpp`` can be used.
They provide a way to emulate async behaviour, that can be triggered manually for a specific context.

Porting to a new platform
-------------------------

To port async to a new platform, one must start by providing an adequate ``Types.h``, mapping the concepts to the
target platform.
