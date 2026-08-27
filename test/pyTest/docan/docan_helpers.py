# *******************************************************************************
# Copyright (c) 2024-2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************
"""
docan_helpers.py - DoCAN / UDS specific constants, addressing helpers, and the
multi-scheme UDS client factory used by the DoCAN integration test suite.

Split out of conftest.py to separate DoCAN/UDS-specific test logic from the
generic hardware-in-loop pytest infrastructure (target process management,
serial capture, CLI options, test parametrization) that conftest.py retains.
Everything defined here is re-exported by conftest.py, so existing test files
doing `from conftest import SUPPORTED_DID, EA_RX_ID, ...` continue to work
unchanged - no test file needs to change its imports.

RUNTIME ADDRESSING (no conditional compilation).

    The referenceApp is built as a SINGLE binary in which Normal (NA),
    Extended (EA) and NormalFixed (NF) addressing all coexist on ONE CAN bus /
    ONE transport layer, classified per frame by the stateless
    DoCanAddressingDispatcher (see DoCanAddressingDispatcher.h). There is
    therefore no compile-time DOCAN_MODE any more: every addressing scheme is
    always live, so the requires_normal/requires_extended/requires_normal_fixed/
    requires_dispatch markers below are all permanent no-ops (kept only so
    existing @requires_* decorators on test classes keep working).
"""
import time

import isotp
from udsoncan.client import Client
from udsoncan.connections import PythonIsoTpConnection
from udsoncan.exceptions import NegativeResponseException, TimeoutException

import pytest
import can

# ---------------------------------------------------------------------------
# Runtime addressing: all schemes live, always.
#
# The build no longer selects a single addressing mode at compile time, so
# there is nothing to auto-detect. DOCAN_MODE is fixed to "DISPATCH" purely so
# the historical marker names keep meaning "this scheme is available" - and in
# the runtime build every scheme is available, so they ALL pass.
# ---------------------------------------------------------------------------
DOCAN_MODE = "DISPATCH"

# In the runtime build NA, EA and NF are all live -> these markers never skip.
requires_normal = pytest.mark.skipif(
    False, reason="Runtime dispatch build: Normal addressing is always active"
)
requires_extended = pytest.mark.skipif(
    False, reason="Runtime dispatch build: Extended addressing is always active"
)
requires_normal_fixed = pytest.mark.skipif(
    False, reason="Runtime dispatch build: NormalFixed addressing is always active"
)
# The gateway/dispatch tests are always applicable in this single build too.
requires_dispatch = pytest.mark.skipif(
    False, reason="Runtime dispatch build: gateway routing is always active"
)


# ---------------------------------------------------------------------------
# DoCAN / UDS constants
# ---------------------------------------------------------------------------

ECU_ADDRESS = 0x2A
SUPPORTED_DID = 0xCF01
UNSUPPORTED_DID = 0xFFFF

EXPECTED_CF01_PAYLOAD = (
    "62 cf 01 01 02 00 02 22 02 16 0f 01 00 00 6d 2f 00 00 01 06 00 00 8f e0 00 00 01"
)

DEFAULT_REQUEST_TIMEOUT_S = 3.0
ECU_READY_RETRIES = 12
ECU_READY_DELAY_S = 0.5

# Extended demo pairing - must match DoCanSystem.cpp (_extendedAddresses[]).
EA_RX_ID = 0x600   # tester -> ECU
EA_TX_ID = 0x601   # ECU -> tester
EA_NL_TA = 0x2A    # target address byte placed in payload[0]
EA_TESTER_AE = 0xF4  # AE expected on RX (tester)
EA_OUT_OF_RANGE_ID = 0x700  # negative-case ID outside reserved [0x600,0x6FF]

# Range Extended demo pairing - must match DoCanSystem.cpp / DoCanConfig.h.
RANGE_EXTENDED_TESTER_AE = 0xF2
RANGE_EXTENDED_ECU_AE = 0x2A
RANGE_EXTENDED_TESTER_CAN_ID = 0x772
RANGE_EXTENDED_ECU_CAN_ID = 0x6AA


# ---------------------------------------------------------------------------
# Addressing modes.
# ---------------------------------------------------------------------------

