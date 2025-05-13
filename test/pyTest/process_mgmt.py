import subprocess
import os
from signal import SIGTERM
from target_info import TargetInfo
from capture_serial import capture_serial_by_name
from time import sleep
from threading import Thread

_target_process_runners = {}
_per_run_process_runners = {}


class ProcessRunner():
    """Manage launching and stopping of child processes

    This launches a process as specified in target configuration files
    using subprocess.Popen() with the options shell=True and
    start_new_session=True. This means that the command_line can
    contain shell instructions like redirection of stdio
    and any further child processes started by command_line will
    all have the same process group ID, and thus SIGTERM can be
    sent to all child processes if the option "kill_at_end" is used.
    """

    def __init__(self, target_name, **kwargs):
        self._target_name = target_name
        self._command_line = kwargs["command_line"]
        self._kill_at_end = kwargs.get("kill_at_end", False)
        wait_for_exit = kwargs.get("wait_for_exit", False)
        self._restart_if_exited = kwargs.get("restart_if_exited", False)
        self._launch_count = 0

        self._launch()

        if wait_for_exit:
            self._p_obj.wait()
            print(f"Wait completed for pid={self._p_obj.pid}")
        elif self._restart_if_exited:
            self._popen_monitor_thread = Thread(
                target=self._popen_monitor_func, daemon=True)
            self._popen_monitor_run = True
            self._popen_monitor_thread.start()

    @property
    def launch_count(self):
        return self._launch_count

    def _launch(self):
        self._p_obj = subprocess.Popen(
            self._command_line,
            shell=True,
            start_new_session=True)
        pgid = os.getpgid(self._p_obj.pid)
        self._launch_count = self._launch_count + 1
        print(f"Launch {self._launch_count} \"{self._command_line}\"" +
              f" for target={self._target_name}" +
              f" pid={self._p_obj.pid} pgid={pgid}")
        self._pgid = pgid if self._kill_at_end else None

    def stop(self):
        if self._restart_if_exited:
            self._popen_monitor_run = False
        if self._pgid:
            print("Send SIGTERM to process group "+str(self._pgid))
            try:
                os.killpg(self._pgid, SIGTERM)
                os.waitid(os.P_PGID, self._pgid, os.WEXITED)
            except Exception as ex:
                pass
        if self._restart_if_exited:
            self._popen_monitor_thread.join()

    def _popen_monitor_func(self):
        while self._popen_monitor_run:
            if self._p_obj.returncode is not None:
                self._launch()
            try:
                self._p_obj.wait(0.1)
            except Exception:
                pass


def start_target_process(target_name):
    target_info = TargetInfo.by_name[target_name]
    if target_info.target_process.get("command_line"):
        if target_info.target_process["restart_before_test"] or \
                not _target_process_runners.get(target_name):
            capture_serial_by_name[target_name].mark_not_booted()
            _target_process_runners[target_name] = ProcessRunner(
                target_name,
                **target_info.target_process)


def stop_target_process(target_name, force=False):
    target_info = TargetInfo.by_name[target_name]
    if target_info.target_process["restart_before_test"] or force:
        if _target_process_runners.get(target_name):
            _target_process_runners[target_name].stop()
            _target_process_runners[target_name] = None
            capture_serial_by_name[target_name].mark_not_booted()


def start_per_run_processes():
    for target_name, target_info in TargetInfo.by_name.items():
        if target_info.per_run_process.get("command_line") and \
                not _per_run_process_runners.get(target_name):
            _per_run_process_runners[target_name] = ProcessRunner(
                target_name,
                **target_info.per_run_process)


def stop_all_processes():
    for target_name in _target_process_runners:
        if _target_process_runners.get(target_name):
            _target_process_runners[target_name].stop()
            _target_process_runners[target_name] = None
    for target_name in _per_run_process_runners:
        if _per_run_process_runners.get(target_name):
            _per_run_process_runners[target_name].stop()
            _per_run_process_runners[target_name] = None
