from time import sleep


def test_console_stats_stack(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    # stats commands need a second after boot
    # and seems to need some time between successive calls to it
    sleep(1)

    capserial.clear()

    capserial.send_command(b"stats stack\n")
    (success, _, _) = capserial.read_until(
        [b"ok",
         b"Received console command \"stats stack\"",
         b"Console command succeeded"],
        timeout=0.5)
    assert success
