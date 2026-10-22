# *******************************************************************************
# Copyright (c) 2026 KPIT Group
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Integration tests for the middleware SOME/IP bridge sample in the referenceApp.

The sample carries three members of service 0x0101 from the FLYNC model over
SOME/IP: the echoUINT8 method, the TestFieldUINT8 field and the
TestEventUINT8Multicast event. It provides instance 1 and consumes instance 2.
The tests use instance 1 from this host and provide instance 2 to it, checking
both on the wire and in the referenceApp's log:

    <ts>: RefApp: DEMO: INFO: [MiddlewareSample] providing instance 1, consuming instance 2, ...
    <ts>: RefApp: DEMO: INFO: [MiddlewareSample] echoUINT8 response, value=<N>

Every check is recorded, and at the end the module prints a report and writes it
to artifacts/middleware_someip_<app>_<time>.json.
"""

import json
import os
from datetime import datetime

import pytest

from someip_sample import (
    ECHO_UINT8,
    GET_TEST_FIELD_UINT8,
    Peer,
    SampleClient,
    SERVICE_PORT,
    SET_TEST_FIELD_UINT8,
    TEST_EVENT_UINT8_MULTICAST,
    TEST_FIELD_UINT8_NOTIFIER,
)

_STARTED = b"[MiddlewareSample] providing instance 1, consuming instance 2"
_START_TIMEOUT = 3
_LOG_TIMEOUT = 1
_PEER_TIMEOUT = 8

_checks = []
_dut = {}


def _check(name, description, ok, detail):
    """Record one check for the report, and fail the test if it did not pass."""
    _checks.append(
        {
            "name": name,
            "description": description,
            "outcome": "pass" if ok else "fail",
            "detail": detail,
        }
    )
    assert ok, f"{description}: {detail}"


@pytest.fixture(scope="module", autouse=True)
def report(request):
    """Print the checks as a report and store it, once the module has run."""
    yield
    if not _checks:
        return
    passed = sum(1 for check in _checks if check["outcome"] == "pass")
    print("\nMiddleware SOME/IP bridge sample")
    for check in _checks:
        outcome = "PASS" if check["outcome"] == "pass" else "FAIL"
        print(f"  [{outcome}] {check['description']:<56} {check['detail']}")
    print(f"\n{passed} passed, {len(_checks) - passed} failed")

    app = request.config.getoption("app")
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    path = os.path.join("artifacts", f"middleware_someip_{app}_{stamp}.json")
    os.makedirs("artifacts", exist_ok=True)
    with open(path, "w") as handle:
        json.dump({"dut": dict(_dut, app=app), "checks": _checks}, handle, indent=2)
    print(f"  results written to {path}")


def _sample_started(target_session):
    """Wait for the sample to come up; skip where it is not built."""
    if target_session.target_name != "posix":
        pytest.skip("the middleware SOME/IP bridge sample is built for POSIX only")
    _dut.update(host=target_session.target_ip_address(), port=SERVICE_PORT)
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()
    success, _, _ = capserial.read_until(_STARTED, timeout=_START_TIMEOUT)
    return success


def _logged(target_session, text, timeout=_LOG_TIMEOUT):
    success, _, _ = target_session.capserial().read_until(text, timeout=timeout)
    return success


def test_sample_started(target_session):
    """The sample provides instance 1 and consumes instance 2 on UDP 30555."""
    started = _sample_started(target_session)
    _check(
        "startup",
        "sample starts on UDP 30555",
        started,
        "provides instance 1, consumes instance 2" if started else "no startup line",
    )


def test_echo_uint8(target_session):
    """echoUINT8 is answered by the generated skeleton through the bridge."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        reply = client.call(ECHO_UINT8, bytes([0x42]))
    served = _logged(target_session, b"[MiddlewareSample] echoUINT8 request, value=66")
    _check(
        "echo_uint8",
        "echoUINT8 (0x0008) round-trip",
        reply == bytes([0x42]) and served,
        f"echoed 0x{reply.hex()}" if reply else "no payload",
    )


