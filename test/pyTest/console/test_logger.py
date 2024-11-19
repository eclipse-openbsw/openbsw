from time import sleep
import pytest


# Test to check the components and logger levels
def test_logger_levels(target_session):
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()
    expected = [
        b"Console command succeeded",
        b"ok",
        b'Received console command "logger level"',
        b"BSP             DEBUG",
        b"COMMON          DEBUG",
        b"DEMO            DEBUG",
        b"GLOBAL          DEBUG",
        b"LIFECYCLE       DEBUG",
        b"CONSOLE         DEBUG",
        b"CAN             DEBUG",
        b"DOCAN           DEBUG",
        b"UDS             DEBUG",
        b"TPROUTER        DEBUG",
    ]
    capserial.send_command(b"logger level\n")
    (success, lines, _) = capserial.read_until(expected, timeout=2)
    assert success


# Test to check the transition components between levels
@pytest.mark.parametrize(
    "level", [("error"), ("info"), ("warn"), ("critical"), ("debug")]
)
@pytest.mark.parametrize(
    "component",
    [
        ("BSP"),
        ("COMMON"),
        ("DEMO"),
        ("GLOBAL"),
        ("LIFECYCLE"),
        ("CAN"),
        ("CONSOLE"),
        ("DOCAN"),
        ("UDS"),
        ("TPROUTER"),
    ],
)
def test_logger(target_session, level, component):

    capserial = target_session.capserial()
    dummy_msg = "DEBUG: [CanDemoListener] CAN frame sent, id=0x558, length=4"
    assert capserial.wait_for_boot_complete()
    expected = [
        f'Received console command "logger level {component}{level}"'.encode(),
        b"ok",
        b"Console command succeeded",
    ]
    command = "logger level " + component + " " + level + "\n"
    capserial.clear()
    capserial.send_command(command.encode())
    (success, output, _) = capserial.read_until(
        expected, timeout=2, match_all_expected=False
    )
    assert success
    if " CAN " in command:
        (success, lines, _) = capserial.read_until(dummy_msg.encode(), timeout=2)
        if " debug" in command:
            assert success
        else:
            assert not success
