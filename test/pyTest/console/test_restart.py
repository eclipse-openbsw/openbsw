def test_console_restart(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    startup_bytes = b"INFO: Initialize level 1"
    booted_bytes = b"DEBUG: Run level 8 done"

    capserial.clear()
    # restart the target
    target_session.restart()

    # Wait for startup line
    (success, received_lines, expected_found_at_i) = capserial.read_until(
        startup_bytes, timeout=0.5
    )
    assert success

    # Wait for booted line
    (success, received_lines, expected_found_at_i) = capserial.read_until(
        booted_bytes, timeout=1.0
    )
    assert success
