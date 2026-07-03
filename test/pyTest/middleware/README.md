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
| `test_foo_broadcast_sent` | Core0 FooSkeleton sends a broadcast within 1 s of boot |
| `test_foo_attribute_notification` | Core1 FooProxy receives the change notification immediately after a broadcast |
| `test_foo_getter_response` | Core1 FooProxy getter round-trip completes within 2 s |
| `test_foo_value_increments` | `fooValue` is strictly incremented across consecutive broadcasts |

## Running

```bash
cd test/pyTest
pytest middleware/ --target=posix -v
```

The `posix` target starts
`build/posix-threadx/executables/referenceApp/application/Release/app.referenceApp.elf`
and captures its output.  Broadcasts are emitted every 1 s and getters every 2 s
so the tests complete quickly.
