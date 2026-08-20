..
   *******************************************************************************
   Copyright (c) 2026 BMW AG

   This program and the accompanying materials are made available under the
   terms of the Apache License Version 2.0 which is available at
   https://www.apache.org/licenses/LICENSE-2.0

   SPDX-License-Identifier: Apache-2.0
   *******************************************************************************

Integrating middleware into an ECU
==================================

Overview
--------

This section covers the ECU integration work that starts before service-specific
middleware artifacts have been generated (for example generated proxies,
skeletons, :term:`cluster connection` code and message types).

The remaining work in this guide is integration code, not generator output:
platform bindings (time provider, logger, OS task ID and concurrency lock
types), runtime object wiring, startup ordering and cyclic processing setup.

At a high level, middleware integration has six parts:

* Provide platform bindings required by middleware.
* Add the middleware deployment configuration.
* Add the generated middleware code to the ECU build.
* Provide the shared runtime objects used by middleware.
* Initialize middleware during ECU startup.
* Run the generated cluster processing entrypoints cyclically.

Required inputs and platform bindings
-------------------------------------

Before starting ECU integration, make sure these upstream inputs already
exist:

* Generated middleware headers and sources for the service deployment.
* Predefined queue and allocator configuration parameters for the target ECU
  (for example queue counts and allocator capacities).

For terminology and definitions used throughout this guide, see the
:doc:`../glossary`.

Middleware time interface
-------------------------

The middleware evaluates timeouts through the platform time interface.
The platform integrator must provide an implementation of these functions and
link it into the ECU build:

* **CMake**: add a source file defining the functions to any library that
  links ``middleware``/``middlewareInterfaces`` (see
  ``executables/referenceApp/middlewareConfiguration/platform_integration/time/src/SystemTimeProvider.cpp``
  for the reference implementation).

The interface consists of two functions:

.. code-block:: cpp

   namespace middleware::time
   {
   uint32_t getCurrentTimeInMs();
   uint32_t getCurrentTimeInUs();
   }

Integration requirements:

* ``getCurrentTimeInMs()`` must be monotonic for timeout handling.
* ``getCurrentTimeInUs()`` must be monotonic and provide a
  microsecond-resolution timestamp for consumers that require finer
  granularity.
* Both functions are 32-bit and may wrap around; implementations must preserve
  deterministic wrap behavior.

A typical bare-metal implementation reads the platform timer service:

.. code-block:: cpp

   #include "bsp/timer/SystemTimer.h" // platform specific header for getSystemTimeMs32Bit and getSystemTimeUs32Bit
   #include "middleware/time/SystemTimerProvider.h"

   namespace middleware::time
   {

   uint32_t getCurrentTimeInMs()
   {
     return getSystemTimeMs32Bit();
   }

   uint32_t getCurrentTimeInUs()
   {
     return getSystemTimeUs32Bit();
   }

   }  // namespace middleware::time

Logger platform interface
-------------------------

The middleware emits diagnostic and error records through a platform logger
interface. These functions must be implemented by the platform integrator and
linked into the ECU build:

* **CMake**: add a source file defining the functions to any library that
  links ``middleware``/``middlewareInterfaces`` (see
  ``executables/referenceApp/middlewareConfiguration/platform_integration/logger/src/LoggerImpl.cpp``
  for the reference implementation).

All logger functions below must be defined. Missing definitions will produce
linker errors for unresolved logger symbols.

.. code-block:: cpp

   namespace middleware::logger
   {
   void log(const LogLevel level, const char* const format, ...);
   void logBinary(const LogLevel level, const etl::span<const uint8_t> data);
   uint32_t getMessageId(const Error id);
   }  // namespace middleware::logger

Recommended behavior:

* Map ``LogLevel`` to your platform severity levels.
* Keep logging payloads bounded and deterministic in size and processing cost.
* Prefer structured, machine-readable payloads for diagnostics and analytics.
* Return stable message IDs from ``getMessageId(...)`` for log decoding.

