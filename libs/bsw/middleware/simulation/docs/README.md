<!--
 *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Middleware Simulation Example

- [Overview](#overview)
- [Concepts](#concepts)
- [Configuration](#configuration)
- [Structure](#structure)
- [Initialization](#initialization)
- [Services](#services)
- [Communication](#communication)
  - [Attribute Broadcasts](#attribute-broadcasts)
  - [Attribute Get](#attribute-get)
  - [FooMethod and Application Polling](#foomethod-and-application-polling)
  - [PDU Writers and Readers](#pdu-writers-and-readers)
- [Building](#building)

---

## Overview

This example illustrates the basic principle of how the middleware communicates
between software components on different cores. It uses POSIX shared memory
(SHM) to implement the inter-core queues so that the simulation can run on any
Linux host without real embedded hardware.

Three clusters are simulated on three cores. Cluster2 runs the PDU broker:

| Core  | Role                | Service Role |
|-------|---------------------|-------------|
| Cluster0 | Provider / Server   | `FooSkeleton` — provides the `Foo` service; writes PDUs |
| Cluster1 | Consumer / Client   | `FooProxy`   — consumes the `Foo` service; reads PDUs |
| Cluster2 | PDU broker          | `PduBrokerConnection` — transmits the PDUs the writers committed to a simulated bus and stores PDUs received from it |

At startup a shared memory segment is created and the `MemoryLayout` (queues and
allocator pools) is constructed in it via placement new. All cores are then
started either as **threads** (default) or as **OS processes** (compile-time
option).

---

## Concepts

| Term                    | Description |
|-------------------------|-------------|
| ECU                     | Electronic Control Unit. In this simulation the host process is considered to be the ECU. |
| Proxy                   | A class providing the API for using a service. Instantiated by the service consumer. |
| Skeleton                | A class providing the API for providing a service. Instantiated by the service provider. |
| Service Provider/Server | A software component offering a service. |
| Service Consumer/Client | A software component using a service from a provider. |
| Attribute               | A typed property on the service provider side, defined as part of the service interface. |
| Method                  | An asynchronous request/response operation; the simulation exposes the FooMethod response through application polling. |

---

## Configuration

```mermaid
graph LR
    subgraph ECU
        subgraph Cluster0[Cluster0 - Provider]
            FooSkeleton
            PduWriter
        end

        subgraph Cluster1[Cluster1 - Consumer]
            FooProxy
            PduReader
        end

        subgraph Cluster2[Cluster2 - PDU broker]
            PduBrokerConnection
        end

        Cluster0 -- SHM queue --> Cluster1
        Cluster1 -- SHM queue --> Cluster0
        PduWriter -- "PDU in SHM + PDU ID queue" --> PduBrokerConnection
        PduBrokerConnection -- "PDU in SHM (from bus)" --> PduReader
    end

    PduBrokerConnection <--> Bus["simulated bus"]
```

The deployment model (`model/deployment-test.yaml`) describes:

- **Three clusters**: `Cluster0` (id 0), `Cluster1` (id 1) and `Cluster2` (id 2). Cluster2
  has no service connections; it only runs the PDU broker.
- **One service**: `Foo` (namespace `org::test::foo`, id 1) with a
  read/write attribute `FooDefault` of type `FooStruct { uint32_t fooValue }`
  and method `FooMethod(uint32_t)` returning `FooMethodResult`, an `etl::array<uint32_t, 4>`.
- **Two connections**:
  - `Cluster0 -> Cluster1` skeleton-only: Cluster0 provides `FooSkeleton`.
  - `Cluster1 -> Cluster0` proxy-only: Cluster1 consumes `FooProxy`.
- **Four PDUs** with unique, non-sequential IDs: `samplePduWithCrc` (40) and
  `samplePduWithDatatypes` (10) are written by Cluster0; `samplePduWithCrcFromBus`
  (30) and `samplePduWithDatatypesFromBus` (20) have no writer and are stored by
  the broker on Cluster2. `outbound_pdu_queue_size` is 10.

---

## Structure

```
libs/bsw/middleware/simulation/
+-- model/
|   +-- deployment-test.yaml        # Deployment YAML fed to jinja2cpp.py
+-- include/
|   +-- etl_profile.h               # ETL config for host
|   +-- foo/
|   |   +-- FooSkeletonWrapper.h    # FooSkeleton concrete impl (Cluster0)
|   |   +-- FooProxyWrapper.h       # FooProxy wrapper (Cluster1)
|   +-- Logger.h                    # Thread-safe simulation logger
+-- platform_integration/
|   +-- concurrency/include/middleware/concurrency/lock_types.h
|   |                               # ScopedCoreLock / ScopedECULock no-op stubs
|   +-- logger/src/LoggerImpl.cpp  # middleware::logger::log() via fprintf
|   +-- os/src/OsDefinitions.cpp   # middleware::os::getProcessId()
|   +-- time/src/SystemTimeProvider.cpp
+-- src/
|   +-- Logger.cpp
|   +-- MainCore0.cpp               # Cluster0 entry point (FooSkeleton provider loop)
|   +-- MainCore1.cpp               # Cluster1 entry point (FooProxy consumer loop)
|   +-- MainCore2.cpp               # PDU broker entry point (simulated bus driver)
|   +-- Sample.cpp                  # main(): creates SHM, spawns cores
|   +-- ShmWrapper.cpp              # SHM open/mmap wrapper
|   +-- ShmWrapper.h
+-- docs/
|   +-- README.md                   # This file
+-- test/
|   +-- FooMethodTest.cpp           # FooMethod integration tests (real generated code)
|   +-- PduAccessTest.cpp           # Tests of the generated PDU table, load/store and PduBroker
|   +-- MemoryLayout.cpp            # In-process getMemoryLayout() shared by the tests
+-- BUILD.bazel
+-- CMakeLists.txt
+-- generated_outputs.bzl           # Expected generated file lists (Bazel)
```

---

## Initialization

```
main()
  +- ShmWrapper::init()          -- opens / creates the POSIX SHM segment
  +- createMemoryLayout()        -- placement-new of queues + pools in SHM
  +- [thread|fork] Cluster0         -- run_main_core0()
  +- [thread|fork] Cluster1         -- run_main_core1()
  +- [thread|fork] Cluster2         -- run_main_core2()
```

**Cluster0 startup** (`run_main_core0`):

1. `middleware::initializeCluster0ClusterConnection()` — wires the Cluster0-side
   cluster to its SHM queues.
2. `FooSkeletonWrapper::init()` — registers the `Foo` service on Cluster0 with
   `Cluster0ProviderFoo`.
3. Main loop: calls `sendBroadcast()` every 5 s, processes incoming messages,
   logs stats every 15 s, exits after 60 s.

**Cluster1 startup** (`run_main_core1`):

1. `middleware::initializeCluster1ClusterConnection()` — wires Cluster1 to its SHM
   queues.
2. `FooProxyWrapper::init()` — registers a change-notification callback on
   `fooDefault` and initialises the `FooProxy` on Cluster1.
3. Main loop: calls `requestGet()` and `requestFooMethod()` every 6 s, processes
   incoming messages, calls `pollFooMethod()` after each processing step, logs stats
   every 15 s, exits after 60 s.

**Cluster2 startup** (`run_main_core2`):

1. `middleware::initializeCluster2ClusterConnection()` — wires Cluster2 to its SHM
   queue.
2. A `PduBrokerConnection` is created from the generated PDU access table and
   the PDU id queue, with `SimPduTxSink` standing in for the bus driver.
3. Main loop: processes incoming messages; every 5 s it stores two PDUs received from the bus
   (`samplePduWithCrcFromBus` and `samplePduWithDatatypesFromBus`, which have no
   writer and the signals of the PDUs that Cluster0 writes), transmits the PDUs
   the writers committed in commit order, logs the PDU id queue stats and the number of discarded unknown IDs every 15 s
   and exits after 60 s. Cluster1 reads all PDUs, including the two from the bus.

---

## Services

### Foo

| Property       | Value |
|----------------|-------|
| Namespace      | `org::test::foo` |
| Service id     | 1 |
| Instance id    | `Cluster0ProviderFoo` = 0 |
| Attribute      | `FooDefault` -- `FooStruct { uint32_t fooValue }` |
| Access         | read / write / subscribe |
| Method         | `FooMethod`, member id 3, timeout 5000 ms, one request per consumer/provider |

The `FooSkeletonWrapper` (Cluster0) increments `fooValue` on every broadcast.
The `FooProxyWrapper<Cluster1>` receives broadcasts via a registered callback and
also issues explicit getter requests.

---

## Communication

### Attribute Broadcasts

When `FooSkeletonWrapper::sendBroadcast()` is called the skeleton writes the
updated `FooStruct` into the SHM queue towards Core1. The middleware event
mechanism delivers it to the change-notification callback registered via
`fooDefault.setReceiveHandler(...)` on the proxy side.

```
Cluster0 FooSkeleton::fooDefault.set(data)
Cluster0 FooSkeleton::fooDefault.send()
         |
         v   SHM queue (Cluster0 -> Cluster1)
Cluster1 FooProxy::onFooDefaultChanged(val)  <- setReceiveHandler callback
```

### Attribute Get

`FooProxyWrapper::requestGet()` sends a getter request through the SHM queue.
The skeleton's `get_FooDefaultAttribute()` override is called, and the response
is routed back to `onGetResponse()` on the proxy side.

```
Cluster1 fooDefault.get(cb)
         |
         v   SHM queue (Cluster1 -> Cluster0)
Cluster0 FooSkeleton::get_FooDefaultAttribute(response)
         |
         v   SHM queue (Cluster0 -> Cluster1)
Cluster1 onGetResponse(val)
```

### FooMethod and Application Polling

`FooMethod(x)` returns exactly four `uint32_t` values: `[x, x+1, x+2, x+3]`.
Arithmetic wraps modulo 2^32: `FooMethod(UINT32_MAX)` returns
`[UINT32_MAX, 0, 1, 2]`. FooMethod does not change the Foo attribute.

1. Cluster1 calls `FooProxyWrapper::requestFooMethod(x)`, which sends a generated
   `FooProxy::fooMethod` request through the SHM queue to Cluster0.
2. Cluster0 dispatches to `FooSkeletonWrapper::fooMethod`, computes the array and
   sends it immediately with `respondFooMethod`.
3. Cluster1 processes the response. The middleware callback **copies** the
   array into the wrapper before the transport releases its response storage.
4. The application calls `pollFooMethod()` without blocking. A terminal result is
   consumed exactly once; the next poll returns `Invalid` (idle).

The middleware remains callback-based; polling is only an application-level
adapter in this simulation. All wrapper operations and middleware dispatch run
on the consumer's thread/process.

| Poll state | Meaning |
| ------------ | --------- |
| `Invalid` | Idle: no outstanding call or unconsumed result. |
| `Pending` | Request is outstanding; polling does not consume it. |
| `Ready` | `value` contains the four result elements; this poll consumes them. |
| Other `Future::State` | Terminal error, including `Timeout`; this poll consumes it. |

Only inspect `value` when the state is `Ready`. `requestFooMethod` returns
`HRESULT::ServiceBusy` while a call is pending **or** a terminal result/error
has not yet been polled. It never overwrites that result. An immediate send
failure is logged and returned as an `HRESULT`, leaving the wrapper idle.
Response errors and the 5000 ms timeout are returned through polling and
logged by the consumer loop. Timeout checking uses the existing middleware
processing cycle. There are no automatic retries; after an error is consumed,
the next regular call may start. `deInit()` clears pending and stored results.

The consumer sends a new start value every 6 seconds and logs every element:

```text
[Cluster1] FooMethod request sent, startValue=0
[Cluster1] FooMethod result, values=[0, 1, 2, 3]
[Cluster1] FooMethod request sent, startValue=1
[Cluster1] FooMethod result, values=[1, 2, 3, 4]
```

The input (4 bytes) and output (16 bytes) fit in the existing inline message
payload. No allocator or queue-size increase is necessary.

### PDU Writers and Readers

**Writer** (Cluster0, `samplePduWithCrcWriter`, `samplePduWithDatatypesWriter`):
`writeX(...)` stages a single signal and `write(...)` stages all of them. Nothing
reaches shared memory before `commit()`. `commit()` only stores the PDU; the
`transmission` strategies of the writer put its ID in the PDU id queue on a
Cluster0 tick. A PDU can only have one writer.

**Broker** (Cluster2): `processTransmissions()` drains the PDU id queue in commit
order, takes a snapshot of each PDU and hands it to the bus. Each ID leaves the
queue as soon as its PDU is handed over. In the other direction,
`onReceive(id, payload)` stores a PDU received from the bus. It returns `false`
for an unknown ID or a payload of the wrong size.

**Reader** (Cluster1): `read()` takes a consistent snapshot and returns the
decoded signals. The reader of `samplePduWithCrc` is given a `Qualifier`, whose
`qualify()` is called for every snapshot before the signals are decoded. The
application checks the result and logs `CRC invalid` if it is not `Ok`. The
datatypes reader passes `nullptr`, as that PDU has no CRC.

```
Cluster0 writer.write...(), writer.commit()
         |
         v   PDU in SHM + ID in the PDU id queue
Cluster2 PduBroker::processTransmissions()  -> bus Tx

Cluster2 PduBroker::onReceive(id, payload)  <- bus Rx
         |
         v   PDU in SHM
Cluster1 reader.read()  -> qualify() -> signals
```

---

## Building

### Prerequisites

Standard OpenBSW POSIX build environment (see top-level README).
The simulation ETL profile defines `ETL_USE_TYPE_TRAITS_BUILTINS`. The
simulation executable therefore does not build with the compilers of the
development container (GCC 14/15, Clang 17); Clang 21 with libc++ is the
only verified configuration. The simulation is not built in CI.
The FooMethod unit test does not use this profile and builds with the regular
unit-test configuration.

The simulation requires `BUILD_EXECUTABLE=middlewareSimulation`. This is a
separate build tree from the `referenceApp`; use a dedicated build directory.

```bash
cmake --preset posix-threadx -B build/middleware-sim \
      -DBUILD_EXECUTABLE=middlewareSimulation
cmake --build build/middleware-sim --target middlewareSimulation
```

### Process-based execution

By default the three cores run as **threads** inside a single process.  To run
each core in its own OS **process** (true process isolation via `fork`):

```bash
cmake --preset posix-threadx -B build/middleware-sim \
      -DBUILD_EXECUTABLE=middlewareSimulation \
      -DSIMULATION_USE_PROCESSES=ON
cmake --build build/middleware-sim --target middlewareSimulation
```

### Run

```bash
./build/middleware-sim/libs/bsw/middleware/simulation/Release/middlewareSimulation
```

### Regenerating code

CMake automatically regenerates the C++ code in the build directory when the
deployment model (`model/deployment-test.yaml`) or generator templates change.
For manual generation, also use a build-directory destination:

```bash
python3 libs/bsw/middleware/tools/cpp_generator/jinja2cpp.py \
  --input  libs/bsw/middleware/tools/cpp_generator \
  --output build/middleware-sim/libs/bsw/middleware/simulation \
  --deployment-yaml libs/bsw/middleware/simulation/model/deployment-test.yaml
```

See `libs/bsw/middleware/tools/cpp_generator/README.md` for full generator
documentation.

### Tests

The FooMethod tests use the real generated proxy, skeleton and queues, with an
in-process memory layout and a deterministic clock. They cover exact array
values and overflow, polling states, busy handling, response storage lifetime,
send failures, response errors, timeout/late responses and deinitialization.
They do not require a running referenceApp or real-time sleeps.

The PduAccess tests run the PDU code generated from the same deployment: the
table is sorted by ID, `store` rejects payloads of the wrong size, `load` and
`store` round-trip, the broker rejects unknown IDs and transmits committed PDUs
in commit order through the real PDU id queue.

```bash
cmake --preset tests-posix-debug -B build/middleware-tests
cmake --build build/middleware-tests \
  --target middlewareSimulationFooMethodTest middlewareRequestResponseTest \
           middlewareClusterConnectionTest
ctest --test-dir build/middleware-tests/libs/bsw/middleware/test -C Debug \
  -R 'FooMethodTest|PduAccessTest|ProxyMethod|ProxyFireAndForget|ResponseBuffer|ConfigurationBaseTest|ConnectionBaseTest' \
  --output-on-failure
```

The corresponding Bazel target is
`//libs/bsw/middleware/simulation:middleware_simulation_foo_method_test` in the
`unit_test` build variant.

Error responses are routed to the requesting proxy just like successful
responses. Dispatcher regression tests cover this routing, unknown proxy
addresses and rejection of error responses by the skeleton endpoint.
