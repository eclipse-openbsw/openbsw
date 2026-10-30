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
# SID 0x19 — ReadDTCInformation
# Positive response SID = 0x59
# Sub-functions:
#   0x02 = ReportDTCByStatusMask
#   0x04 = ReportDTCSnapshotRecordByDTCNumber
#   0x06 = ReportDTCExtendedDataRecordByDTCNumber
#   0x0A = ReportSupportedDTCs
# ---------------------------------------------------------------------------

# Test: 0x19 0x02 <mask> — ReportDTCByStatusMask shall return 0x59 positive response
@pytest.mark.parametrize("status_mask", [0xFF, 0x09, 0x08])
def test_read_dtc_by_status_mask_positive(target_session, status_mask, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x19, 0x02, status_mask])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # First byte of positive response must be 0x59
    assert payload[0] == 0x59
    # Second byte echoes sub-function
    assert payload[1] == 0x02


# Test: 0x19 0x04 <DTC 3 bytes> <snapshot record 0xFF> — ReportDTCSnapshotRecordByDTCNumber
# With an empty FaultManager (no faults reported), DTC 0x123456 does not exist.
# Expected: NRC 0x31 (requestOutOfRange) — correct per ISO 14229-1 §11.3.4
def test_read_dtc_snapshot_positive(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    # DTC 0x123456, record number 0xFF (all records)
    request_payload = bytes([0x19, 0x04, 0x12, 0x34, 0x56, 0xFF])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # DTC not in FaultManager (no faults set) → NRC 0x31 requestOutOfRange
    assert hexlify(payload) == "7f 19 31"


# Test: 0x19 0x06 <DTC 3 bytes> <extended record 0xFF> — ReportDTCExtendedDataRecordByDTCNumber
# With an empty FaultManager (no faults reported), DTC 0x123456 does not exist.
# Expected: NRC 0x31 (requestOutOfRange) — correct per ISO 14229-1 §11.3.6
def test_read_dtc_extended_data_positive(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x19, 0x06, 0x12, 0x34, 0x56, 0xFF])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    # DTC not in FaultManager (no faults set) → NRC 0x31 requestOutOfRange
    assert hexlify(payload) == "7f 19 31"


# Test: 0x19 0x0A — ReportSupportedDTCs shall return 0x59 0x0A
def test_read_supported_dtcs_positive(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x19, 0x0A])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert payload[0] == 0x59
    assert payload[1] == 0x0A


# Test: 0x19 0x01 — unsupported sub-function shall return NRC 0x12
def test_read_dtc_unsupported_subfunction(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x19, 0x01])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "7f 19 12"


# Test: Request too short (only SID, no sub-function) — NRC 0x13
def test_read_dtc_too_short(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    request_payload = bytes([0x19])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert hexlify(payload) == "7f 19 13"


# Test: udsTool raw — ReportDTCByStatusMask (0x19 0x02 0xFF) shall return PositiveResponse
def test_read_dtc_by_status_mask_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    cmd = _uds_raw_cmd(channel, "1902ff")
    output = run_process(cmd, "PositiveResponse", timeout=5)
    lines = output.strip().splitlines()
    assert any("PositiveResponse" in line for line in lines), "Missing PositiveResponse"
    # Response must start with 59 02
    assert any("5902" in line.replace(" ", "") for line in lines), "Response SID/SF mismatch"


# Test: udsTool raw — ReportSupportedDTCs (0x19 0x0A) shall return PositiveResponse
def test_read_supported_dtcs_udstool(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    channel = target_session.target_info.socketcan["channel"]
    cmd = _uds_raw_cmd(channel, "190a")
    output = run_process(cmd, "PositiveResponse", timeout=5)
    assert "PositiveResponse" in output


if __name__ == "__main__":
    pytest.main()
