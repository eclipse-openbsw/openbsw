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
DoCAN / UDS integration tests - mode-agnostic UDS service coverage.

These tests are parametrized over `addr_mode` (via the `uds_client` fixture)
and run identically regardless of which addressing scheme (NA/EA/NF) is
active on the single runtime-dispatch build. They exercise core UDS services
(DiagnosticSessionControl, ReadDataByIdentifier, TesterPresent,
SecurityAccess) without caring about the underlying CAN-ID/addressing wiring.

Split out of the original monolithic test_docan.py to keep addressing-scheme
-specific coverage (NA/EA/NF/dispatch) in their own files. See test_docan.py
in this same directory for the full module index / aggregator.
"""
from __future__ import annotations
import udsoncan
from udsoncan import services as uds
from udsoncan.exceptions import NegativeResponseException, TimeoutException
from conftest import (
    SUPPORTED_DID,
    UNSUPPORTED_DID,
    EXPECTED_CF01_PAYLOAD,
    hexlify,
    accept_timeout_if_functional,
    expect_negative_or_timeout,
)
# ---------------------------------------------------------------------------
# Mode-agnostic tests: parametrized over the active mode's addr_mode values.
# ---------------------------------------------------------------------------
class TestDiagnosticSessionControl:
    def test_default_session(self, uds_client, addr_mode):
        try:
            resp = uds_client.change_session(0x01)
            assert resp.valid
            assert resp.service_data.session_echo == 0x01
        except TimeoutException as e:
            accept_timeout_if_functional(addr_mode, e)
class TestReadDataByIdentifier:
    def test_rdbi_valid_request(self, uds_client, addr_mode):
        req = uds.ReadDataByIdentifier.make_request(
            [SUPPORTED_DID], {SUPPORTED_DID: udsoncan.AsciiCodec(4)}
        )
        try:
            resp = uds_client.send_request(req)
            assert resp.get_payload()[0] == 0x62
            # The CF01 response is 27 bytes (0x62 + DID + 24 data bytes), so a
            # full-payload match also proves ISO 15765-2 multi-frame
            # segmentation/reassembly (FF + CF) works in this addressing mode.
            assert hexlify(resp.get_payload()) == EXPECTED_CF01_PAYLOAD
        except TimeoutException as e:
            accept_timeout_if_functional(addr_mode, e)
    def test_rdbi_unsupported_did_negative(self, uds_client, addr_mode):
        """
        ISO 14229: a ReadDataByIdentifier for a DID the server does not
        support must yield NRC 0x31 (requestOutOfRange). openbsw sets this
        as the default return code (ReadDataByIdentifier.cpp), and the job
        root returns 0x31 for an unhandled identifier.
        """
        req = uds.ReadDataByIdentifier.make_request(
            [UNSUPPORTED_DID], {UNSUPPORTED_DID: udsoncan.AsciiCodec(1)}
        )
        expect_negative_or_timeout(uds_client, addr_mode, req, allowed_nrcs=[0x31])
class TestTesterPresent:
    def test_tester_present_positive(self, uds_client, addr_mode):
        try:
            resp = uds_client.tester_present()
            assert resp.valid
        except TimeoutException as e:
            accept_timeout_if_functional(addr_mode, e)
class TestSecurityAccess:
    def test_request_seed_level_01(self, uds_client, addr_mode):
        try:
            resp = uds_client.request_seed(0x01)
            assert resp is not None
            assert resp.valid
        except NegativeResponseException as e:
            assert e.response is not None
        except TimeoutException as e:
            accept_timeout_if_functional(addr_mode, e)
