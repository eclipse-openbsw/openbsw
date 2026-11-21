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
Integration tests for the middleware PDU broker running inside the referenceApp.

The DemoSystem writes a PDU every 1 s. The PDU broker transmits it to a loopback bus,
which returns it as a different PDU that the application reads:

    <ts>: RefApp: DEMO: INFO: [Middleware] PDU written, counter=<N>
    <ts>: RefApp: DEMO: INFO: [Middleware] PDU transmitted, id=<ID>
    <ts>: RefApp: DEMO: INFO: [Middleware] PDU received, counter=<N>
"""

import re

# A PDU is written every 1 s; the app start is sometimes delayed by several seconds.
_PDU_TIMEOUT = 15


def _extract_counter(lines: list[bytes]) -> int | None:
    """Parse counter=<N> out of the last line that has one."""
    for line in reversed(lines):
        m = re.search(rb"counter=(\d+)", line)
        if m:
            return int(m.group(1))
    return None


def test_pdu_transmitted(target_session):
    """The PDU broker transmits a committed PDU to the bus."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, _, _ = capserial.read_until(
        b"[Middleware] PDU transmitted", timeout=_PDU_TIMEOUT
    )
    assert success, "Timed out waiting for a transmitted PDU"


def test_pdu_received(target_session):
    """A PDU received from the bus is stored and can be read by the application."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, _, _ = capserial.read_until(
        b"[Middleware] PDU received", timeout=_PDU_TIMEOUT
    )
    assert success, "Timed out waiting for a received PDU"


def test_pdu_received_value_matches_written_value(target_session):
    """The counter that is read back is the counter that was written."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    success, lines, _ = capserial.read_until(
        b"[Middleware] PDU written", timeout=_PDU_TIMEOUT
    )
    assert success, "Timed out waiting for a written PDU"
    written = _extract_counter(lines)
    assert written is not None

    success, lines, _ = capserial.read_until(
        b"[Middleware] PDU received", timeout=_PDU_TIMEOUT
    )
    assert success, "Timed out waiting for a received PDU"
    assert _extract_counter(lines) == written


def test_pdu_value_increments(target_session):
    """The received counter increases from one PDU to the next."""
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()

    values = []
    for _ in range(2):
        success, lines, _ = capserial.read_until(
            b"[Middleware] PDU received", timeout=_PDU_TIMEOUT
        )
        assert success, "Timed out waiting for a received PDU"
        values.append(_extract_counter(lines))

    assert None not in values, f"Could not extract counter, got: {values}"
    assert values[1] > values[0], f"counter did not increment: {values}"
