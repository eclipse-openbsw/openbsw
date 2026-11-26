# *******************************************************************************
# Copyright (c) 2026 KPIT Technologies Ltd
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import pytest
import udsoncan
import udsoncan.services as uds
from helpers.helper_functions import hexlify, run_process
import os


# ---------------------------------------------------------------------------
# udsTool helper
# ---------------------------------------------------------------------------
def _uds_raw_cmd(channel, payload_hex):
    config = os.path.join(os.getcwd(), "../../tools/UdsTool/app/canConfig.json")
    return (
        f"python3 ../../tools/UdsTool/udsTool.py raw --can --channel {channel} "
        f"--txid 0x7E0 --rxid 0x7E8 --data {payload_hex} --config {config}"
    )


# ---------------------------------------------------------------------------
# DID 0xF186 — ActiveDiagnosticInformation (ReadActiveSessionDiagnosticInformation)
# Accessed via SID 0x22 (RDBI)
# Positive response: 0x62 0xF1 0x86 <session byte>
#   session byte: 0x01 = Default, 0x02 = Programming, 0x03 = Extended
# ---------------------------------------------------------------------------

# Test: Read DID 0xF186 in Default session — response shall contain session byte 0x01
def test_read_active_session_default(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Ensure we are in Default session
    req_session = uds.DiagnosticSessionControl().make_request(0x01)
    uds_client.send_request(req_session)

    request_payload = bytes([0x22, 0xF1, 0x86])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # Response: 62 F1 86 01
    assert hexlify(payload) == "62 f1 86 01"


# Test: Read DID 0xF186 in Extended session — response shall contain session byte 0x03
def test_read_active_session_extended(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    req_session = uds.DiagnosticSessionControl().make_request(0x03)
    uds_client.send_request(req_session)

    request_payload = bytes([0x22, 0xF1, 0x86])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "62 f1 86 03"


# Test: Programming session ($10 0x02) from Default session.
# Without PLATFORM_SUPPORT_PROGRAMMING_SESSION, Default→Programming returns NRC 0x7E.
# With it defined, the transition succeeds (0x50 response) and 0xF186 returns 0x02.
def test_read_active_session_programming(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Always start from Default session to make test independent of prior state
    uds_client.conn.send(bytes([0x10, 0x01]))
    uds_client.conn.wait_frame(timeout=2, exception=False)

    uds_client.conn.send(bytes([0x10, 0x02]))
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # From Default: NRC 0x7E (programming not supported) — positive response 0x50 not expected
    assert hexlify(payload) == "7f 10 7e"


# Test: Session changes are reflected immediately in subsequent 0xF186 reads
@pytest.mark.parametrize(
    "session_id,expected_session_byte",
    [
        (0x01, "01"),
        (0x03, "03"),
        (0x01, "01"),
    ],
)
def test_read_active_session_transitions(target_session, session_id, expected_session_byte, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Reset to Default first so this test is independent of prior session state
    uds_client.conn.send(bytes([0x10, 0x01]))
    uds_client.conn.wait_frame(timeout=2, exception=False)

    req_session = uds.DiagnosticSessionControl().make_request(session_id)
    uds_client.send_request(req_session)

    request_payload = bytes([0x22, 0xF1, 0x86])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == f"62 f1 86 {expected_session_byte}"


# Test: udsTool read — DID F186 in Default session shall return PositiveResponse with 0x01
def test_read_active_session_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    config = os.path.join(os.getcwd(), "../../tools/UdsTool/app/canConfig.json")
    cmd = (
        f"python3 ../../tools/UdsTool/udsTool.py read --can --channel {channel} "
        f"--txid 0x7E0 --rxid 0x7E8 --did F186 --config {config}"
    )
    output = run_process(cmd, "PositiveResponse", timeout=5)
    lines = output.strip().splitlines()
    assert any("PositiveResponse" in line for line in lines), "Missing PositiveResponse"
    # Response hex must contain 62f186
    assert any("62f186" in line.replace(" ", "") for line in lines), "DID 0xF186 not in response"


if __name__ == "__main__":
    pytest.main()
