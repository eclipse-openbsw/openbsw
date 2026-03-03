import os
import threading
from datetime import datetime

_log_file = None
_log_lock = threading.Lock()


def start_minilog(path: str):
    global _log_file
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    _log_file = open(path, "a", encoding="utf-8", buffering=1)
    _log_file.write(
        f"===== Serial capture started: {datetime.now().astimezone().isoformat(timespec='milliseconds')} =====\n"
    )


def stop_minilog():
    global _log_file
    if _log_file:
        _log_file.write(
            f"===== Serial capture ended:   {datetime.now().astimezone().isoformat(timespec='milliseconds')} =====\n"
        )
        _log_file.flush()
        _log_file.close()
        _log_file = None


def on_line(target_name: str, raw_line: bytes):
    """Callback to pass into CaptureSerial"""
    if _log_file is None:
        return
    try:
        text = (
            raw_line.replace(b"\r\n", b"\n")
            .replace(b"\r", b"\n")
            .decode("utf-8", errors="replace")
        )
        if not text.endswith("\n"):
            text += "\n"
        line = f"{datetime.now().astimezone().isoformat(timespec='milliseconds')} {target_name} | {text}"
        with _log_lock:
            _log_file.write(line)
            _log_file.flush()
    except Exception:
        pass