Example implementation:

.. code-block:: cpp

   #include <array>
   #include <cstdarg>
   #include <cstdio>

   #include <etl/span.h>

   #include "middleware/logger/Logger.h"
   #include "Logger.h" // platform specific header toPlatFormLevel, platformWrite, platformBinaryWrite

   namespace middleware::logger
   {
   namespace
   {
   constexpr std::size_t kTextBufferSize = 256U;

   uint8_t toPlatformLevel(const LogLevel level)
   {
     switch (level)
     {
       case LogLevel::Critical: return 1U;
       case LogLevel::Error: return 2U;
       case LogLevel::Info: return 3U;
       // ...add remaining log levels used by your platform.
       case LogLevel::None:
       default: return 0U;
     }
   }

   void platformWrite(const uint8_t level, const char* text)
   {
     // Replace this with the target platform sink (DLT/UART/trace backend).
   }

   void platformBinaryWrite(const uint8_t level, const etl::span<const uint8_t> data)
   {
     // Replace this with the target platform binary sink (ex. DLT).
   }
   }  // namespace

   void log(const LogLevel level, const char* const format, ...)
   {
     std::array<char, kTextBufferSize> buffer{};
     va_list args;
     va_start(args, format);
     (void)std::vsnprintf(buffer.data(), buffer.size(), format, args);
     va_end(args);

     platformWrite(toPlatformLevel(level), buffer.data());
   }

   void logBinary(const LogLevel level, const etl::span<const uint8_t> data)
   {
       platformBinaryWrite(toPlatformLevel(level), data);
   }

   uint32_t getMessageId(const Error id)
   {
     switch (id)
     {
       case Error::Allocation: return 0x1001U;
       case Error::Deallocation: return 0x1002U;
       case Error::ProxyInitialization: return 0x1003U;
       // ...add stable IDs for all remaining error codes.
       default: return 0x10FFU;
     }
   }
   }  // namespace middleware::logger

.. note::
  A simulation logger implementation is available at
  ``//libs/bsw/middleware/simulation/`` folder for reference implementations.
  It is a useful reference when creating a target-specific integration.

OS platform interface
---------------------

The middleware uses an OS abstraction to identify the current task context.
This ID is used for crossthread checks in generated proxies and skeletons.

The function is provided by the platform integrator and linked into the ECU
build:

* **CMake**: add a source file defining the function to any library that
  links ``middleware``/``middlewareInterfaces`` (see
  ``executables/referenceApp/middlewareConfiguration/platform_integration/os/src/OsDefinitions.cpp``
  for the reference implementation).

The platform integrator must provide the following function:

.. code-block:: cpp

  namespace middleware::os
  {
  uint32_t getProcessId();
  }  // namespace middleware::os

If this symbol is not defined, linking fails with unresolved
``middleware::os::getProcessId``.

The function should return a stable identifier for the currently executing
task context.

Example implementation:

.. code-block:: cpp

   #include <cstdint>

   #include "middleware/os/TaskIdProvider.h"
   #include "TaskId.h" // platform specific header for Os::GetCurrentTaskId()

   namespace middleware::os
   {
   uint32_t getProcessId()
   {
     // Replace this with the target OS API and return a deterministic
     // uint32_t identifier for the current execution task context.
     return static_cast<uint32_t>(Os::GetCurrentTaskId());
   }
   }  // namespace middleware::os

.. note::
  A simulation reference implementation is available at:

  * ``//libs/bsw/middleware/simulation/platform_integration/os/``

  The function declaration is defined in
  ``libs/bsw/middleware/interfaces/include/middleware/os/TaskIdProvider.h``.

Concurrency lock types
----------------------

The middleware uses two platform-provided lock guards to protect critical
sections in core-local and ECU-shared paths. These types are exposed via
``middleware/concurrency/LockStrategies.h`` and provided by the platform
integrator:

* **CMake**: provide a ``middleware/concurrency/lock_types.h`` header on the
  include path of any library that links ``middleware``/``middlewareInterfaces``
  (see ``executables/referenceApp/middlewareConfiguration/include/platform_integration/middleware/concurrency/lock_types.h``
  for the reference implementation).

The platform integrator must provide both lock types in
``middleware/concurrency/lock_types.h`` under this namespace:

.. code-block:: cpp

   namespace middleware::concurrency::integration
   {
   struct ScopedCoreLock;
   struct ScopedECULock;
   }  // namespace middleware::concurrency::integration

Usage and ordering guidance:

* Use ``ScopedCoreLock`` only for short core-local critical sections.
* Use ``ScopedECULock`` for shared-memory objects visible across cores
  (queues, allocators, and related shared metadata).

Example implementation shape:

.. code-block:: cpp

   #include <cstdint>

   namespace middleware::concurrency::integration
   {
   struct ScopedCoreLock
   {
       ScopedCoreLock()
       {
           // Replace with platform primitive, e.g. disable interrupts.
           Platform::DisableInterrupts();
       }

       ~ScopedCoreLock()
       {
           // Restore previous core state.
           Platform::RestoreInterrupts();
       }

       ScopedCoreLock(ScopedCoreLock const&) = delete;
       ScopedCoreLock& operator=(ScopedCoreLock const&) = delete;
       ScopedCoreLock(ScopedCoreLock&&) = delete;
       ScopedCoreLock& operator=(ScopedCoreLock&&) = delete;
   };

   struct ScopedECULock
   {
       explicit ScopedECULock(uint8_t volatile* const mutex)
           : mutex_(mutex)
       {
           // Replace with platform primitive, e.g. spinlock/mutex lock.
           Platform::LockMutex(mutex_);
       }

       ~ScopedECULock()
       {
           Platform::UnlockMutex(mutex_);
       }

       ScopedECULock(ScopedECULock const&) = delete;
       ScopedECULock& operator=(ScopedECULock const&) = delete;
       ScopedECULock(ScopedECULock&&) = delete;
       ScopedECULock& operator=(ScopedECULock&&) = delete;

     private:
       uint8_t volatile* mutex_;
   };
   }  // namespace middleware::concurrency::integration

For single-threaded or simulation targets, no-op implementations are valid as
long as they preserve the same API and RAII behavior.

.. note::
  Reference implementations are available at:

  * ``//libs/bsw/middleware/test/platform_integration/concurrency/``
  * ``//libs/bsw/middleware/simulation/platform_integration/concurrency/``

  The interface aliases are defined in
  ``libs/bsw/middleware/interfaces/include/middleware/concurrency/LockStrategies.h``.

Add the generated middleware code to the ECU build
--------------------------------------------------

The generated code should be integrated as a normal ECU dependency instead of
being used directly from the generator output folder. In practice, the ECU
usually defines:

* One library containing the generated middleware headers and sources
  (for example ``middleware_generated`` with generated cluster connection and
  proxy/skeleton code).
* One library containing shared-memory setup code
  (for example ``middleware_shm`` with shared-memory layout types and
  accessors).

CMake build integration
+++++++++++++++++++++++

For CMake-based ECUs, generated code is produced by invoking the code
generator (``libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py``) from a
custom command driven by the deployment YAML, then compiled into a per-ECU
configuration library together with the platform binding sources.

The reference integration
(``executables/referenceApp/middlewareConfiguration/CMakeLists.txt``) follows
this pattern:

