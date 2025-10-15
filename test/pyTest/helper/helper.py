import subprocess
import time
import os


def run_process(cmd):
    session_name = f"RefAppTest{os.getpid()}"
    subprocess.run(["tmux", "new-session", "-d", "-s", session_name])
    subprocess.run(["tmux", "send-keys", "-t", session_name, cmd, "C-m"])
    time.sleep(1)

    result = subprocess.run(
        ["tmux", "capture-pane", "-t", session_name, "-p"],
        capture_output=True,
        text=True,
    )
    subprocess.run(["tmux", "kill-session", "-t", session_name])
    return result.stdout.strip().splitlines()[-4:-1]
