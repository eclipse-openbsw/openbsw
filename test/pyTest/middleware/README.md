<!--
 *******************************************************************************
  Copyright (c) 2026 BMW AG

  This program and the accompanying materials are made available under the
  terms of the Apache License Version 2.0 which is available at
  https://www.apache.org/licenses/LICENSE-2.0

  SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
-->

# Middleware pytest tests

These tests verify the middleware Foo service integration in the referenceApp
(`PLATFORM_SUPPORT_MIDDLEWARE`).

## What is tested

| Test | Description |
|------|-------------|
| `test_foo_broadcast_sent` | Cluster0 FooSkeleton sends a broadcast within 1 s of boot |
| `test_foo_attribute_notification` | Cluster1 FooProxy receives the change notification immediately after a broadcast |
| `test_foo_getter_response` | Core1 FooProxy getter round-trip completes within 2 s |
| `test_foo_value_increments` | `fooValue` is strictly incremented across consecutive broadcasts |
| `test_pdu_transmitted` | The PDU broker transmits a committed PDU to the (loopback) bus |
| `test_pdu_received` | A PDU received from the bus is stored and read by the application |
| `test_pdu_received_value_matches_written_value` | The counter read back equals the counter that was written |
| `test_pdu_value_increments` | The received counter increases from one PDU to the next |

## Running

```bash
cd test/pyTest
pytest middleware/ --target=posix -v
```

The `posix` target starts
`build/posix-threadx/executables/referenceApp/application/Release/app.referenceApp.elf`
and captures its output.  Broadcasts are emitted every 1 s and getters every 2 s
so the tests complete quickly.