.. code-block:: cmake

   set(_GEN_SCRIPT ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py)
   set(_GEN_INPUT ${CMAKE_SOURCE_DIR}/libs/bsw/middleware/tools/cpp_generator)
   set(_DEPLOYMENT_YAML ${CMAKE_CURRENT_SOURCE_DIR}/model/deployment.yaml)
   set(_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})

   add_custom_command(
       OUTPUT ${_GENERATED_HEADERS} ${_GENERATED_SRCS}
       COMMAND ${Python3_EXECUTABLE} ${_GEN_SCRIPT}
               --input ${_GEN_INPUT} --output ${_GEN_DIR}
               --deployment-yaml ${_DEPLOYMENT_YAML}
       DEPENDS ${_GEN_SCRIPT} ${_DEPLOYMENT_YAML}
       COMMENT "Generating middleware C++ code from deployment.yaml"
       VERBATIM)

   add_library(middlewareConfiguration
               ${_GENERATED_SRCS}
               src/platform_integration/logger/LoggerImpl.cpp
               src/platform_integration/os/OsDefinitions.cpp
               src/platform_integration/time/SystemTimeProvider.cpp
               src/MemoryLayout.cpp)

   target_include_directories(middlewareConfiguration
       PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
              ${_GEN_DIR}/include/generated_code)

   target_link_libraries(middlewareConfiguration PUBLIC asyncBinding middleware logger)

Key points:

* ``add_custom_command`` regenerates code whenever the deployment YAML,
  templates, or generator script change; list all of them as ``DEPENDS``.
* Platform binding sources (time, logger, OS, concurrency) are compiled into
  this same configuration library, not into ``middleware`` itself. The
  concrete implementation is simply an additional source file.
* Link the resulting configuration library into the final ECU executable;
  linking ``middleware`` alone is not sufficient because it only provides the
  generic core, not the generated service code or platform bindings.

Instantiate shared-memory runtime objects from the queue and allocator configuration
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Middleware runtime objects must exist before any cluster processing
or service initialization can run. In the reference integration, these runtime
objects are placed in a :term:`shared memory region`:

* Queue mutexes.
* Queue instances.
* Allocator mutexes.
* Allocator instances.

The generated shared-memory configuration provides a ``MemoryLayout`` type that
owns these runtime objects. Its constructor performs the required queue and
allocator setup (allocator mutex binding). This constructor-based pattern
replaces separate queue/allocator initialize functions.

Exactly one :term:`owner core` must construct ``MemoryLayout`` once in the
:term:`shared memory region`. All :term:`attached core` instances must only bind to the same
already-constructed instance.

The generated shared-memory layout contains the deployment-specific queues and
allocators:

.. code-block:: cpp

   struct MemoryLayout
   {
     uint8_t queueToFooMutex{0U};
     uint8_t queueToBarMutex{0U};
     uint8_t defaultAllocatorMutex{0U};
     MiddlewareQueue<QUEUE_TO_FOO_SIZE> queueToFoo{&queueToFooMutex};
     MiddlewareQueue<QUEUE_TO_BAR_SIZE> queueToBar{&queueToBarMutex};
     message_allocator defaultAllocator{&defaultAllocatorMutex};
   };
   extern ETL_CONSTINIT etl::typed_storage<middleware::shm::MemoryLayout> middlewareData;
   extern middleware::shm::MemoryLayout& middleware::shm::getMemoryLayout();

SHM library .cpp:

.. code-block:: cpp

   ETL_CONSTINIT etl::typed_storage<middleware::shm::MemoryLayout> middlewareData SHARED_BSS;
   middleware::shm::MemoryLayout& middleware::shm::getMemoryLayout()
   {
     return *middlewareData;
   }

Integration on controller main core main.cpp:

.. code-block:: cpp

   void run_core()
   {
     middlewareData.create();
   }

.. note::
  ``middleware::shm::getMemoryLayout()`` is declared in generated SHM config
  headers, but its definition must be provided by platform/integrator shared
  memory integration code.

Shared-memory synchronization requirements
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Startup ordering must guarantee that attached cores do not access the shared
``MemoryLayout`` instance before owner-core construction has completed.

Required guarantees:

* The shared region mapping is complete on every participating core.
* The :term:`owner core` performs ``create()`` exactly once.
* :term:`attached core` instances wait until owner construction is complete
  before binding to or accessing the shared ``MemoryLayout`` instance.

