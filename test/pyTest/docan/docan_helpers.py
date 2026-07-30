# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Helpers to build a UDS client for each of the ISO 15765-2 addressing schemes that
referenceApp's DoCanSystem wires up simultaneously on CAN_0 (see DoCanSystem.cpp and
DoCanMultiAddressingTransportLayer for the C++ side of this setup).
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

# Extended Addressing (explicit, table-based CAN identifiers).
EXTENDED_TESTER_ID = 0xF4
EXTENDED_REQUEST_CAN_ID = 0x600
EXTENDED_RESPONSE_CAN_ID = 0x601

# Range Extended Addressing (CAN identifiers 0x680-0x77F mapped arithmetically onto transport
# addresses 0x00-0xFF).
RANGE_EXTENDED_TESTER_ID = 0xF2
RANGE_EXTENDED_BASE_CAN_ID = 0x680

# Normal Fixed Addressing (29 bit CAN identifiers, physical id base 0x18DA, functional (group)
# id base 0x18DB).
NORMAL_FIXED_TESTER_ID = 0xF3
NORMAL_FIXED_GROUP_ADDRESS = 0x33
NORMAL_FIXED_PHYSICAL_ID_BASE = 0x18DA0000
NORMAL_FIXED_FUNCTIONAL_ID_BASE = 0x18DB0000

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


def extended_addressing_client(bus):
    """Client for the (explicit, table-based) Extended Addressing scheme's physical (1:1)
    requests (tester 0xF4).

    The same client/stack also receives the (always physical) response to a functional request
    sent via send_extended_addressing_functional_request() below.
    """
    address = isotp.Address(
        isotp.AddressingMode.Extended_11bits,
        txid=EXTENDED_REQUEST_CAN_ID,
        rxid=EXTENDED_RESPONSE_CAN_ID,
        target_address=ECU_ADDRESS,
        source_address=EXTENDED_TESTER_ID,
    )
    return _client_from_address(bus, address)


def send_extended_addressing_functional_request(bus, payload):
    """Sends a raw single-frame Extended Addressing functional (broadcast) request.

    DoCanExtendedAddressingFilter requires an explicit table entry for every target address it
    accepts, so FUNCTIONAL_ALL_ISO14229 (0xDF) only resolves because referenceApp added such an
    entry to DoCanSystem.cpp - unlike Range Extended Addressing, this genuinely requires that
    extra referenceApp-side wiring. That entry deliberately reuses the ECU's own, real response
    CAN identifier (EXTENDED_RESPONSE_CAN_ID) rather than an unrelated placeholder one, so that
    even a (relatively unusual) multi-frame functional request would still cause any Flow Control
    frame sent while receiving it to go out correctly addressed - which required a small docan
    library change (DoCanExtendedAddressingFilter now allows successive entries to share the same
    _canId, needed since this entry and the ECU's own physical one both use the same, real CAN
    identifier). Since a single isotp.Address cannot express both the initial request's target
    (0xDF) and the Flow Control's target (0x2A) needed for our (multi-frame) CF01 response, the
    request is sent as a raw CAN frame instead of through an isotp stack;
    extended_addressing_client() (with its ordinary physical target address) is then used to
    receive the response, including its own correctly-addressed Flow Control frames. With
    extended addressing the CAN identifier only identifies the sender, not physical vs functional,
    so the functional request reuses the same EXTENDED_REQUEST_CAN_ID as physical requests. Only
    single-frame requests (payload of up to 7 bytes) are supported here, matching common UDS
    functional-request usage.
    """
    assert len(payload) <= 7, "functional requests must fit a single frame"
    data = (bytes([FUNCTIONAL_ALL_ISO14229, len(payload)]) + bytes(payload)).ljust(8, b"\x00")
    bus.send(can.Message(arbitration_id=EXTENDED_REQUEST_CAN_ID, data=data, is_extended_id=False))


def range_extended_addressing_client(bus):
    """Client for the Range Extended Addressing scheme's physical (1:1) requests (tester 0xF2)."""
    address = isotp.Address(
        isotp.AddressingMode.Extended_11bits,
        txid=RANGE_EXTENDED_BASE_CAN_ID + RANGE_EXTENDED_TESTER_ID,
        rxid=RANGE_EXTENDED_BASE_CAN_ID + ECU_ADDRESS,
        target_address=ECU_ADDRESS,
        source_address=RANGE_EXTENDED_TESTER_ID,
    )
    return _client_from_address(bus, address)


