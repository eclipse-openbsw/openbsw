.. _learning_tracing:

Tracing
=======

Previous: :ref:`learning_hwio`

The tracing feature collects scheduling events like task switches and interrupt entry and exit.
This data can be used by tracing tools (e.g. `babeltrace2 <https://babeltrace.org/>`_)
running on a host to visualize the kernel's inner workings and the behavior of its subsystems
(e.g. when a task is switched in or out).

The internal trace format is very efficient and usually uses a single 32 bit word per traced event.
After the trace is done, the data is downloaded via a debugger and a post processing script is
used to convert the traced data to other trace formats.

Usage
-----

Before using the tracer it should be initialized. Initialization also clears the tracing buffer
and ensures that existing data in the buffer is not confused with actual tracing data.

.. code-block:: cpp

	runtime::Tracer::init();

When tracing is started, events are recorded into the tracing buffer.
It is possible to stop tracing and restart it later. Note however, that the time span
recorded between events before and after a longer pause might be wrong due to overflow
of the underlying clock.

.. code-block:: cpp

	runtime::Tracer::start();
	runtime::Tracer::stop();

Once the trace buffer is full, the tracing will stop automatically.
You can use the `init()` method to start over with an empty trace buffer.

In order to trace their own custom events, users may use the method `traceUser(uint8_t usrIdx)`.
The `usrIdx` argument can be used to distinguish different user events.

.. code-block:: cpp

	runtime::Tracer::traceUser(uint8_t usrIdx);

Building
--------

Use build flag `-DBUILD_TRACING=ON` to turn tracing on and `-DTRACING_BUFFER_SIZE` to configure
the tracing buffer size. For example, to build for S32K148EVB:

.. code-block:: bash

    cmake -B cmake-build-s32k148-tracing -S executables/referenceApp -DBUILD_TARGET_PLATFORM="S32K148EVB" \
        -DBUILD_TRACING=ON -DTRACING_BUFFER_SIZE=65536 --toolchain ../../admin/cmake/ArmNoneEabi-gcc.cmake
    cmake --build cmake-build-s32k148-tracing --target app.referenceApp -j

The size of tracing buffer in RAM is configurable (TRACING_BUFFER_SIZE).
If it is not provided in the cmake command, then the default value of 4096 bytes is configured.

Analyzing the Data
------------------

Once the application is run, one can collect binary traces using gdb:

.. code-block:: bash

    (gdb) info address runtime::Tracer::_ramTraces
    Symbol "runtime::Tracer::_ramTraces" is static storage at address 0x1ffee6bc.
    (gdb) dump binary memory trace_file 0x1ffee6bc 0x1ffee6bc+65536

This is an example of using trace_convert.py script to convert a binary trace to human-readable format.

.. code-block:: bash

    cp trace_file .
    cp tools/tracing/trace_convert.py .
    python3 ./trace_convert.py trace_file > output

This is an example of using the babeltrace2 tool and source plugin bt_plugin_openbsw.py to convert
a binary trace to human-readable format. The plugin is used to read trace data from ``trace_file``
in described format and feed it into the babeltrace2 framework for conversion.

.. code-block:: bash

    cp trace_file .
    cp tools/tracing/trace_convert.py .
    cp tools/tracing/bt_plugin_openbsw.py .
    babeltrace2 --plugin-path . -c source.openbsw.OpenBSWSource --params 'inputs=["trace_file"]' > output

This is an example output:

.. code-block:: none

    [01:00:00.009286800] (+?.?????????) thread_switched_in: { id = 8 }
    [01:00:00.009307300] (+0.000020500) thread_switched_out: { id = 8 }
    [01:00:00.009312387] (+0.000005087) thread_switched_in: { id = 7 }
    [01:00:00.009321662] (+0.000009275) thread_switched_out: { id = 7 }
    [01:00:00.009325687] (+0.000004025) thread_switched_in: { id = 6 }
    [01:00:00.009459762] (+0.000134075) thread_switched_out: { id = 6 }
    [01:00:00.009465125] (+0.000005363) thread_switched_in: { id = 5 }
    [01:00:00.009473562] (+0.000008437) thread_switched_out: { id = 5 }
    [01:00:00.009478112] (+0.000004550) thread_switched_in: { id = 4 }
    [01:00:00.009486075] (+0.000007963) thread_switched_out: { id = 4 }
    [01:00:00.009490187] (+0.000004112) thread_switched_in: { id = 3 }
    [01:00:00.009500287] (+0.000010100) thread_switched_out: { id = 3 }
