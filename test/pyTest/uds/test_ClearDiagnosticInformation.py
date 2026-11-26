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
# SID 0x14 — ClearDiagnosticInformation
# Positive response SID = 0x54  (no payload beyond SID)
# GROUP_ALL_DTCS = 0xFF 0xFF 0xFF
# ---------------------------------------------------------------------------

# Test: 0x14 0xFF 0xFF 0xFF — clear all DTCs shall return positive response 0x54
def test_clear_all_dtcs_positive(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Switch to Extended session — required by the implementation
    req_session = uds.DiagnosticSessionControl().make_request(0x03)
    uds_client.send_request(req_session)

    request_payload = bytes([0x14, 0xFF, 0xFF, 0xFF])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # Positive response for 0x14 is simply 0x54
    assert payload[0] == 0x54


# Test: 0x14 0xFF 0xFF 0xFF can be sent in Default session too (no session restriction in spec)
def test_clear_all_dtcs_default_session(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x14, 0xFF, 0xFF, 0xFF])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert payload[0] == 0x54


# Test: Group identifier other than GROUP_ALL_DTCS (0xFF 0xFF 0xFF)
#       If not supported, ECU shall return NRC 0x31 (requestOutOfRange)
def test_clear_specific_group_nrc(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Group 0x00 0x00 0x00 — not GROUP_ALL_DTCS
    request_payload = bytes([0x14, 0x00, 0x00, 0x00])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "7f 14 31"


# Test: Request too short (missing group bytes) — NRC 0x13
def test_clear_dtc_too_short(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x14, 0xFF])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "7f 14 13"


# Test: udsTool raw — clear all DTCs (0x14 0xFF 0xFF 0xFF) shall return PositiveResponse
def test_clear_all_dtcs_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    # Enter Extended session first
    session_cmd = _uds_raw_cmd(channel, "1003")
    run_process(session_cmd, "PositiveResponse", timeout=5)

    cmd = _uds_raw_cmd(channel, "14ffffff")
    output = run_process(cmd, "PositiveResponse", timeout=5)
    lines = output.strip().splitlines()
    assert any("PositiveResponse" in line for line in lines), "Missing PositiveResponse"
    # Response payload must be exactly 54
    assert any("54" in line for line in lines), "Expected 0x54 positive response SID"


if __name__ == "__main__":
    pytest.main()