def send_range_extended_addressing_functional_request(bus, payload):
    """Sends a raw single-frame Range Extended Addressing functional (broadcast) request.

    DoCanRangeExtendedAddressingFilter maps *any* address extension byte within its configured
    0x00-0xFF range arithmetically onto a CAN identifier, with no check that it matches the ECU's
    own configured address - so FUNCTIONAL_ALL_ISO14229 (0xDF) already resolves correctly today,
    without any docan code changes, and the request is recognized as functional by the UDS layer
    via TransportConfiguration::isFunctionalAddress(). However, since the response to our
    (multi-frame) CF01 request requires a Flow Control frame addressed back to the ECU's *real*
    address (0x2A) - not 0xDF - a single isotp.Address cannot express both the initial request's
    target (0xDF) and the Flow Control's target (0x2A), so the request is sent as a raw CAN frame
    instead of through an isotp stack; range_extended_addressing_client() (with its ordinary
    physical target address) is then used to receive the (always physically-addressed) response,
    including its own correctly-addressed Flow Control frames. Only single-frame requests
    (payload of up to 7 bytes) are supported here, matching common UDS functional-request usage.
    """
    assert len(payload) <= 7, "functional requests must fit a single frame"
    can_id = RANGE_EXTENDED_BASE_CAN_ID + RANGE_EXTENDED_TESTER_ID
    data = (bytes([FUNCTIONAL_ALL_ISO14229, len(payload)]) + bytes(payload)).ljust(8, b"\x00")
    bus.send(can.Message(arbitration_id=can_id, data=data, is_extended_id=False))


def normal_fixed_addressing_client(bus):
    """Client for the Normal Fixed Addressing scheme's physical (1:1) requests (tester 0xF3).

    The same client/stack also receives the (always physical) response to a functional request
    sent via send_normal_fixed_functional_request() below.
    """
    address = isotp.Address(
        isotp.AddressingMode.NormalFixed_29bits,
        target_address=ECU_ADDRESS,
        source_address=NORMAL_FIXED_TESTER_ID,
    )
    return _client_from_address(bus, address)


def send_normal_fixed_functional_request(bus, payload):
    """Sends a raw single-frame Normal Fixed Addressing functional (broadcast) request.

    isotp.Address ties one fixed target_address to both physical and functional sends, but for
    Normal Fixed Addressing the functional target is a *group* address (0x33) distinct from any
    ECU's own physical address (0x2A), so the functional request is sent as a raw CAN frame
    instead of through an isotp stack. Only single-frame requests (payload of up to 7 bytes) are
    supported here, matching common UDS functional-request usage.
    """
    assert len(payload) <= 7, "functional requests must fit a single frame"
    can_id = (
        NORMAL_FIXED_FUNCTIONAL_ID_BASE | (NORMAL_FIXED_GROUP_ADDRESS << 8) | NORMAL_FIXED_TESTER_ID
    )
    data = bytes([len(payload)]) + bytes(payload)
    data = data.ljust(8, b"\x00")
    bus.send(can.Message(arbitration_id=can_id, data=data, is_extended_id=True))


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


def send_extended_addressing_functional_first_frame(bus, total_size=10):
    """Sends a raw ISO 15765-2 First Frame targeting Extended Addressing's functional/broadcast
    address (0xDF), declaring more data than fits a single frame; see
    send_normal_addressing_functional_first_frame() for why this must be ignored.
    """
    _send_first_frame(
        bus, EXTENDED_REQUEST_CAN_ID, bytes([FUNCTIONAL_ALL_ISO14229]), total_size, False
    )


def send_range_extended_addressing_functional_first_frame(bus, total_size=10):
    """Sends a raw ISO 15765-2 First Frame targeting Range Extended Addressing's functional/
    broadcast address (0xDF), declaring more data than fits a single frame; see
    send_normal_addressing_functional_first_frame() for why this must be ignored.
    """
    can_id = RANGE_EXTENDED_BASE_CAN_ID + RANGE_EXTENDED_TESTER_ID
    _send_first_frame(bus, can_id, bytes([FUNCTIONAL_ALL_ISO14229]), total_size, False)


def send_normal_fixed_addressing_functional_first_frame(bus, total_size=10):
    """Sends a raw ISO 15765-2 First Frame to Normal Fixed Addressing's functional (broadcast)
    identifier (group address 0x33), declaring more data than fits a single frame; see
    send_normal_addressing_functional_first_frame() for why this must be ignored.
    """
    can_id = (
        NORMAL_FIXED_FUNCTIONAL_ID_BASE | (NORMAL_FIXED_GROUP_ADDRESS << 8) | NORMAL_FIXED_TESTER_ID
    )
    _send_first_frame(bus, can_id, b"", total_size, True)


def assert_no_response(bus, response_can_id, is_extended_id, timeout=1.0):
    """Asserts that no frame with arbitration id `response_can_id` arrives on `bus` within
    `timeout` seconds - used to confirm a (deliberately malformed, e.g. multi-frame functional)
    request was correctly ignored rather than answered, e.g. with a Flow Control frame.
    """
    mask = 0x1FFFFFFF if is_extended_id else 0x7FF
    bus.set_filters([{"can_id": response_can_id, "can_mask": mask, "extended": is_extended_id}])
    message = bus.recv(timeout=timeout)
    assert message is None, f"unexpected response on 0x{response_can_id:x}: {message}"
