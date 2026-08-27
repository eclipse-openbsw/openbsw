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
DoCAN / UDS integration tests - Classic Extended Addressing (EA), 11-bit.

Covers the fixed single CAN-ID pair (0x600/0x601) with NL_TA target-address
byte demo pairing. Runs under DOCAN_MODE=EXTENDED (@requires_extended) only.
For the formula-based Range Extended Addressing scheme, see
test_docan_ea_range.py instead.

Split out of the original monolithic test_docan.py to isolate EA-specific
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
    EA_RX_ID,
    EA_TX_ID,
    EA_NL_TA,
    EA_OUT_OF_RANGE_ID,
    hexlify,
    wait_for_ecu_ready,
    requires_extended,
)
# ---------------------------------------------------------------------------
# Extended11 (DOCAN_MODE=EXTENDED) coverage
# Demo pairing: rxId=0x600, txId=0x601, NL_TA=0x2A.
# ---------------------------------------------------------------------------
@requires_extended
class TestExtended11Addressing:
    """
    Confirmed scenarios (from architecture handoff):
      Positive 1: TesterPresent on 0x600 with NL_TA=0x2A.
      Positive 2: RDBI CF01 on 0x600 with NL_TA=0x2A.
      Positive 3: DiagnosticSessionControl 0x01 on 0x600 with NL_TA=0x2A.
      Negative 1: Wrong NL_TA on 0x600 -> no response.
      Negative 2: CAN ID outside [0x600, 0x6FF] -> no response.
      Negative 3: Correct CAN ID but no payload byte -> no response.
    """
    # ---- Positive cases -------------------------------------------------
    def test_positive_tester_present(self, uds_client_factory):
        client = uds_client_factory.create(
            EA_RX_ID, EA_TX_ID, target_address=EA_NL_TA
        )
        try:
            assert wait_for_ecu_ready(client), (
                "ECU not ready for Extended11 physical demo pairing "
                f"({EA_RX_ID:#x}/{EA_TX_ID:#x}, NL_TA={EA_NL_TA:#x})"
            )
            resp = client.tester_present()
            assert resp.valid
        finally:
            client.close()
    def test_positive_rdbi_cf01(self, uds_client_factory):
        client = uds_client_factory.create(
            EA_RX_ID, EA_TX_ID, target_address=EA_NL_TA
        )
        try:
            assert wait_for_ecu_ready(client)
            req = uds.ReadDataByIdentifier.make_request(
                [SUPPORTED_DID], {SUPPORTED_DID: udsoncan.AsciiCodec(4)}
            )
            resp = client.send_request(req)
            assert resp.valid
            assert resp.get_payload()[0] == 0x62
            # Proves ISO-TP FF/CF transport works on the Extended path.
            assert hexlify(resp.get_payload()) == EXPECTED_CF01_PAYLOAD
        finally:
            client.close()
    def test_positive_default_session(self, uds_client_factory):
        client = uds_client_factory.create(
            EA_RX_ID, EA_TX_ID, target_address=EA_NL_TA
        )
        try:
            assert wait_for_ecu_ready(client)
            resp = client.change_session(0x01)
            assert resp.valid
            assert resp.service_data.session_echo == 0x01
        finally:
            client.close()
    # ---- Negative cases -------------------------------------------------
    def test_negative_wrong_nl_ta(self, uds_client_factory):
        """NL_TA byte != configured target => filter must drop the frame."""
        wrong_nl_ta = (EA_NL_TA + 1) & 0xFF
        assert wrong_nl_ta != EA_NL_TA, "test setup error: NL_TA collision"
        client = uds_client_factory.create(
            EA_RX_ID, EA_TX_ID, target_address=wrong_nl_ta
        )
        client.empty_rxqueue()          # <-- drain any stale response from a prior test
        try:
            with pytest.raises(TimeoutException):
                client.tester_present()
        finally:
            client.close()
    def test_negative_can_id_outside_reserved_range(self, uds_client_factory):
        """CAN ID outside [0x600, 0x6FF] must be rejected by IntervalFilter."""
        # Bypass the width-based factory route by passing target_address so
        # Extended_11bits mode is used with an ID the ECU does not accept.
        client = uds_client_factory.create(
            EA_OUT_OF_RANGE_ID, EA_OUT_OF_RANGE_ID + 1, target_address=EA_NL_TA
        )
        try:
            with pytest.raises(TimeoutException):
                client.tester_present()
        finally:
            client.close()
    def test_negative_zero_length_payload_dropped(self, uds_client_factory):
        """
        A frame with no payload byte cannot carry NL_TA. The filter must
        reject it. Simulated by sending a raw CAN frame with an empty
        payload directly on the bus and asserting no diagnostic response.
        """
        target_session = uds_client_factory._target_session
        bus = target_session.can_bus()
        try:
            # Zero-DLC frame on the reserved RX ID.
            frame = can.Message(
                arbitration_id=EA_RX_ID,
                data=b"",
                is_extended_id=False,
            )
            bus.send(frame)
            # Listen briefly; there must be nothing on the TX ID.
            reply = bus.recv(timeout=1.0)
            if reply is not None:
                assert reply.arbitration_id != EA_TX_ID, (
                    f"Unexpected diagnostic reply on {EA_TX_ID:#x} "
                    f"for zero-length request"
                )
        finally:
            bus.shutdown()
