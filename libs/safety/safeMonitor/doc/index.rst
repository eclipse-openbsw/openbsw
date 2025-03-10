safeMonitor - System Integrity Classes
======================================

Overview
--------

The safeMonitor module is a collection of templated classes which formalize a condition, its check
and the reporting channel if the condition isn't met.

Design goals & assumptions
--------------------------

The monitors are subject to be used in safety relevant environments. Therefore certain design goals
are applied, which (not exclusively) are the following.

As little code as possible
++++++++++++++++++++++++++

Safety relevant code has to be reviewed. Less code means less to review. So keep that in mind if
you'd like to extend the monitor's functionality! Keep it simple and don't unnecessarily bloat the
functional scope.

No inheritance
++++++++++++++

Since it is not defined by the standard how inheritance and virtual function calls are to be
implemented by the compiler vendor, it's easier to avoid them entirely.

As few dependencies as possible
+++++++++++++++++++++++++++++++

Every dependency which gets introduced has to be reviewed from a safety point of view as well. So
it's better to avoid dependencies as much as possible.

ASIL partition
++++++++++++++

It is assumed that the monitors live within an ASIL partition which is protected by a MPU in order
to ensure freedom from interference. If a monitor has to be triggered/called from outside this
partition, the ScopedMutex might be used to "open" the MPU.

Usage
-----

General
+++++++

All monitors have in common that they need at least the following things:

#.  An event type that can be reported

    .. code-block:: C++

        enum MyEvent
        {
            MY_EVENT_A,
            MY_EVENT_B,
            // and any other event needed ...
        };

#.  A class or struct that provides a handle method

    .. code-block:: C++

        class MyHandler
        {
        public:
            void handle(MyEvent const& event)
            {
                switch (event)
                {
                    case MY_EVENT_A:
                    {
                        // do something ...
                        break;
                    }
                    case MY_EVENT_B:
                    {
                        // do something else
                        break;
                    }
                    // ...
                    // handle other events
                }
            }
        };

Examples
++++++++

Register
~~~~~~~~

The register monitor can be used to monitor an arbitrary set of registers. Despite the name, it
actually can monitor any addressable piece of memory. It does so by comparing entry by entry with an
expected value.

.. code-block:: C++

    #include <safeMonitor/Register.h>

    using MyRegisterMonitor = ::safeMonitor::Register<MyHandler, MyEvent, uint32_t>;

    const MyRegisterMonitor::Entry registers[] = {
        {0xDEADBEEF, 0xFF00FF00, 0xAB00BA00}, // Byte 4 expected to be 0xAB, Byte 2 expected to be 0xBA
        {0xB00000B5, 0xFFFFFFFF, 0x00000000}, // None of the 32 bits at address 0xB00000B5 expected to be set
        {0xBADA5555, 0x80000001, 0x80000001}  // MSB and LSB of memory at address 0xBADA5555 expected to be set
    };

    MyHandler handler;
    MyRegisterMonitor registerMonitor(handler, MyEvent::REGISTER_MISMATCH, registers);

    void cyclic()
    {
        // Will call the handler if one of the entries does not match its expected value
        registerMonitor.check();
    }

Sequence
~~~~~~~~

Sequence objects can be used to monitor the execution sequence. Deviations from the predefined
sequence are reported by calling a special method of a handler object. See hit() for more details.

.. code-block:: C++

    #include <safeMonitor/Sequence.h>

    enum MyCheckpoint
    {
        ALPHA,
        BETA,
        CHARLIE,
        DELTA,
    };

    using MySequence = ::safeMonitor::Sequence<MyHandler, MyEvent, MyCheckpoint>;

    MyHandler handler;
    MySequence s(handler, MyEvent::DEVIATION_FROM_SEQUENCE, MyCheckpoint::ALPHA, MyCheckpoint::DELTA);

    void someTask()
    {
        s.hit(MyCheckpoint::ALPHA); // ok
        s.hit(MyCheckpoint::BETA);  // ok
        s.hit(MyCheckpoint::CHARLIE);   // ok
        s.hit(MyCheckpoint::DELTA);     // ok
        s.hit(MyCheckpoint::ALPHA); // wrap to beginning -> ok
        s.hit(MyCheckpoint::CHARLIE);   // deviation -> handler gets called
    }

Trigger
~~~~~~~

Triggers can be used to immediately trigger the event handler.

