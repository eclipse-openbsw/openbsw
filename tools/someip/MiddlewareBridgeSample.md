<!--
 *******************************************************************************
  Copyright (c) 2026 KPIT Group

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Middleware SOME/IP Bridge Sample

The referenceApp carries three members of service `0x0101` from the FLYNC model in
`flync-configuration/flync` over SOME/IP with the middleware SOME/IP bridge
(`libs/bsw/middleware/bindings/someip`):

| Member | Kind | Id |
|---|---|---|
| `echoUINT8` | method | `0x0008` |
| `TestFieldUINT8` | field: getter, setter, notifier | `0x0026`, `0x0027`, `0x8006` |
| `TestEventUINT8Multicast` | multicast event | `0x800B` |

The service, its datatypes and their SOME/IP codecs are generated from the model;
the application code is in `executables/referenceApp/application/include/middleware`
and `src/middleware`, next to Foo's wrappers:

| File | Role |
|---|---|
| `SampleProvider.h` | this node's instance, on the generated skeleton |
| `SampleConsumer.h` | the peer's instance, through the generated proxy |
| `SampleSomeIp.h/.cpp` | the deployment wiring: ids, instances, port, SD timing, multicast group, which members go on SOME/IP, and which cluster connection the bridge takes over, on the stack `SomeIpSystem` owns |

Requests for any other member of the service are rejected by the stack.

Each node runs the application the FLYNC model places on its ECU:

| Build | ECU in the model | Provides | Consumes | Cluster |
|---|---|---|---|---|
| POSIX | `high_performance_compute` | instance 1 | instance 2 | `Cluster2` |
| S32K148 | `zonal_platform1` | instance 2 | instance 1 | `Cluster3` |

## Building

The sample is built when `PLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE` is on, which
`platforms/posix/Options.cmake` sets for POSIX. The middleware code is then
generated from `middlewareConfiguration/model/deployment.yaml` and the FLYNC
model together, merged by `merge_deployment.py`. For S32K148 it is off; turn it on
with `-DPLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE=ON` when configuring.

## Network layout (POSIX)

| Side | IP | Port |
|---|---|---|
| referenceApp (`eth0::IP_ADDRESS`) | `192.168.0.201` | service `30555`, SD `30490` |
| Host (`tap0`) | `192.168.0.10` | any |
| SD multicast | `225.0.0.1` | `30490` |
| Event multicast | `239.2.41.16` | `30555` |

## Running

Bring up `tap0` once per session:

```bash
sudo tools/enet/bring-up-ethernet.sh
```

Start the referenceApp:

```bash
build/posix-freertos/executables/referenceApp/application/Release/app.referenceApp.elf
```

It logs which instance it provides, then each request it serves and each answer
and event its consumer receives:

```
RefApp: DEMO: INFO: [MiddlewareSample] providing instance 1, consuming instance 2, on UDP port 30555
RefApp: DEMO: INFO: [MiddlewareSample] echoUINT8 request, value=42
RefApp: DEMO: INFO: [MiddlewareSample] echoUINT8 response, value=1
```

Its consumer only sends requests once SD has found the peer's instance 2.

## Testing

`test/pyTest/middleware_someip` tests both directions: it uses the sample's
instance 1 from the host, and provides instance 2 for the sample to consume. It
starts the referenceApp itself:

```bash
cd test/pyTest
pytest middleware_someip/ -s -v --target=posix --app=freertos
```

It prints a report, one line per check, and writes the same checks as JSON to
`test/pyTest/artifacts/middleware_someip_<app>_<time>.json`. See
`test/pyTest/middleware_someip/README.md`.
