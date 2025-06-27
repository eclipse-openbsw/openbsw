.. _learning_benchmark:

Benchmark mode
==============

Previous: :ref:`learning_tracing`

The reference app can be built to run benchmark tests. There is a BenchmarkSystem class that
implements the following tests:

* interrupt latency test - to measure the time between raising the interrupt line and entering the ISR
* task switch latency test - to measure the time between an async runnable setting an event
  and another (woken up) runnable starting execution
* task switch after timeout latency test - to measure the time between scheduling an async runnable
  and its start of execution (woken up by a timer)
* load test - record CPU idle time during cyclic artificial load across various tasks

Use the following command to build the reference app in benchmark mode for the S32K148EVB:

.. code-block:: bash

    cmake -B cmake-build-s32k148-benchmark -S executables/referenceApp -DBUILD_TARGET_PLATFORM="S32K148EVB" \
        -DBUILD_BENCHMARK=ON --toolchain ../../admin/cmake/ArmNoneEabi-gcc.cmake
    cmake --build cmake-build-s32k148-benchmark --target app.referenceApp -j

When started, the application runs benchmark tests and prints out measurement results on the console.
