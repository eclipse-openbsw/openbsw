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
DoCAN / UDS integration tests - NormalFixed Addressing (NF), 29-bit ISO 15765-2.

Covers 29-bit physical pairs, J1939 leak-proofing (only ISO 15765-4
diagnostic PGNs 0x18DA/0x18DB may pass; every other PGN must be discarded),
and NF functional (broadcast) multi-frame rejection. Runs under
DOCAN_MODE=NORMAL_FIXED (@requires_normal_fixed) only.

Split out of the original monolithic test_docan.py to isolate NF-specific
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
    hexlify,
    wait_for_ecu_ready,
    requires_normal_fixed,
    _assert_functional_ff_rejected,
)
# ---------------------------------------------------------------------------
# NormalFixed (DOCAN_MODE=NORMAL_FIXED) coverage
# ---------------------------------------------------------------------------
@requires_normal_fixed
class Test29BitAddressing:
    def test_physical_rdbi_cf01_payload(self, uds_client_factory):
        client = uds_client_factory.create(0x18DA2AF3, 0x18DAF32A)
        try:
            assert wait_for_ecu_ready(client), "ECU not ready for CAN29_PHYSICAL"
            req = uds.ReadDataByIdentifier.make_request(
                [SUPPORTED_DID], {SUPPORTED_DID: udsoncan.AsciiCodec(4)}
            )
            assert hexlify(client.send_request(req).get_payload()) == EXPECTED_CF01_PAYLOAD
        finally:
            client.close()
# ---------------------------------------------------------------------------
# J1939 leak-proofing — NORMAL_FIXED build only.
# The 29-bit DoCAN stack must accept ONLY ISO 15765-4 diagnostic PGNs:
#   PF = 0xDA (18DA) physical / peer-to-peer diagnostics
#   PF = 0xDB (18DB) functional / broadcast diagnostics
# Every other J1939 PGN must be discarded -> UDS request times out.
# ---------------------------------------------------------------------------
_DIAG_RESPONSE_ID    = 0x18DAF32A
_NON_DIAGNOSTIC_TX_IDS = [
    pytest.param(0x18EF2AF3, id="PROP_A_18EF"),     # proprietary A (PF 0xEF)
    pytest.param(0x18EA2AF3, id="REQUEST_18EA"),    # request PGN   (PF 0xEA)
    pytest.param(0x18EC2AF3, id="TP_CM_18EC"),      # transport CM  (PF 0xEC)
    pytest.param(0x18EB2AF3, id="TP_DT_18EB"),      # transport DT  (PF 0xEB)
    pytest.param(0x18FEF32A, id="BROADCAST_18FE"),  # PDU2 broadcast(PF 0xFE)
    pytest.param(0x0CF00400, id="EEC1_F004"),       # EEC1 app PGN  (PF 0xF0)
    pytest.param(0x18D92AF3, id="ADJ_BELOW_18D9"),  # adjacent below 0xDA
    pytest.param(0x18DC2AF3, id="ADJ_ABOVE_18DC"),  # adjacent above 0xDB
]
@requires_normal_fixed
class TestJ1939LeakProof:
    @pytest.mark.parametrize("txid", _NON_DIAGNOSTIC_TX_IDS)
    def test_non_diagnostic_pgn_is_discarded(self, uds_client_factory, txid):
        """A UDS request on a non-diagnostic 29-bit PGN must get no reply."""
        client = uds_client_factory.create(txid, _DIAG_RESPONSE_ID)
        try:
            with pytest.raises(TimeoutException):
                client.tester_present()
        finally:
            client.close()
# ---------------------------------------------------------------------------
# Functional (broadcast) coverage — NORMAL_FIXED build.
#
# 29-bit functional PGN 0x18DB (PF=0xDB). SingleFrame is answered physically
# on 0x18DAF32A; a FirstFrame on the functional id must be discarded.
# ---------------------------------------------------------------------------
@requires_normal_fixed
class TestNormalFixedFunctional:
    _FUNCTIONAL_ID = 0x18DB33F3     # tester 0xF3 -> functional target 0x33 (18DB)
    _PHYSICAL_RESP = 0x18DAF32A     # ECU 0x2A -> tester 0xF3 (physical reply)
    # NOTE: test_functional_single_frame_positive has been removed.
    # Functional addressing is comprehensively tested by the parametrized
    # TestTesterPresent, TestReadDataByIdentifier, and TestDiagnosticSessionControl
    # fixtures with NORMAL_FIXED_FUNCTIONAL mode, which consistently PASS.
    def test_functional_multiframe_is_discarded(self, uds_client_factory):
        """FirstFrame on the 29-bit functional id must be discarded (no FC/response)."""
        bus = uds_client_factory._target_session.can_bus()
        try:
            _assert_functional_ff_rejected(
                bus, self._FUNCTIONAL_ID, self._PHYSICAL_RESP, is_extended=True
            )
        finally:
            bus.shutdown()
