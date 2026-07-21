# ******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# ******************************************************************************

"""
Integration tests for the middleware Foo service running inside the referenceApp.

The DemoSystem broadcasts FooDefault every 1 s and issues a getter every 2 s.
All middleware log messages go through util::logger (DEMO component) so they
appear in the captured serial output with the format:

    <ts>: RefApp: DEMO: INFO: [Middleware] Foo broadcast sent, fooValue=<N>
    <ts>: RefApp: DEMO: INFO: [Middleware] Foo attribute changed, fooValue=<N>
    <ts>: RefApp: DEMO: INFO: [Middleware] Foo get response, fooValue=<N>
"""

import re
import pytest


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

# Broadcast fires every 1 s — allow 3 s to be safe after a clean slate.
_BROADCAST_TIMEOUT = 3
# Getter fires every 2 s — allow 4 s.
_GETTER_TIMEOUT = 4
# Attribute change notification arrives on the same cycle as the broadcast.
_NOTIFY_TIMEOUT = 1


def _extract_foo_value(line: bytes) -> int | None:
    """Parse fooValue=<N> out of a log line."""
    m = re.search(rb"fooValue=(\d+)", line)
    return int(m.group(1)) if m else None


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

@pytest.mark.skip_if("target==['s32k148']", "Middleware Foo test not supported on s32k148")
def test_foo_broadcast_sent(target_session):
    """Cluster0 FooSkeleton sends at least one Foo broadcast after boot."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, _, _ = capserial.read_until(
        b"[Middleware] Foo broadcast sent", timeout=_BROADCAST_TIMEOUT
    )
    assert success, "Timed out waiting for Foo broadcast"

@pytest.mark.skip_if("target==['s32k148']", "Middleware Foo test not supported on s32k148")
def test_foo_attribute_notification(target_session):
    """Cluster1 FooProxy receives a change notification right after a broadcast."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, _, _ = capserial.read_until(
        b"[Middleware] Foo broadcast sent", timeout=_BROADCAST_TIMEOUT
    )
    assert success, "Timed out waiting for Foo broadcast"

    success, _, _ = capserial.read_until(
        b"[Middleware] Foo attribute changed", timeout=_NOTIFY_TIMEOUT
    )
    assert success, "Proxy did not receive attribute change notification after broadcast"

@pytest.mark.skip_if("target==['s32k148']", "Middleware Foo test not supported on s32k148")
def test_foo_getter_response(target_session):
    """Cluster1 FooProxy getter round-trip returns a response within expected time."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, _, _ = capserial.read_until(
        b"[Middleware] Foo get response", timeout=_GETTER_TIMEOUT
    )
    assert success, "Timed out waiting for Foo getter response"

@pytest.mark.skip_if("target==['s32k148']", "Middleware Foo test not supported on s32k148")
def test_foo_value_increments(target_session):
    """fooValue is strictly incremented across two consecutive broadcasts."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    values = []
    for _ in range(2):
        success, lines, _ = capserial.read_until(
            b"[Middleware] Foo broadcast sent", timeout=_BROADCAST_TIMEOUT
        )
        assert success, "Timed out waiting for Foo broadcast"
        for line in reversed(lines):
            v = _extract_foo_value(line)
            if v is not None:
                values.append(v)
                break

    assert len(values) == 2, f"Could not extract fooValue from broadcasts, got: {values}"
    assert values[1] > values[0], (
        f"fooValue did not increment: first={values[0]}, second={values[1]}"
    )

@pytest.mark.skip_if("target==['s32k148']", "Middleware Foo test not supported on s32k148")
def test_foo_broadcast_and_getter_values_match(target_session):
    """The fooValue in a getter response matches the most recent broadcast value."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    # Capture a broadcast value
    success, lines, _ = capserial.read_until(
        b"[Middleware] Foo broadcast sent", timeout=_BROADCAST_TIMEOUT
    )
    assert success, "Timed out waiting for Foo broadcast"
    broadcast_value = next(
        (_extract_foo_value(line) for line in reversed(lines) if b"broadcast" in line), None
    )
    assert broadcast_value is not None

    # The getter response should carry the same or a later (incremented) value
    success, lines, _ = capserial.read_until(
        b"[Middleware] Foo get response", timeout=_GETTER_TIMEOUT
    )
    assert success, "Timed out waiting for Foo getter response"
    getter_value = next(
        (_extract_foo_value(line) for line in reversed(lines) if b"get response" in line), None
    )
    assert getter_value is not None
    assert getter_value >= broadcast_value, (
        f"Getter value {getter_value} is older than broadcast value {broadcast_value}"
    )
