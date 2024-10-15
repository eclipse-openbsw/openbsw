import pytest
from can.interfaces import socketcan
import isotp
from udsoncan.client import Client
from udsoncan.connections import PythonIsoTpConnection
from target_info import TargetInfo
from process_mgmt import start_target_process, stop_target_process, \
    start_per_run_processes, stop_all_processes
from capture_serial import start_capture_serial, close_capture_serial, \
    capture_serial_by_name, CaptureSerial


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
        start_target_process(self.target_name, True if TargetSession.counter == 1 else False)

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
        return (self.target_info.eth["ip_address"])

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

    def uds_client(self):
        """This can be called from a test to get a udsoncan Client object

        Return:
            udsoncan Client object for UDS interaction with the target
        """
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
    if TargetInfo.by_name:
        # Will be executed before the first test
        start_per_run_processes()
        start_capture_serial()
    yield 1
    if TargetInfo.by_name:
        # Will be executed after the last test
        close_capture_serial()
        stop_all_processes()


@pytest.fixture()
def hw_tester(request):
    return CaptureSerial(**request.param)


def pytest_addoption(parser):
    parser.addoption(
        "--target",
        action="append",
        default=[],
        help=TargetInfo.target_arg_help
    )
    parser.addoption(
        "--no-restart",
        action="store_true",
        help="Skip restart of target(s) before each test",
    )


def pytest_configure(config):
    if not TargetInfo.by_name:
        TargetInfo.load(config.getoption("target"),
                        config.getoption("--no-restart"))


def pytest_generate_tests(metafunc):
    if "target_session" in metafunc.fixturenames:
        if "hw_tester" in metafunc.fixturenames:
            fixture_args = []
            for name, target_info in TargetInfo.by_name.items():
                if target_info.hw_tester_serial:
                    fixture_args.append([name, target_info.hw_tester_serial])
            metafunc.parametrize("target_session,hw_tester",
                                 fixture_args,
                                 indirect=True)
        else:
            metafunc.parametrize("target_session",
                                 [name for name in TargetInfo.by_name],
                                 indirect=True)
