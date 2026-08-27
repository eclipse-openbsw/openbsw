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
DoCAN / UDS integration tests - Range Extended Addressing (EA_Range), 11-bit.

Covers DoCanRangeExtendedAddressingFilter: a formula-based CAN ID per node
(canId = 0x680 + transportId) with the AE (address extension) byte naming
the recipient, distinct from Classic Extended Addressing's single fixed
CAN-ID pair (see test_docan_ea.py). Tester AE 0xF2, ECU AE 0x2A -> tester
CAN id 0x772 (RANGE_EXTENDED_TESTER_CAN_ID), ECU CAN id 0x6AA
(RANGE_EXTENDED_ECU_CAN_ID). Runs only under the dispatch/runtime build
(@requires_dispatch), same as the rest of the adversarial bug-finding suite.

Split out of TestGatewayAdversarial in the original monolithic
test_docan.py to isolate Range Extended-specific coverage. The R16
concurrent-burst test (which exercises Range Extended alongside NA/EA/NF in
one combined test) remains in test_docan_dispatch_integration.py since it is
inherently cross-scheme. See test_docan.py in this same directory for the
full module index / aggregator.
"""
from __future__ import annotations
from conftest import (
    wait_for_ecu_ready,
    requires_dispatch,
    RANGE_EXTENDED_TESTER_CAN_ID,
    RANGE_EXTENDED_ECU_CAN_ID,
    RANGE_EXTENDED_ECU_AE,
    _drain_bus,
)
import can
import time


def _re_client(factory, target_address=RANGE_EXTENDED_ECU_AE):
    """
    Create a Range Extended Addressing client.

    The client transmits on RANGE_EXTENDED_TESTER_CAN_ID (its own AE-derived
    slot, 0x772) and listens on RANGE_EXTENDED_ECU_CAN_ID (the ECU's own
    AE-derived slot, 0x6AA). target_address is the AE byte carried in
    outgoing frames and names the recipient (defaults to the ECU's AE,
    0x2A).
    """
    return factory.create(
        RANGE_EXTENDED_TESTER_CAN_ID,
        RANGE_EXTENDED_ECU_CAN_ID,
        target_address=target_address,
    )


def _send_and_watch(
    target_session,
    arbitration_id,
    data,
    is_extended,
    watch_ids,
    window_s=1.0,
):
    """
    Send one raw CAN frame and return the first frame whose arbitration_id
    is in watch_ids. Return None if no watched frame is observed.

    Use this only for negative/silence checks.
    """
    bus = target_session.can_bus()
    try:
        _drain_bus(bus)

        bus.send(
            can.Message(
                arbitration_id=arbitration_id,
                data=bytearray(data),
                is_extended_id=is_extended,
            )
        )

        deadline = time.time() + window_s
        while time.time() < deadline:
            remaining = deadline - time.time()
            if remaining <= 0:
                return None

            rx = bus.recv(timeout=remaining)
            if rx is None:
                return None

            if rx.arbitration_id in watch_ids:
                return rx

        return None

    finally:
        bus.shutdown()


@requires_dispatch
class TestRangeExtendedAddressing:
    """
    Bug-finding tests for Range Extended Addressing, split out of the
    original TestGatewayAdversarial class.

    Coverage note: Range Extended Addressing (tester AE 0xF2, ECU AE 0x2A)
    uses a formula-based CAN ID per node (canId = 0x680 + AE) with the AE
    byte naming the recipient; its wiring is confirmed and covered starting
    at R17. Range Extended's functional-address (broadcast) configuration
    is not yet confirmed, so no functional-MF-rejection test (mirroring R13
    in the NA/NF suites) exists for it yet.
    """

    def test_range_extended_basic_request_response(self, uds_client_factory):
        """
        Basic sanity check for Range Extended Addressing.

        Tester (AE 0xF2) transmits on its own AE-derived slot
        RANGE_EXTENDED_TESTER_CAN_ID (0x772) with AE byte = ECU's AE (0x2A,
        the recipient). ECU is expected to respond on its own AE-derived
        slot RANGE_EXTENDED_ECU_CAN_ID (0x6AA) with AE byte = tester's AE
        (0xF2, the recipient). This exact request/response pair was
        confirmed live via candump/RefApp log on 2026-08-19.
        """
        client = _re_client(uds_client_factory)

        try:
            assert wait_for_ecu_ready(client), "Range Extended ECU not ready"
            client.empty_rxqueue()

            resp = client.change_session(0x01)

            assert resp.valid
            assert resp.service_data.session_echo == 0x01

        finally:
            client.close()

    def test_range_extended_wrong_target_address_dropped(
        self,
        uds_client_factory,
    ):
        """
        Range Extended frame on RANGE_EXTENDED_TESTER_CAN_ID (0x772,
        tester's own slot) with an AE byte that does NOT name the ECU
        (0x2A) must be dropped. This mirrors R5 (EA wrong NL_TA) for the
        Range Extended scheme: the CAN id alone identifies a valid sender,
        but the AE byte must still correctly name this ECU as the
        recipient, or the request must be silently rejected.
        """
        ready = _re_client(uds_client_factory)

        try:
            assert wait_for_ecu_ready(ready), "Range Extended ECU not ready"
            ready.empty_rxqueue()
        finally:
            ready.close()

        offender = _send_and_watch(
            uds_client_factory._target_session,
            arbitration_id=RANGE_EXTENDED_TESTER_CAN_ID,
            data=[0x2B, 0x02, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00],
            is_extended=False,
            watch_ids={RANGE_EXTENDED_ECU_CAN_ID},
            window_s=1.0,
        )

        assert offender is None, (
            f"ECU wrongly responded to bad Range Extended AE byte: {offender}"
        )