.. code-block:: C++

    #include <safeMonitor/Trigger.h>

    using MyError = ::safeMonitor::Trigger<MyHandler, MyEvent>;

    MyHandler handler;
    MyError stackOverflow(handler, MyEvent::STACK_OVERFLOW);

    void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
    {
        stackOverflow.trigger(); // will call handler with event MyEvent::STACK_OVERFLOW
    }

Value
~~~~~

Value monitors can be used for checking if a given value matches an expected one. Mismatches are
reported by calling the handle method of the handler object.

.. code-block:: C++

    #include <safeMonitor/Value.h>

    using MyValueMonitor = ::safeMonitor::Value<MyHandler, MyEvent>;

    MyHandler handler;
    MyValueMonitor mpuClosed(handler, MyEvent::MPU_NOT_CLOSED, true);

    void someTask()
    {
        // ...
        mpuClosed.check(mpu.isClosed()); // will call handler if "mpu.isClosed()" returns false
        // ...
    }

Watchdog
~~~~~~~~

Watchdog monitors can be used to monitor algorithms or similar in terms of timeouts. Deadline
violations are reported by calling the handle method of the handler object. See service() for more
information.

.. code-block:: C++

    #include <safeMonitor/Sequence.h>

    using MyWatchdogMonitor = ::monitor::Watchdog<MyHandler, MyEvent>;

    MyHandler handler;
    MyWatchdogMonitor snoopy(handler, MyEvent::TIMEOUT, 100U);

    void cyclic5ms()
    {
        snoopy.service();
    }

    void someTaskToMonitor()
    {
        while (true)
        {
            // ...
            snoopy.kick(); // has to be called within 500ms, otherwise a timeout will occur
        }
    }

Memory boundaries
+++++++++++++++++

The monitors can be understood as a formal event channel. If used in a functional safety context,
the monitor objects themselves most likely live in a special memory region (partition), protected by
the MPU. Now if the monitored code lives in another partition, one does not simply communicate over
this memory boundary. The monitors provide a RAII based mechanism to solve this issue. All you have
to do is to pass another type into your template instantiation.

.. code-block:: C++

    struct ScopedMpuOpener
    {
        ScopedMpuOpener()
        {
            // check if memory is protected and open if necessary
        }

        ~ScopedMpuOpener()
        {
            // restore previous protection state
        }
    };

    // note the ScopedMpuOpener being passed as template argument
    using MyWatchdogMonitor = ::monitor::Watchdog<MyHandler, MyEvent, ScopedMpuOpener>;

    // assume the following two objects live in a safety partition...
    MyHandler handler;
    MyWatchdogMonitor snoopy(handler, MyEvent::TIMEOUT, 100U);

    // ...and this is a task running in QM
    void someTaskToMonitor()
    {
        while (true)
        {
            snoopy.kick(); // Mpu is being opened/closed accordingly
        }
    }

The above example shows what the monitors are all about. The caller doesn't need to know anything
about the odd functional safety driven implementation details. This also inverts the dependency and
makes it easy to implement the functional safety bits and pieces in a centralized manner.

Additional context
++++++++++++++++++

Sometimes, in addition to the event itself, more information is required for processing. For this
reason we can pass a context object into most monitor check functions. Let's take the trigger
example from above. It shows how one can trigger the handler method in case of a stack overflow.
But how could we forward the task number to the handler? Like in the memory boundary example above,
all we have to do is to pass yet another custom type into your template instance.

.. code-block:: C++

    #include <safeMonitor/Trigger.h>

    struct StackOverflowContext
    {
        uint8_t taskNumber;
    };

    // note the StackOverflowContext type being passed at the end
    using MyError = ::safeMonitor::Trigger<
        MyHandler, MyEvent, ::safeMonitor::DefaultMutex, StackOverflowContext>;

    MyHandler handler;
    MyError stackOverflow(handler, MyEvent::STACK_OVERFLOW);

    void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
    {
        // This will copy the given context into the stackOverflow object and call the handler
        stackOverflow.trigger({
            .taskNumber = pxTask->uxTaskNumber;
        });
    }

    void MyHandler::handle(MyEvent const& e)
    {
        switch (e)
        {
            case MyEvent::STACK_OVERFLOW:
            {
                auto const ctx = stackOverflow.getContext();
                // do something with ctx.taskNumber
            }
        }
    }

This example also works in combination with the memory boundaries example above since the context
object gets copied into the corresponding safeMonitor object. The context type can be arbitrarily
complex, as long as it is default constructible.
