
def test_console_lc_reboot(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    capserial.clear()

    capserial.send_command(b"lc reboot\n")
    capserial.mark_not_booted()
    assert capserial.wait_for_boot_complete()
