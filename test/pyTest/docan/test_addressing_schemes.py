# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Integration test showing that referenceApp's DoCanSystem correctly serves the same UDS
request over all four ISO 15765-2 addressing schemes it wires up simultaneously on CAN_0:
Normal Addressing, Extended Addressing, Range Extended Addressing and Normal Fixed Addressing,
each tested both physically and functionally (broadcast).

Also verifies, for every scheme, that a multi-frame request targeting the functional (broadcast)
address is ignored (no Flow Control frame sent in reply), as ISO 15765-2 forbids multi-frame
requests to a functional target.

See DoCanSystem.cpp and DoCanMultiAddressingTransportLayer for the C++ side of this setup, and
docan_helpers.py for how each scheme's isotp Address/Client is constructed and, for functional
requests, why a raw CAN frame is used instead of an isotp stack.
"""

import binascii

import udsoncan
import udsoncan.services as uds
from docan_helpers import (
    ECU_ADDRESS,
    EXTENDED_RESPONSE_CAN_ID,
    NORMAL_FIXED_PHYSICAL_ID_BASE,
    NORMAL_FIXED_TESTER_ID,
    NORMAL_RESPONSE_CAN_ID,
    RANGE_EXTENDED_BASE_CAN_ID,
    assert_no_response,
    extended_addressing_client,
    normal_addressing_client,
    normal_fixed_addressing_client,
    range_extended_addressing_client,
    send_extended_addressing_functional_first_frame,
    send_extended_addressing_functional_request,
    send_normal_addressing_functional_first_frame,
    send_normal_addressing_functional_request,
    send_normal_fixed_addressing_functional_first_frame,
    send_normal_fixed_functional_request,
    send_range_extended_addressing_functional_first_frame,
    send_range_extended_addressing_functional_request,
)

# DID actually implemented by referenceApp's UDS jobs (a 24-byte hard-coded value), read the
# same way over each addressing scheme.
DID_CF01 = 0xCF01
EXPECTED_CF01_PAYLOAD = (
    "62 cf 01 01 02 00 02 22 02 16 0f 01 00 00 6d 2f 00 00 01 06 00 00 8f e0 00 00 01"
)


def _hexlify(value):
    response = binascii.hexlify(value).decode("ascii")
    return " ".join(response[i : i + 2] for i in range(0, len(response), 2))


def _read_cf01(uds_client):
    req = uds.ReadDataByIdentifier.make_request(
        [DID_CF01], {DID_CF01: udsoncan.AsciiCodec(4)}
    )
    return _hexlify(uds_client.send_request(req).get_payload())


def test_normal_addressing_physical_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    client = normal_addressing_client(bus)
    assert _read_cf01(client) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_normal_addressing_functional_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    # The functional (broadcast) request is sent as a raw frame (see docan_helpers), but the ECU
    # always replies physically, so the same isotp stack used for physical requests receives it.
    client = normal_addressing_client(bus)
    send_normal_addressing_functional_request(bus, bytes([0x22, 0xCF, 0x01]))
    payload = client.conn.wait_frame(timeout=2, exception=True)
    assert _hexlify(payload) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_extended_addressing_physical_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    client = extended_addressing_client(bus)
    assert _read_cf01(client) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_extended_addressing_functional_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    # The functional (broadcast) request is sent as a raw frame (see docan_helpers), but the ECU
    # always replies physically, so the same isotp stack used for physical requests receives it.
    client = extended_addressing_client(bus)
    send_extended_addressing_functional_request(bus, bytes([0x22, 0xCF, 0x01]))
    payload = client.conn.wait_frame(timeout=2, exception=True)
    assert _hexlify(payload) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_range_extended_addressing_physical_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    client = range_extended_addressing_client(bus)
    assert _read_cf01(client) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_range_extended_addressing_functional_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    # The functional (broadcast) request is sent as a raw frame (see docan_helpers), but the ECU
    # always replies physically, so the same isotp stack used for physical requests receives it.
    client = range_extended_addressing_client(bus)
    send_range_extended_addressing_functional_request(bus, bytes([0x22, 0xCF, 0x01]))
    payload = client.conn.wait_frame(timeout=2, exception=True)
    assert _hexlify(payload) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_normal_fixed_addressing_physical_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    client = normal_fixed_addressing_client(bus)
    assert _read_cf01(client) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


def test_normal_fixed_addressing_functional_read_cf01(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    # The functional (broadcast) request is sent as a raw frame (see docan_helpers), but the ECU
    # always replies physically, so the same isotp stack used for physical requests receives it.
    client = normal_fixed_addressing_client(bus)
    send_normal_fixed_functional_request(bus, bytes([0x22, 0xCF, 0x01]))
    payload = client.conn.wait_frame(timeout=2, exception=True)
    assert _hexlify(payload) == EXPECTED_CF01_PAYLOAD
    client.close()
    bus.shutdown()


# ISO 15765-2 forbids multi-frame requests to a functional (broadcast) target. The following
# tests confirm each scheme's addressing filter correctly reports an invalid transmission
# address for such a request (see the corresponding addressing filter's getReceptionParameters()
# for the mechanism), causing DoCanReceiver to ignore it rather than reply with a Flow Control
# frame - verified here by asserting no frame at all is seen on the scheme's response identifier
# within a short timeout.


def test_normal_addressing_functional_multiframe_is_ignored(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    send_normal_addressing_functional_first_frame(bus)
    assert_no_response(bus, NORMAL_RESPONSE_CAN_ID, is_extended_id=False)
    bus.shutdown()


def test_extended_addressing_functional_multiframe_is_ignored(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    send_extended_addressing_functional_first_frame(bus)
    assert_no_response(bus, EXTENDED_RESPONSE_CAN_ID, is_extended_id=False)
    bus.shutdown()


def test_range_extended_addressing_functional_multiframe_is_ignored(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    send_range_extended_addressing_functional_first_frame(bus)
    assert_no_response(bus, RANGE_EXTENDED_BASE_CAN_ID + ECU_ADDRESS, is_extended_id=False)
    bus.shutdown()


def test_normal_fixed_addressing_functional_multiframe_is_ignored(target_session):
    assert target_session.capserial().wait_for_boot_complete()
    bus = target_session.can_bus()
    send_normal_fixed_addressing_functional_first_frame(bus)
    response_can_id = NORMAL_FIXED_PHYSICAL_ID_BASE | (NORMAL_FIXED_TESTER_ID << 8) | ECU_ADDRESS
    assert_no_response(bus, response_can_id, is_extended_id=True)
    bus.shutdown()
