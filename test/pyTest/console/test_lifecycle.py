from time import sleep
import pytest


# Test checks whether jumping back to level 8 from other levels result in reappearing of Dummy messages
@pytest.mark.parametrize("level1", [(7), (6), (5), (4), (3), (2), (1)])
def test_lc_dummy_msg_check(target_session, level1):
    capserial = target_session.capserial()
    dummy_msg = "DEBUG: [CanDemoListener] CAN frame sent, id=0x558, length=4"
    assert capserial.wait_for_boot_complete()

    capserial.clear()
    capserial.write(f"lc level {level1}\n".encode())
    (success, lines, _) = capserial.read_until(
        b"Console command succeeded", timeout=0.5
    )
    assert success
    (success, lines, _) = capserial.read_until(
        b"DEBUG: [CanDemoListener] CAN frame sent, id=0x558, length=4", timeout=3
    )
    assert not success

    capserial.clear()
    capserial.write(b"lc level 8\n")

    (success, lines, _) = capserial.read_until(
        b"DEBUG: [CanDemoListener] CAN frame sent, id=0x558, length=4", timeout=3
    )
    assert success


# Test to check change to level1 is successful by checking if level2 has properly shutdown
def test_lc_levelchange(target_session):
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()
    levels = [(7, 8), (6, 7), (5, 6), (4, 5), (3, 4), (2, 3), (1, 2)]
    for level1, level2 in levels:
        capserial.write(f"lc level {level1}\n".encode())
        expected = [
            b"Console command succeeded",
            b"ok",
            f'Received console command "lc level {level1}"'.encode(),
            f"DEBUG: Shutdown level {level2} done".encode(),
        ]

        (success, lines, _) = capserial.read_until(expected, timeout=2)
        assert success
    target_session.restart()
    assert capserial.wait_for_boot_complete()


# Test to check lifecycle reboot command
def test_lc_reboot(target_session):
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()
    expected = [
        b"Console command succeeded",
        b"ok",
        b"DEBUG: Shutdown level 8 done",
        b"DEBUG: Shutdown level 7 done",
        b"DEBUG: Shutdown level 6 done",
        b"DEBUG: Shutdown level 5 done",
        b"DEBUG: Shutdown level 4 done",
        b"DEBUG: Shutdown level 3 done",
        b"DEBUG: Shutdown level 2 done",
        b"DEBUG: Shutdown level 1 done",
        b"hello",
        b"DEBUG: Run level 1 done",
        b"DEBUG: Run level 2 done",
        b"DEBUG: Run level 3 done",
        b"DEBUG: Run level 4 done",
        b"DEBUG: Run level 5 done",
        b"DEBUG: Run level 6 done",
        b"DEBUG: Run level 7 done",
        b"DEBUG: Run level 8 done",
    ]
    capserial.write(b"lc reboot\n")
    (success, lines, _) = capserial.read_until(expected, timeout=3)
    assert success