class AddrMode:
    """
    Descriptor for one parametrization of the addr_mode fixture.

    target_address:
        None       => Normal 11-bit or NormalFixed 29-bit addressing.
        integer    => Extended 11-bit addressing; the byte placed in payload[0]
                      (NL_TA) and matched against the ECU's rxTargetAddress.
    """

    def __init__(self, label, txid, rxid, target_address=None):
        self.label = label
        self.txid = txid
        self.rxid = rxid
        self.target_address = target_address

    def __repr__(self):
        return self.label

    @property
    def is_functional(self):
        return "FUNCTIONAL" in self.label

    @property
    def is_extended_addressing(self):
        return self.target_address is not None

    @property
    def is_can29(self):
        return (self.txid > 0x7FF) or (self.rxid > 0x7FF)

    @property
    def is_physical(self):
        return not self.is_functional


# ---------------------------------------------------------------------------
# Per-scheme parametrization catalogues.
#
# In the runtime build all three catalogues are ALWAYS exposed together, since
# NA, EA and NF are simultaneously live on the one bus.
#
# !!! VERIFY txid/rxid against DoCanSystem.cpp before relying upstream !!!
#   NORMAL        physical 0x7E0->0x7E8, functional 0x7DF
#   EXTENDED      0x600<->0x601, NL_TA 0x2A
#   NORMAL_FIXED  physical 0x18DA2AF3<->0x18DAF32A, functional 0x18DB33F3
# ---------------------------------------------------------------------------

_NORMAL_MODES = [
    AddrMode("NORMAL_PHYSICAL", txid=0x7E0, rxid=0x7E8),
    AddrMode("NORMAL_FUNCTIONAL", txid=0x7DF, rxid=0x7E8),
]

_EXTENDED_MODES = [
    AddrMode("EXTENDED_PHYSICAL", txid=EA_RX_ID, rxid=EA_TX_ID, target_address=EA_NL_TA),
]

_NORMAL_FIXED_MODES = [
    AddrMode("NORMAL_FIXED_PHYSICAL", txid=0x18DA2AF3, rxid=0x18DAF32A),
    AddrMode("NORMAL_FIXED_FUNCTIONAL", txid=0x18DB33F3, rxid=0x18DAF32A),
]

# Runtime build: every scheme is live -> expose them all at once.
_ACTIVE_MODES = _NORMAL_MODES + _EXTENDED_MODES + _NORMAL_FIXED_MODES

# Backward-compatible alias referenced by the addr_mode fixture.
ALL_MODES = _ACTIVE_MODES

ISO15765_PHYSICAL_PAIRS = [
    pytest.param(0x7E0, 0x7E8, id="ECU1_7E0_TO_7E8"),
    pytest.param(0x7E1, 0x7E9, id="ECU2_7E1_TO_7E9"),
    pytest.param(0x7E2, 0x7EA, id="ECU3_7E2_TO_7EA"),
    pytest.param(0x7E3, 0x7EB, id="ECU4_7E3_TO_7EB"),
    pytest.param(0x7E4, 0x7EC, id="ECU5_7E4_TO_7EC"),
    pytest.param(0x7E5, 0x7ED, id="ECU6_7E5_TO_7ED"),
    pytest.param(0x7E6, 0x7EE, id="ECU7_7E6_TO_7EE"),
    pytest.param(0x7E7, 0x7EF, id="ECU8_7E7_TO_7EF"),
]


# ---------------------------------------------------------------------------
# DoCAN helpers
# ---------------------------------------------------------------------------

def hexlify(data):
    return " ".join(f"{b:02x}" for b in data)


def format_expected(address):
    return f"0x{address:08X}"


def wait_for_ecu_ready(client, retries=ECU_READY_RETRIES, delay=ECU_READY_DELAY_S):
    """Poll the ECU with TesterPresent until it responds (positively or negatively)."""
    try:
        client.empty_rxqueue()
    except Exception:
        pass
    for _ in range(retries):
        try:
            resp = client.tester_present()
            if resp is not None:
                try:
                    client.empty_rxqueue()
                except Exception:
                    pass
                return True
        except NegativeResponseException:
            try:
                client.empty_rxqueue()
            except Exception:
                pass
            return True
        except TimeoutException:
            time.sleep(delay)
    try:
        client.empty_rxqueue()
    except Exception:
        pass
    return False


