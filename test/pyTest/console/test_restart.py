
def test_console_restart(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    startup_bytes = b"INFO: Initialize level 1"
    booted_bytes = b"DEBUG: Run level 8 done"

    capserial.clear()
    capserial.mark_not_booted()
    # restart the target
    target_session.restart()

    assert capserial.wait_for_boot_complete()
