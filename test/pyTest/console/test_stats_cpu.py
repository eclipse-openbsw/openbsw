from time import sleep


def test_console_stats_cpu(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    # stats commands need a second after boot
    # and seems to need some time between successive calls to it
    sleep(1)

    capserial.clear()

    capserial.send_command(b"stats cpu\n")
    (success, _, _) = capserial.read_until(
        [b"ok",
         b"Received console command \"stats cpu\"",
         b"Console command succeeded"],
        timeout=2.0,
        match_all_expected=False)
    assert success


def test_console_cpu_health(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    # stats commands need a while after boot we retry a couple of times
    success = False
    for _ in range(6):
        sleep(5)

        capserial.clear()

        capserial.send_command(b"stats cpu\n")
        (success, lines, _) = capserial.read_until(b"idle", timeout=2.0)
        if success:
            break

    assert success

    parts = lines[-1].split()
    assert (parts[0] == b"idle")
    percent_idle = float(parts[1])
    if "posix_ci" != capserial._target_name:
        assert percent_idle > 10, f"Idle time {percent_idle}% is not > 10%"
    else:
        assert percent_idle == 0, f"Time should be frozen in Nix CI"