def accept_timeout_if_functional(addr_mode, exc):
    """Functional (broadcast) requests may legally get no reply."""
    if addr_mode.is_functional:
        return
    raise exc


def expect_negative_or_timeout(uds_client, addr_mode, req, allowed_nrcs):
    try:
        response = uds_client.send_request(req)
        assert response.negative, "Expected negative response"
        assert response.code in allowed_nrcs
    except NegativeResponseException as e:
        assert e.response.code in allowed_nrcs
    except TimeoutException as e:
        accept_timeout_if_functional(addr_mode, e)


def _drain_bus(bus, duration_s=0.3):
    deadline = time.time() + duration_s
    while time.time() < deadline:
        if bus.recv(timeout=0.05) is None:
            break


def _assert_functional_ff_rejected(bus, functional_id, physical_resp_id, is_extended):
    """
    Send a FirstFrame (20-byte announced) on functional_id and assert the ECU
    stays silent: no response on physical_resp_id and no FlowControl (PCI 0x3X)
    frame anywhere. Per ISO 15765-2, a functional (broadcast) FirstFrame MUST
    be discarded by the server (no FC, no response).

    Shared by TestNormalFunctional (NA), TestNormalFixedFunctional (NF), and
    TestIso15765_4_Functional_MF_Rejection (dispatch build) - all three
    reimplemented this identical "send FF, poll for silence, assert no FC"
    logic before being consolidated here.
    """
    ff = can.Message(
        arbitration_id=functional_id,
        data=bytes([0x10, 0x14, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00]),
        is_extended_id=is_extended,
    )
    bus.send(ff)
    reply = bus.recv(timeout=1.0)
    while reply is not None:
        assert reply.arbitration_id != physical_resp_id, (
            f"multi-frame functional request must be discarded, but got a "
            f"frame on physical id {physical_resp_id:#x}: {reply}"
        )
        assert (reply.data and (reply.data[0] & 0xF0) != 0x30), (
            f"ECU must not send FlowControl for functional multi-frame: {reply}"
        )
        reply = bus.recv(timeout=0.2)


# ---------------------------------------------------------------------------
# Managed UDS client
# ---------------------------------------------------------------------------

class _ManagedUdsClient:
    """
    Wraps a udsoncan Client with deterministic teardown of the underlying
    isotp stack, PythonIsoTpConnection and SocketCAN bus.
    """

    def __init__(self, client, connection, tp_layer, can_bus=None):
        self._client = client
        self._connection = connection
        self._tp_layer = tp_layer
        self._can_bus = can_bus

    def __getattr__(self, name):
        return getattr(self._client, name)

    # Support `with client:` usage in the gateway tests.
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()
        return False

    def empty_rxqueue(self):
        try:
            self._connection.empty_rxqueue()
        except Exception:
            pass

    def close(self):
        self.empty_rxqueue()
        for step in (self._client.close, self._connection.close):
            try:
                step()
            except Exception:
                pass
        try:
            self._tp_layer.stop()
        except Exception:
            pass
        if self._can_bus is not None:
            try:
                self._can_bus.shutdown()
            except Exception:
                pass


