# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Helpers to build a UDS client for the ISO 15765-2 addressing schemes that referenceApp's
DoCanSystem wires up on CAN_0 (see DoCanSystem.cpp).
"""

import can
import isotp
from udsoncan.client import Client
from udsoncan.connections import PythonIsoTpConnection

# ECU's own address, shared by all addressing schemes (see appConfig.h LOGICAL_ADDRESS).
ECU_ADDRESS = 0x2A

# UDS functional (broadcast) target address recognized generically by referenceApp's UDS
# dispatcher, regardless of addressing scheme (see TransportConfiguration::isFunctionalAddress()
# and FUNCTIONAL_ALL_ISO14229).
FUNCTIONAL_ALL_ISO14229 = 0xDF

# Normal Addressing (legislative ISO 15765-4 identifiers).
NORMAL_TESTER_ID = 0xF1
NORMAL_REQUEST_CAN_ID = 0x7E0
NORMAL_RESPONSE_CAN_ID = 0x7E8
NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID = 0x7DF

_ISOTP_PARAMS = {
    "stmin": 0,
    "blocksize": 8,
    "wftmax": 0,
    "tx_padding": 0,
    "tx_data_min_length": None,
    "rx_flowcontrol_timeout": 1000,
    "rx_consecutive_frame_timeout": 1000,
}


def _client_from_address(bus, address):
    """Build a udsoncan Client on top of an isotp stack for the given Address."""
    stack = isotp.CanStack(bus=bus, address=address, params=_ISOTP_PARAMS)
    conn = PythonIsoTpConnection(stack)
    conn.open()
    return Client(conn)


def normal_addressing_client(bus):
    """Client for the legislative Normal Addressing scheme's physical (1:1) requests (tester
    0xF1).

    The same client/stack also receives the (always physical) response to a functional request
    sent via send_normal_addressing_functional_request() below.
    """
    address = isotp.Address(
        isotp.AddressingMode.Normal_11bits,
        txid=NORMAL_REQUEST_CAN_ID,
        rxid=NORMAL_RESPONSE_CAN_ID,
    )
    return _client_from_address(bus, address)


def send_normal_addressing_functional_request(bus, payload):
    """Sends a raw single-frame Normal Addressing functional (broadcast) request.

    isotp.Address's Normal_11bits mode only supports a single (physical) txid/rxid pair, with no
    separate functional identifier, so the functional request is sent as a raw CAN frame instead
    of through an isotp stack. Only single-frame requests (payload of up to 7 bytes) are
    supported here, matching common UDS functional-request usage.
    """
    assert len(payload) <= 7, "functional requests must fit a single frame"
    data = (bytes([len(payload)]) + bytes(payload)).ljust(8, b"\x00")
    bus.send(
        can.Message(
            arbitration_id=NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID, data=data, is_extended_id=False
        )
    )


def _send_first_frame(bus, can_id, prefix, total_size, is_extended_id):
    """Sends a raw ISO-TP First Frame (with no actual Consecutive Frames to follow), declaring
    `total_size` bytes total. `prefix` holds any bytes preceding the ISO-TP PCI (e.g. the address
    extension/N_TA byte for extended addressing, or empty for normal/normal fixed addressing).
    Only used to probe how a receiver reacts to *starting* a multi-frame reception; the sender
    never actually continues with Consecutive Frames, matching this module's tests, which only
    care whether a Flow Control frame is (wrongly) sent in reply.
    """
    assert total_size > 7, "must declare a size that requires multiple frames"
    pci = bytes([0x10 | ((total_size >> 8) & 0x0F), total_size & 0xFF])
    data = (prefix + pci).ljust(8, b"\x00")
    bus.send(can.Message(arbitration_id=can_id, data=data, is_extended_id=is_extended_id))


def send_normal_addressing_functional_first_frame(bus, total_size=10):
    """Sends a raw ISO 15765-2 First Frame to the Normal Addressing functional (broadcast)
    identifier (0x7DF), declaring more data than fits a single frame. ISO 15765-2 forbids
    multi-frame requests to a functional target, so a compliant receiver must ignore this (not
    reply with a Flow Control frame) - see test_addressing_schemes.py for the assertion.
    """
    _send_first_frame(bus, NORMAL_ADDRESSING_FUNCTIONAL_CAN_ID, b"", total_size, False)


def assert_no_response(bus, response_can_id, is_extended_id, timeout=1.0):
    """Asserts that no frame with arbitration id `response_can_id` arrives on `bus` within
    `timeout` seconds - used to confirm a (deliberately malformed, e.g. multi-frame functional)
    request was correctly ignored rather than answered, e.g. with a Flow Control frame.
    """
    mask = 0x1FFFFFFF if is_extended_id else 0x7FF
    bus.set_filters([{"can_id": response_can_id, "can_mask": mask, "extended": is_extended_id}])
    message = bus.recv(timeout=timeout)
    assert message is None, f"unexpected response on 0x{response_can_id:x}: {message}"

