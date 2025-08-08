import os
import subprocess
import time


def run_process(cmd):
    uds_session_name = f"RefAppUdsTest{os.getpid()}"
    subprocess.run(["tmux", "new-session", "-d", "-s", uds_session_name])
    subprocess.run(["tmux", "send-keys", "-t", uds_session_name, cmd, "C-m"])
    time.sleep(1)

    result = subprocess.run(
        ["tmux", "capture-pane", "-t", uds_session_name, "-p"],
        capture_output=True,
        text=True,
    )
    subprocess.run(["tmux", "kill-session", "-t", uds_session_name])
    return result.stdout.strip().splitlines()[-4:-1]