In short, shared middleware data must be constructed once, by one core only.
All other cores must wait until that construction is finished before they bind
to or use the shared data.

Typical platform implementations use one of these barriers:

* A :term:`startup barrier` or event that owner core signals after successful construction.
* A shared state flag with release/acquire semantics.
* A platform startup sequencer that runs owner-core construction before
  attached-core binding.

If an :term:`attached core` binds before construction, behavior is undefined and may
look like random queue or allocator failures during early lifecycle cycles.

Memory layout sizing inputs
~~~~~~~~~~~~~~~~~~~~~~~~~~~

``MemoryLayout`` size is generated from deployment configuration. Integrators
should dimension shared memory from these inputs:

* Number of queues.
* Queue capacities/depth.
* Number of allocators and pool sizes.
* Per-object alignment requirements in generated layout types.

When queue or allocator configuration changes, reserve size must be revalidated
for the target memory region before integration testing.

Initialize middleware during ECU startup
----------------------------------------

Middleware initialization should be part of the ECU startup sequence, before
the application starts executing its regular lifecycle.

For the owner core, the reference order is:

#. Prepare/attach the shared memory region used by middleware runtime objects.
#. Construct ``middleware::shm::MemoryLayout`` exactly once in shared memory.
#. Initialize the generated cluster connections for the local cluster.
#. Continue with the remaining application startup.

These examples intentionally show only middleware-related responsibilities in
``main()`` and omit non-middleware system and lifecycle setup.

For an owner core, a minimal ``main.cpp`` sketch looks like:

.. code-block:: cpp

   #include <etl/typed_storage.h>

   #include "shm.h" // Platform specific header for the shared memory access
   #include "generated/cluster_connections.h"

   int main()
   {
     // Shared-memory mapping is platform-specific and done before middleware init.
     middlewareData.create(); // middlewareData is declared and instantiated in the shm lib

     // Initialize local generated cluster connections.
     middleware::initializeFooBarClusterConnection();

     // ...rest of application startup / scheduler...

     return 0;
   }

For an attached core, the order is slightly different because shared objects
already exist:

#. Attach to the shared memory region.
#. Bind to the existing ``middleware::shm::MemoryLayout`` instance.
#. Initialize the generated cluster connections for the local cluster.
#. Continue with the remaining application startup.

For an attached core, a minimal ``main.cpp`` sketch looks like:

.. code-block:: cpp

   #include <etl/typed_storage.h>

   #include "shm.h" // Platform specific header for the shared memory access
   #include "generated/cluster_connections.h"

   int main()
   {
     // Shared-memory mapping is platform-specific and done before middleware init.
     // Owner core constructs shared data once; attached core waits and binds.
     platform::waitForMiddlewareShmReady();

     // Initialize local generated cluster connections.
     middleware::initializeFooBarClusterConnection();

     // ...rest of application startup / scheduler...
     return 0;
   }

For 3 or more participating cores, apply the same ownership rule:

#. :term:`owner core` maps shared memory and constructs ``MemoryLayout``.
#. :term:`owner core` signals construction completion through a platform barrier.
#. :term:`attached core` instances wait for that signal and only then bind to ``value()``.
#. Each core initializes only local :term:`cluster connection` objects and wrappers.

Attached cores can then continue in parallel after the construction-complete
barrier has been observed. In general, each core should initialize only the
generated :term:`cluster connection` objects that belong to the :term:`middleware cluster`
executed on that core.

Run cluster processing cyclically in the application lifecycle
--------------------------------------------------------------

Generated proxies and skeletons do not make progress on their own. Each local
:term:`middleware cluster` must be driven by a periodic or otherwise well-defined
application lifecycle hook.

In the reference integration, the application lifecycle calls the generated
cluster processing function from its cyclic execution path:

.. code-block:: cpp

   void App::cyclic()
   {
       // Application logic...
     middleware::processFooBarCluster();
   }

That generated function performs the middleware work for the local cluster:

