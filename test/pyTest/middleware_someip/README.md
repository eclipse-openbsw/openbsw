<!--
 *******************************************************************************
  Copyright (c) 2026 KPIT Group

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Middleware SOME/IP bridge sample tests

These tests verify the middleware SOME/IP bridge sample in the referenceApp
(`PLATFORM_SUPPORT_SOMEIP_BRIDGE_SAMPLE`, on for POSIX). The sample provides
instance 1 of service `0x0101` from the FLYNC model on UDP `30555` and consumes
instance 2. The tests use instance 1 from this host (`SampleClient`) and provide
instance 2 to it (`Peer`), both in `someip_sample.py`.

## What is tested

| Test | Description |
|------|-------------|
| `test_sample_started` | The sample comes up: provides instance 1, consumes instance 2 |
| `test_echo_uint8` | `echoUINT8` is answered by the generated skeleton through the bridge |
| `test_field_getter` | The `TestFieldUINT8` getter answers with the initial value |
| `test_field_setter` | The setter stores the value; the getter then returns it |
| `test_eventgroup_subscription` | A SubscribeEventgroup for `eg2` is acknowledged |
| `test_field_notification` | A subscriber is notified when the field is set |
| `test_multicast_event` | `TestEventUINT8Multicast` arrives on `239.2.41.16`, counting up |
| `test_consumer_uses_peer` | The sample's proxy finds instance 2, its echo, getter and setter are answered, and it receives the peer's notification and multicast event |

On other targets the tests are skipped: the sample is built for POSIX only.

## Running

The target network must be up (`sudo tools/enet/bring-up-ethernet.sh`).

```bash
cd test/pyTest
pytest middleware_someip/ -s -v --target=posix --app=freertos
```

## Report

At the end the tests print a report, one line per check:

```
Middleware SOME/IP bridge sample
  [PASS] sample starts on UDP 30555                               provides instance 1, consumes instance 2
  [PASS] echoUINT8 (0x0008) round-trip                            echoed 0x42
  ...

12 passed, 0 failed
  results written to artifacts/middleware_someip_freertos_<time>.json
```

The same checks are written as JSON to `artifacts/middleware_someip_<app>_<time>.json`:

```json
{
  "dut": {"host": "192.168.0.201", "port": 30555, "app": "freertos"},
  "checks": [
    {"name": "echo_uint8", "description": "echoUINT8 (0x0008) round-trip",
     "outcome": "pass", "detail": "echoed 0x42"}
  ]
}
```
