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
conftest.py - generic hardware-in-loop pytest infrastructure (target process
management, serial capture, CLI options, test parametrization) shared across
ALL test modules, not just DoCAN.

DoCAN/UDS-specific constants, addressing helpers, and the multi-scheme UDS
client factory now live in docan_helpers.py and are re-exported below, so
existing test files doing `from conftest import SUPPORTED_DID, EA_RX_ID, ...`
continue to work unchanged - no test file needs to change its imports.

RUNTIME ADDRESSING (no conditional compilation).

    The referenceApp is built as a SINGLE binary in which Normal (NA),
    Extended (EA) and NormalFixed (NF) addressing all coexist on ONE CAN bus /
    ONE transport layer, classified per frame by the stateless
    DoCanAddressingDispatcher (see DoCanAddressingDispatcher.h and
    docan_helpers.py for the requires_* markers this enables).

Usage:

    pytest tests/test_docan.py        # NA + EA + NF all active in one build
"""

import pytest
import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "docan"))
import functools
from datetime import datetime

from can.interfaces import socketcan
import isotp

from doipclient import DoIPClient
from doipclient.connectors import DoIPClientUDSConnector

# Kept as pass-through for backward compatibility: conftest.py itself does
# not use these three directly, but the original conftest.py imported them
# at module level, so any other test script doing
# `from conftest import udsoncan` / `Request` / `uds` (services alias) must
# keep working unchanged.
import udsoncan
from udsoncan import Request, services as uds
from udsoncan.client import Client
from udsoncan.connections import PythonIsoTpConnection

from target_info import TargetInfo
from process_mgmt import (
    start_target_process,
    stop_target_process,
    start_per_run_processes,
    stop_all_processes,
)
from capture_serial import (
    start_capture_serial,
    close_capture_serial,
    capture_serial_by_name,
    CaptureSerial,
)
from pathlib import Path
from serial_minilog import start_minilog, stop_minilog, on_line as minilog_on_line

# Re-export everything DoCAN/UDS-specific so `from conftest import X` keeps
# working for every test file exactly as before the split. See
# docan_helpers.py for definitions and rationale.
from docan_helpers import (  # noqa: F401
    DOCAN_MODE,
    requires_normal,
    requires_extended,
    requires_normal_fixed,
    requires_dispatch,
    ECU_ADDRESS,
    SUPPORTED_DID,
    UNSUPPORTED_DID,
    EXPECTED_CF01_PAYLOAD,
    DEFAULT_REQUEST_TIMEOUT_S,
    ECU_READY_RETRIES,
    ECU_READY_DELAY_S,
    EA_RX_ID,
    EA_TX_ID,
    EA_NL_TA,
    EA_TESTER_AE,
    EA_OUT_OF_RANGE_ID,
    RANGE_EXTENDED_TESTER_AE,
    RANGE_EXTENDED_ECU_AE,
    RANGE_EXTENDED_TESTER_CAN_ID,
    RANGE_EXTENDED_ECU_CAN_ID,
    AddrMode,
    ALL_MODES,
    ISO15765_PHYSICAL_PAIRS,
    hexlify,
    format_expected,
    wait_for_ecu_ready,
    accept_timeout_if_functional,
    expect_negative_or_timeout,
    _drain_bus,
    _assert_functional_ff_rejected,
    _ManagedUdsClient,
    UdsClientFactory,
    # Fixtures - pytest discovers these via conftest.py's namespace, not the
    # file they were originally defined in.
    uds_client_factory,
    addr_mode,
    uds_client,
)


class TargetSession:
    """Utility to help tests interact with a target
    This is instantiated and passed to each test with the target_session
    fixture.
    """

    counter = 0

    def __init__(self, target_name):
        TargetSession.counter += 1
        self.target_name = target_name
        self.target_info = TargetInfo.by_name[target_name]

    def start(self):
        """Start the target.
        This is called in fixture setup,
        it is not intended to be called from a test.
        """
        self.capserial().clear()
        start_target_process(self.target_name, TargetSession.counter == 1)

    def stop(self):
        """Stop the target.
        This is called in fixture teardown,
        it is not intended to be called from a test.
        """
        stop_target_process(self.target_name)

    def restart(self):
        """This can be called from a test to restart the target"""
        stop_target_process(self.target_name, force=True)
        start_target_process(self.target_name)

    def target_ip_address(self):
        """Provides Target IP Address"""
        return self.target_info.eth["ip_address"]

    def can_bus(self):
        """This can be called from a test to get a SocketcanBus object

        Return:
            SocketcanBus object on the same bus as used by the target
        """
        return socketcan.SocketcanBus(**self.target_info.socketcan)

    def capserial(self):
        """This can be called from a test to get a CaptureSerial object

        Return:
            CaptureSerial object for serial comms with the target
        """
        return capture_serial_by_name[self.target_name]

    def uds_client(self, uds_transport):
        """This can be called from a test to get a UDS Client

        Args:
            uds_transport: string containing "can" or "eth"

        Return:
            Client object for UDS interaction with the target
        """

        if uds_transport == "can":
            bus = self.can_bus()
            # legislative (ISO 15765-4) Normal Addressing CAN identifiers used by
            # DoCanSystem's Normal Addressing scheme (see DoCanSystem.cpp).
            tp_addr = isotp.Address(
                isotp.AddressingMode.Normal_11bits, txid=0x7E0, rxid=0x7E8
            )
            isotp_params = {
                "stmin": 0,
                "blocksize": 8,
                "wftmax": 0,
                "tx_padding": 0,
                "tx_data_min_length": None,
                "rx_flowcontrol_timeout": 1000,
                "rx_consecutive_frame_timeout": 1000,
            }
            stack = isotp.CanStack(bus=bus, address=tp_addr, params=isotp_params)
            conn = PythonIsoTpConnection(stack)
            conn.open()
            return Client(conn)

        if uds_transport == "eth":
            # Create a DoIPClient instance
            doipClient = DoIPClient(
                ecu_ip_address=self.target_ip_address(),
                ecu_logical_address=ECU_ADDRESS,
                protocol_version=2,
                client_logical_address=0x0EF1,
            )
            # Create a DoIPClientUDSConnector instance
            udsConnector = DoIPClientUDSConnector(doipClient)
            udsConnector.open()
            return Client(udsConnector)


@pytest.fixture()
def target_session(request):
    """Return a TargetSession object with target helpers and information.
    Tests using this fixture are parameterized to run once per target.
    """
    # Will be executed before each test requesting this fixture
    session = TargetSession(request.param)
    session.start()
    yield session
    # Will be executed after each test requesting this fixture
    session.stop()


@pytest.fixture(scope="session", autouse=True)
def once_per_pytest_run():
    """This is used once for the whole pytest run to provide opportunity
    for setup before all tests and teardown after all tests.
    """
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_path = os.environ.get("SERIAL_LOG_PATH", f"artifacts/serial_{timestamp}.log")
    if TargetInfo.by_name:
        # Will be executed before the first test
        start_per_run_processes()

        start_minilog(log_path)

        def on_line_factory(target_name):
            # Bind the target into the callback
            return functools.partial(minilog_on_line, target_name)

        start_capture_serial(on_line_factory=on_line_factory)

    yield 1

    if TargetInfo.by_name:
        close_capture_serial()
        stop_minilog()
        stop_all_processes()


@pytest.fixture()
def hw_tester(request):
    return CaptureSerial(**request.param)


@pytest.fixture()
def uds_transport(request):
    return request.param


def pytest_addoption(parser):
    parser.addoption(
        "--target", action="append", default=[], help=TargetInfo.target_arg_help
    )
    parser.addoption(
        "--no-restart",
        action="store_true",
        help="Skip restart of target(s) before each test",
    )
    parser.addoption(
        "--app",
        action="store",
        default="refApp_freertos",
        help="Select which software (app) configuration to flash, e.g., intTestApp_freertos or refApp_threadx based application",
    )


def pytest_configure(config):
    if not TargetInfo.by_name:
        TargetInfo.load(
            config.getoption("target"),
            config.getoption("--no-restart"),
            config.getoption("app"),
        )
    # Advertise the addressing model in the pytest header for CI visibility.
    config.stash["docan_mode"] = DOCAN_MODE


def pytest_report_header(config):
    return "DoCAN addressing: RUNTIME DISPATCH (NA + EA + NF all live on one bus)"


def pytest_generate_tests(metafunc):
    """Parameterize tests with fixtures. (unchanged)"""
    all_targets_fixture_args = []

    if "target_session" in metafunc.fixturenames:
        fixture_names = "target_session"

        need_hw_tester = False
        if "hw_tester" in metafunc.fixturenames:
            need_hw_tester = True
            fixture_names += ",hw_tester"

        need_uds_transport = False
        if "uds_transport" in metafunc.fixturenames:
            need_uds_transport = True
            fixture_names += ",uds_transport"

        for name, target_info in TargetInfo.by_name.items():
            if not need_hw_tester and not need_uds_transport:
                all_targets_fixture_args.append(name)

            if need_hw_tester:
                if target_info.hw_tester_serial:
                    all_targets_fixture_args.append(
                        [name, target_info.hw_tester_serial]
                    )

            if need_uds_transport:
                if target_info.socketcan:
                    all_targets_fixture_args.append([name, "can"])
                if target_info.eth:
                    all_targets_fixture_args.append([name, "eth"])

        metafunc.parametrize(fixture_names, all_targets_fixture_args, indirect=True)


def pytest_runtest_setup(item):
    skip_marker = item.get_closest_marker("skip_if")
    if skip_marker:
        condition = skip_marker.args[0]
        target = item.config.getoption("target")
        app = item.config.getoption("app")

        context = {"target": target, "app": app}
        if eval(condition, {}, context):
            pytest.skip(f"Skipped because condition '{condition}' matched")

# NOTE: Test classes must live in a test module (test_docan*.py), not in
# conftest. The gateway routing tests live in test_docan_dispatch_integration.py
# as TestDispatchGateway (gated by @requires_dispatch, which always passes in
# this single runtime-dispatch build).


def pytest_collection_modifyitems(config, items):
    for name, target in TargetInfo.by_name.items():
        enabled_tests, reason = TargetInfo.load_enabled_tests(target, config.option.app)
    if enabled_tests is None:
        for item in items:
            if reason is not None:
                pytest.skip(reason)
            else:
                pytest.skip(
                    f"No test_list configuration is assigned to the target under the application '{config.option.app}'"
                )
    for item in items:
        if item.path not in enabled_tests:
            item.add_marker(
                pytest.mark.skip(
                    reason="Test not enabled under enabled_tests in test config"
                )
            )