* It takes a snapshot of the incoming :term:`queue`.
* Dispatches messages according to the source cluster.
* Calls the generated :term:`cluster connection` routing logic.
* Updates timeouts when requested.

If this cluster processing entrypoint is not called, requests, responses,
attribute updates and timeout handling will not progress.

Example mapping: reference ECU flow
-----------------------------------

A reference ECU can use a compact flow with a provider on one
core and a consumer on another:

* The middleware build target collects generated code, shared-memory support
  and wrapper code.
* Core 0 prepares shared memory, constructs ``middleware::shm::MemoryLayout``
  and initializes the server-side
  cluster connection.
* Core 1 attaches to shared memory, binds to the existing
  ``middleware::shm::MemoryLayout`` instance and
  initializes the client-side cluster connection.
* The server application initializes its generated skeleton wrapper.
* The lifecycle periodically calls the generated cluster processing function.

This is a good reference pattern, but it should be treated as an example of
integration order rather than as the only valid ECU structure.

Extracting statistics from the middleware system
------------------------------------------------

Middleware exposes runtime statistics for queues and memory allocation paths.
These statistics are useful during bring-up and can also be exported to
diagnostics for continuous health monitoring.

What can be extracted
+++++++++++++++++++++

For each queue instance, you can extract queue statistics including
``processedMessages``, ``lostMessages``, ``maxLoad``, ``startupLoad``, and
``maxFillRate``.

For memory behavior, you can read allocator totals via
``AllocatorBase::getStats()``, which provides ``allocations``,
``deallocations``, and ``unknownPtrsError``. You can also inspect per-pool
metrics through ``PoolStats``, including ``successfulAllocations``,
``failedAllocations``, ``delegatedAllocations``, ``internalFragmentation``,
and ``maxLoad``.

How to read and reset metrics
+++++++++++++++++++++++++++++

Queue API:

* Read current values with ``queue->getStats()``
* Start a new measurement window with ``queue->resetStats()``

Memory API:

* Read allocator totals with ``allocator.getStats()``
* Read a specific pool via ``pool.getPoolStats()``
* Collect all pools via ``Aggregator::collectStats(collector)``
* Reset queue statistics explicitly with ``queue->resetStats()``; pool
  statistics should be reset explicitly by the integration if a new pool
  measurement window is required.

Where to call it in ECU lifecycle
+++++++++++++++++++++++++++++++++

Collect metrics in the same periodic path that already calls
the generated cluster processing entrypoint. Use a slower sampling interval than the
cluster processing period (for example every 1-15 seconds) to keep overhead
bounded.

Recommended pattern:

#. Call ``process<LocalCluster>Cluster()`` every cycle.
#. Every N cycles read queue and memory statistics.
#. Publish/log values.
#. Optionally reset the counters to start a new observation window.

Example extraction loop
+++++++++++++++++++++++

.. code-block:: cpp

   void App::cyclic()
   {
       static uint32_t cycleCounter = 0U;

  middleware::processFooBarCluster();

       // Example: collect metrics every 100 cycles.
       if ((cycleCounter % 100U) == 0U)
       {
           const auto& qStats = middleware::shm::getQueueToCore0()->getStats();
           middleware::logger::log(middleware::logger::LogLevel::Info,
                                   "Q processed=%u lost=%u maxLoad=%u maxFillRate=%u",
                                   qStats.processedMessages,
                                   qStats.lostMessages,
                                   qStats.maxLoad,
                                   qStats.maxFillRate);

           const auto& aStats = middleware::shm::getAllocatorCore0()->getStats();
           middleware::logger::log(middleware::logger::LogLevel::Info,
                                   "A alloc=%u dealloc=%u invalidPtr=%u",
                                   aStats.allocations,
                                   aStats.deallocations,
                                   aStats.unknownPtrsError);

           // Optional window reset.
           middleware::shm::getQueueToCore0()->resetStats();
       }

       ++cycleCounter;
   }