class UdsClientFactory:
    """
    Creates managed UDS clients bound to the target's SocketCAN bus.

    create(txid, rxid) selects addressing as follows:
        target_address is None
            - CAN ID > 0x7FF  -> isotp Normal_29bits (NormalFixed 29-bit)
            - CAN ID <= 0x7FF -> isotp Normal_11bits (Normal 11-bit)
        target_address is set (integer)
            - Extended_11bits addressing; NL_TA byte placed in payload[0].
    """

    def __init__(self, target_session):
        self._target_session = target_session
        self._clients = []

    @staticmethod
    def _build_isotp_params():
        return {
            "stmin": 0,
            "blocksize": 8,
            "wftmax": 0,
            "tx_padding": 0,
            "tx_data_min_length": None,
            "rx_flowcontrol_timeout": 1500,
            "rx_consecutive_frame_timeout": 1500,
        }

    def create(
        self,
        txid,
        rxid,
        is_extended_id=None,
        target_address=None,
        source_address=None,
        timeout=DEFAULT_REQUEST_TIMEOUT_S,
    ):
        # Distinguish 11-bit vs 29-bit CAN ID width.
        if is_extended_id is None:
            is_extended_id = (txid > 0x7FF) or (rxid > 0x7FF)

        can_bus = self._target_session.can_bus()

        # Choose the isotp addressing mode.
        if target_address is not None:
            if is_extended_id:
                raise ValueError(
                    "Extended addressing (target_address) with 29-bit CAN IDs "
                    "is not supported by this factory."
                )

            rx_source_address = EA_TESTER_AE if source_address is None else source_address

            # Range Extended's tester/ECU CAN-id pair uses a different AE
            # convention than Classic EA (see RANGE_EXTENDED_* constants
            # above) - recognize it by its known txid/rxid/target_address
            # triple and use its own tester AE instead of EA_TESTER_AE.
            if (
                txid == RANGE_EXTENDED_TESTER_CAN_ID
                and rxid == RANGE_EXTENDED_ECU_CAN_ID
                and target_address == RANGE_EXTENDED_ECU_AE
                and source_address is None
            ):
                rx_source_address = RANGE_EXTENDED_TESTER_AE

            address = isotp.Address(
                isotp.AddressingMode.Extended_11bits,
                txid=txid,
                rxid=rxid,
                target_address=target_address,      # AE written on TX  (target = ECU)
                source_address=rx_source_address,   # AE expected on RX (target = tester)
            )
        elif is_extended_id:
            address = isotp.Address(
                isotp.AddressingMode.Normal_29bits,
                txid=txid,
                rxid=rxid,
            )
        else:
            address = isotp.Address(
                isotp.AddressingMode.Normal_11bits,
                txid=txid,
                rxid=rxid,
            )

        params = self._build_isotp_params()

        tp_layer = isotp.CanStack(bus=can_bus, address=address, params=params)
        connection = PythonIsoTpConnection(tp_layer)

        # Guard against "Transport Layer is already started" across isotp versions.
        try:
            connection.open()
        except RuntimeError as e:
            if "already started" not in str(e).lower():
                raise
        print(
            f"ISOTP CREATE: txid=0x{txid:X} rxid=0x{rxid:X} "
            f"ext_id={is_extended_id} target_address={target_address}"
        )

        client = Client(
            connection,
            request_timeout=timeout,
            config={
                "exception_on_negative_response": True,
                "exception_on_invalid_response": True,
                "exception_on_unexpected_response": True,
                "p2_timeout": 5.0,
                "p2_star_timeout": 5.0,
            },
        )
        # Do NOT call client.open() here. connection.open() already started
        # the transport layer. Calling client.open() would attempt to start
        # it a second time and raise RuntimeError.

        managed = _ManagedUdsClient(client, connection, tp_layer, can_bus)
        managed.empty_rxqueue()  # <-- ensure a clean queue on every created client
        self._clients.append(managed)
        return managed

    def close_all(self):
        while self._clients:
            self._clients.pop().close()


# ---------------------------------------------------------------------------
# DoCAN fixtures
#
# These are picked up by pytest once imported into conftest.py's namespace -
# fixture discovery inspects the conftest module's globals, not the file the
# fixture function was originally written in. `uds_client_factory` depends on
# the `target_session` fixture, which lives in conftest.py itself; this works
# fine because pytest resolves fixtures by name against the whole session's
# fixture registry, regardless of which module defined which fixture.
# ---------------------------------------------------------------------------

@pytest.fixture(scope="function")
def uds_client_factory(target_session):
    factory = UdsClientFactory(target_session)
    try:
        yield factory
    finally:
        factory.close_all()


@pytest.fixture(params=ALL_MODES, ids=lambda m: m.label)
def addr_mode(request):
    return request.param


@pytest.fixture(scope="function")
def uds_client(uds_client_factory, addr_mode):
    client = uds_client_factory.create(
        addr_mode.txid,
        addr_mode.rxid,
        target_address=addr_mode.target_address,
    )
    if addr_mode.is_physical:
        assert wait_for_ecu_ready(client), (
            f"ECU not ready for {addr_mode.label} after "
            f"{ECU_READY_RETRIES} retries x {ECU_READY_DELAY_S:.1f}s"
        )
    client.empty_rxqueue()
    try:
        yield client
    finally:
        client.close()