def test_field_getter(target_session):
    """The TestFieldUINT8 getter answers with the field's initial value."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        reply = client.call(GET_TEST_FIELD_UINT8)
    _check(
        "field_getter",
        "TestFieldUINT8 getter (0x0026)",
        reply == bytes([0]),
        f"value {reply[0]}" if reply else "no payload",
    )


def test_field_setter(target_session):
    """The TestFieldUINT8 setter stores the value; the getter then returns it."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        applied = client.call(SET_TEST_FIELD_UINT8, bytes([42]))
        stored = client.call(GET_TEST_FIELD_UINT8)
    _check(
        "field_setter",
        "TestFieldUINT8 setter (0x0027), then getter",
        applied == bytes([42]) and stored == bytes([42]),
        f"set answered {applied[0]}, get answered {stored[0]}",
    )


def test_eventgroup_subscription(target_session):
    """A SubscribeEventgroup for eg2 is acknowledged."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        acked = client.subscribe()
    _check(
        "subscribe_eg2",
        "SubscribeEventgroup eg2 acknowledged",
        acked,
        "ack received" if acked else "no ack",
    )


def test_field_notification(target_session):
    """A subscriber of eg2 is notified when TestFieldUINT8 is set."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        assert client.subscribe(), "no SubscribeEventgroupAck for eg2"
        client.send(SET_TEST_FIELD_UINT8, bytes([43]))
        received = client.collect(2.0)
    notified = [value for method, value in received if method == TEST_FIELD_UINT8_NOTIFIER]
    _check(
        "field_notification",
        "TestFieldUINT8 notifier (0x8006) after a set",
        43 in notified,
        f"notified {notified}" if notified else "no notification",
    )


def test_multicast_event(target_session):
    """TestEventUINT8Multicast arrives on the multicast group, counting up."""
    assert _sample_started(target_session)
    with SampleClient(target_session.target_ip_address()) as client:
        assert client.subscribe(), "no SubscribeEventgroupAck for eg2"
        received = client.collect(3.5)
    values = [value for method, value in received if method == TEST_EVENT_UINT8_MULTICAST]
    counting = len(values) >= 2 and all(b == (a + 1) % 256 for a, b in zip(values, values[1:]))
    _check(
        "multicast_event",
        "TestEventUINT8Multicast (0x800B) on 239.2.41.16",
        counting,
        f"received {values}" if values else "no event",
    )


def test_consumer_uses_peer(target_session):
    """The sample's proxy finds instance 2, calls it, and receives its events."""
    assert _sample_started(target_session)
    capserial = target_session.capserial()
    expected = {
        "consumer_echo": (
            "proxy echoUINT8 answered by the peer",
            b"[MiddlewareSample] echoUINT8 response, value=",
        ),
        "consumer_getter": (
            "proxy TestFieldUINT8 getter answered by the peer",
            b"[MiddlewareSample] TestFieldUINT8 get response, value=7",
        ),
        "consumer_setter": (
            "proxy TestFieldUINT8 setter answered by the peer",
            b"[MiddlewareSample] TestFieldUINT8 set response, value=",
        ),
        "consumer_notification": (
            "proxy receives the peer's TestFieldUINT8 notification",
            b"[MiddlewareSample] TestFieldUINT8 notified, value=",
        ),
        "consumer_multicast_event": (
            "proxy receives the peer's multicast event",
            b"[MiddlewareSample] TestEventUINT8Multicast received, value=101",
        ),
    }
    peer = Peer(target_session.target_ip_address())
    peer.start()
    try:
        _, lines, found_at = capserial.read_until(
            [pattern for _, pattern in expected.values()], timeout=_PEER_TIMEOUT
        )
    finally:
        peer.stop()

    failed = []
    for name, (description, pattern) in expected.items():
        index = found_at.get(pattern)
        ok = index is not None
        detail = lines[index].decode(errors="replace").split("] ", 1)[-1].strip() if ok else "not seen"
        _checks.append(
            {
                "name": name,
                "description": description,
                "outcome": "pass" if ok else "fail",
                "detail": detail,
            }
        )
        if not ok:
            failed.append(description)
    assert peer.subscribed, "the sample never subscribed to the peer's eg2"
    assert not failed, f"not seen: {failed}"
