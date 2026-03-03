import pytest, os
from can.interfaces import socketcan
import isotp
from doipclient import DoIPClient
from doipclient.connectors import DoIPClientUDSConnector
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
from serial_minilog import start_minilog, stop_minilog, on_line as minilog_on_line
import functools
from datetime import datetime


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
            tp_addr = isotp.Address(
                isotp.AddressingMode.Normal_11bits, txid=0x002A, rxid=0x00F0
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
                ecu_logical_address=0x002A,
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
            # Bind the target into the callback (CaptureSerial calls _on_line(self._target_name, bytes(line)))
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
        default="freertos",
        help="Select which software (app) configuration to flash, e.g., threadx or freertos based application",
    )


def pytest_configure(config):
    if not TargetInfo.by_name:
        TargetInfo.load(
            config.getoption("target"),
            config.getoption("--no-restart"),
            config.getoption("app"),
        )


def pytest_generate_tests(metafunc):
    """Parameterize tests with fixtures.

    All tests have the fixture `target_session`.
    If more than one target is specified on the `pytest` command line
    then tests will run once for each target, with a different value for `target_session`.

    Tests with the fixture `hw_tester` require a target with a special test board
    and the section `[hw_tester_serial]` in the target's `.toml` file specifies
    how to communicate with that board.
    If this is not found in the target's `.toml` file then tests with `hw_tester` are skipped.

    Tests with the fixture `uds_transport` require a target with
    either `[socketcan]` for UDS over CAN or `[eth]` for UDS over IP,
    or, if both `[socketcan]` and `[eth]` are in the target's `.toml` file
    then those tests will run twice, once over CAN and once over IP.
    If neither are present then those tests are skipped.

    Note this doesn't support tests having both fixtures `hw_tester` and `uds_transport`
    as no such test exists yet.
    """
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
                    # The target has hw_tester so this test can run
                    all_targets_fixture_args.append(
                        [name, target_info.hw_tester_serial]
                    )

            if need_uds_transport:
                if target_info.socketcan:
                    # Test UDS over CAN
                    all_targets_fixture_args.append([name, "can"])
                if target_info.eth:
                    # Test UDS over Ethernet
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
