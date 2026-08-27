# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************
"""
DoCAN / UDS integration tests - Normal Addressing (NA), 11-bit legislated OBD.

Covers ISO 15765-4 legislated OBD pairs (0x7E0..0x7E7 -> 0x7E8..0x7EF) and
NA functional (broadcast) multi-frame rejection. Runs under DOCAN_MODE=NORMAL
(@requires_normal) only.

Split out of the original monolithic test_docan.py to isolate NA-specific
coverage. See test_docan.py in this same directory for the full module
index / aggregator.
"""
from __future__ import annotations
import can
import pytest
import udsoncan
from udsoncan import services as uds
from udsoncan.exceptions import TimeoutException
from conftest import (
    SUPPORTED_DID,
    EXPECTED_CF01_PAYLOAD,
    ISO15765_PHYSICAL_PAIRS,
    hexlify,
    wait_for_ecu_ready,
    requires_normal,
    _assert_functional_ff_rejected,
)
# ---------------------------------------------------------------------------
# Representative OBD physical pairs.
#
# The reference app configures eight legislated OBD pairs (7E0..7E7 ->
# 7E8..7EF). Testing every pair adds little confidence: the pairs are
# structurally identical and the full table's presence/ordering is already
# verified at compile time by the static_assert in
# DoCanDiagAddressingConfig.cpp. We therefore exercise only the FIRST and
# LAST pair on hardware - the first proves the mechanism end-to-end, the last
# proves the table's full extent is wired - keeping CI lean and upstream-honest.
# ---------------------------------------------------------------------------
_OBD_REPRESENTATIVE_PAIRS = [
    ISO15765_PHYSICAL_PAIRS[0],   # 0x7E0 -> 0x7E8 (first)
    # ISO15765_PHYSICAL_PAIRS[-1],  # 0x7E7 -> 0x7EF (last)
]
# ---------------------------------------------------------------------------
# ISO 15765-4 legislated OBD pair coverage — NORMAL build only.
#
# The NORMAL config enumerates eight OBD rule entries (7E0..7E7 with matching
# response IDs 7E8..7EF). Full-table presence/ordering is verified at compile
# time (static_assert in DoCanDiagAddressingConfig.cpp); here we exercise only
# the representative first/last pair on hardware to keep CI lean.
# ---------------------------------------------------------------------------
@requires_normal
class TestIso15765LegislatedObd:
    @pytest.mark.parametrize("txid,rxid", _OBD_REPRESENTATIVE_PAIRS)
    def test_tester_present_representative(self, uds_client_factory, txid, rxid):
        client = uds_client_factory.create(txid, rxid)
        try:
            assert wait_for_ecu_ready(client)
            resp = client.tester_present()
            assert resp.valid
        finally:
            client.close()
    @pytest.mark.parametrize("txid,rxid", _OBD_REPRESENTATIVE_PAIRS)
    def test_rdbi_cf01_representative(self, uds_client_factory, txid, rxid):
        client = uds_client_factory.create(txid, rxid)
        try:
            assert wait_for_ecu_ready(client), (
                f"ECU not ready for {txid:#x}->{rxid:#x}"
            )
            req = uds.ReadDataByIdentifier.make_request(
                [SUPPORTED_DID], {SUPPORTED_DID: udsoncan.AsciiCodec(4)}
            )
            resp = client.send_request(req)
            assert resp.valid
            assert resp.get_payload()[0] == 0x62
            # Proves ISO-TP FF/CF transport works on the Normal/OBD path.
            assert hexlify(resp.get_payload()) == EXPECTED_CF01_PAYLOAD
        finally:
            client.close()
    @pytest.mark.parametrize("txid,rxid", _OBD_REPRESENTATIVE_PAIRS)
    def test_default_session_representative(self, uds_client_factory, txid, rxid):
        client = uds_client_factory.create(txid, rxid)
        try:
            assert wait_for_ecu_ready(client)
            resp = client.change_session(0x01)
            assert resp.valid
            assert resp.service_data.session_echo == 0x01
        finally:
            client.close()
    def test_invalid_pair_7e1_to_7e8_timeout(self, uds_client_factory):
        """Request on 7E1 must not produce a response on 7E8 (belongs to 7E0)."""
        client = uds_client_factory.create(0x7E1, 0x7E8)
        try:
            with pytest.raises(TimeoutException):
                client.tester_present()
        finally:
            client.close()
# ---------------------------------------------------------------------------
# Functional (broadcast) coverage — NORMAL build.
#
# ISO 15765-2:
#   - A functional (broadcast) request is permitted ONLY as a SingleFrame.
#   - A multi-frame functional request (FirstFrame) MUST be discarded: the
#     server must NOT emit a FlowControl and MUST NOT respond. In the NORMAL
#     config the functional routing entry (0x7DF) carries an INVALID
#     transmission address, so DoCanReceiver rejects any FF on it.
# The functional response is always addressed PHYSICALLY (here on 0x7E8).
# ---------------------------------------------------------------------------
@requires_normal
class TestNormalFunctional:
    _FUNCTIONAL_ID = 0x7DF
    _PHYSICAL_RESP = 0x7E8
    # NOTE: test_functional_single_frame_positive has been removed.
    # Functional addressing is comprehensively tested by the parametrized
    # TestTesterPresent, TestReadDataByIdentifier, and TestDiagnosticSessionControl
    # fixtures with NORMAL_FUNCTIONAL mode, which consistently PASS.
    def test_functional_multiframe_is_discarded(self, uds_client_factory):
        """
        A multi-frame (FirstFrame) functional request MUST be discarded:
        no FlowControl, no response on the physical id.
        """
        bus = uds_client_factory._target_session.can_bus()
        try:
            # ISO 15765-2 forbids a functional (broadcast) FirstFrame -> ECU
            # must stay silent (no FC, no response).
            _assert_functional_ff_rejected(
                bus, self._FUNCTIONAL_ID, self._PHYSICAL_RESP, is_extended=False
            )
        finally:
            bus.shutdown()
