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
# udsTool helper (raw command, mirrors test_udsToolRDBI pattern)
# ---------------------------------------------------------------------------
def _uds_raw_cmd(channel, payload_hex):
    config = os.path.join(os.getcwd(), "../../tools/UdsTool/app/canConfig.json")
    return (
        f"python3 ../../tools/UdsTool/udsTool.py raw --can --channel {channel} "
        f"--txid 0x7E0 --rxid 0x7E8 --data {payload_hex} --config {config}"
    )


# ---------------------------------------------------------------------------
# SID 0x85 — ControlDTCSetting
# Sub-function 0x01 = DTC setting ON
# Sub-function 0x02 = DTC setting OFF
# Positive response SID = 0xC5
# ---------------------------------------------------------------------------

# Test: Switch DTC setting OFF then ON; both shall return positive response 0xC5
@pytest.mark.parametrize(
    "subfunction,expected_hex",
    [
        (0x01, "c5 01"),   # dtcSettingOn
        (0x02, "c5 02"),   # dtcSettingOff
    ],
)
def test_control_dtc_setting_positive(target_session, subfunction, expected_hex, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # Switch to Extended session first (0x85 requires ExtendedDiagnostic)
    req_session = uds.DiagnosticSessionControl().make_request(0x03)
    uds_client.send_request(req_session)

    # Build raw request: 85 <subfunction>
    request_payload = bytes([0x85, subfunction])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload[:2]) == expected_hex


# Test: Sub-function 0x00 is not defined — shall return NRC 0x12 (subfunctionNotSupported)
def test_control_dtc_setting_invalid_subfunction(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    req_session = uds.DiagnosticSessionControl().make_request(0x03)
    uds_client.send_request(req_session)

    request_payload = bytes([0x85, 0x00])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # NRC frame: 7F 85 12
    assert hexlify(payload) == "7f 85 12"


# Test: Request too short (no sub-function byte) — shall return NRC 0x13 (incorrectMessageLengthOrInvalidFormat)
def test_control_dtc_setting_too_short(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    req_session = uds.DiagnosticSessionControl().make_request(0x03)
    uds_client.send_request(req_session)

    request_payload = bytes([0x85])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "7f 85 13"


# Test: udsTool raw — DTC setting OFF (0x85 0x02) shall return PositiveResponse
def test_control_dtc_setting_off_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    # First switch to Extended session via raw
    session_cmd = _uds_raw_cmd(channel, "1003")
    run_process(session_cmd, "PositiveResponse", timeout=5)

    cmd = _uds_raw_cmd(channel, "8502")
    output = run_process(cmd, "PositiveResponse", timeout=5)
    assert "PositiveResponse" in output


# Test: udsTool raw — DTC setting ON (0x85 0x01) shall return PositiveResponse
def test_control_dtc_setting_on_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    session_cmd = _uds_raw_cmd(channel, "1003")
    run_process(session_cmd, "PositiveResponse", timeout=5)

    cmd = _uds_raw_cmd(channel, "8501")
    output = run_process(cmd, "PositiveResponse", timeout=5)
    assert "PositiveResponse" in output


if __name__ == "__main__":
    pytest.main()
