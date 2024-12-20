from serial import Serial
from threading import Thread, Event, Timer
import queue
import time
from target_info import TargetInfo
from pty_forwarder import PtyForwarder
from os import symlink, unlink


capture_serial_by_name = {}


class CaptureSerial(Serial):
    """ Capture bytes received from the target
    and make it easy to check content against expected.

    This derives from pyserial's Serial class and accepts
    the same arguments in its constructor.
    It adds a thread which is always reading bytes received.
    In the thread, each line received is checked for messages
    which signify boot has started or completed.
    Tests can easily wait for boot to be completed using
    the function wait_for_boot_complete().
    Each line read on the thread is added to a queue.
    Tests can easily read off this queue and check for
    expected messages using read_until().
    """

    def __init__(self, *args, **kwargs):
        self._target_name = ""
        if "name" in kwargs:
            self._target_name = kwargs["name"]
            del kwargs["name"]
        self._boot_started_bytes = None
        self._boot_complete_bytes = None
        self._max_boot_time = None
        self._last_booted_time = time.time()
        self._booted_event = Event()
        self._command_successful_event = Event()
        self._command_successful_expected = None
        self._boot_complete_wait_timer = None
        self._received_lines_queue = queue.Queue()

        self._pty_forwarder = None
        self._pty_ext_link = None
        if "port" in kwargs and \
                kwargs["port"] == PtyForwarder.PTY_SLAVE_A_ALIAS:
            self._pty_ext_link = kwargs["ext_link"]
            del kwargs["ext_link"]
            self._pty_forwarder = PtyForwarder()
            self._pty_forwarder.start()
            kwargs["port"] = self._pty_forwarder.slave_a_path
            symlink(self._pty_forwarder.slave_b_path, self._pty_ext_link)

        self._write_byte_delay = None
        if "write_byte_delay" in kwargs:
            self._write_byte_delay = kwargs["write_byte_delay"]
            del kwargs["write_byte_delay"]

        self._send_command_expected_bytes = None
        if "send_command_expected" in kwargs:
            self._send_command_expected_bytes = \
                kwargs["send_command_expected"].encode()
            del kwargs["send_command_expected"]

        self._send_command_timeout = None
        if "send_command_timeout" in kwargs:
            self._send_command_timeout = kwargs["send_command_timeout"]
            del kwargs["send_command_timeout"]

        self._send_command_max_retries = None
        if "send_command_max_retries" in kwargs:
            self._send_command_max_retries = kwargs["send_command_max_retries"]
            del kwargs["send_command_max_retries"]

        super().__init__(*args, **kwargs)

    def open(self):
        super().open()
        self._read_thread = Thread(target=self._read_thread_func, daemon=True)
        self._read_thread_run = True
        self._read_thread.start()

    def close(self):
        self._read_thread_run = False
        self._read_thread.join(timeout=0.1)
        super().close()
        if self._pty_forwarder:
            self._pty_forwarder.stop()
            unlink(self._pty_ext_link)

    def write(self, data, write_byte_delay=None):
        """Write data with a delay before each byte if specified as
        an argument or in the target's configuration file,
        otherwise write all data.
        If the write_byte_delay argument is set it overrides
        the same setting in the target's configuration file.
        """
        if write_byte_delay is None:
            write_byte_delay = self._write_byte_delay
        if write_byte_delay:
            write_count = 0
            for i in range(len(data)):
                time.sleep(write_byte_delay)
                ret = super().write(data[i:i+1])
                if isinstance(ret, int):
                    write_count = write_count + ret
            return write_count
        else:
            return super().write(data)

    def send_command(self, data,
                     expected=None,
                     timeout=None,
                     max_retries=None,
                     write_byte_delay=None):
        """Send a command and wait for expected bytes that confirm success.
        Arguments passed override target configuration file settings.
        """
        if expected is None:
            expected = self._send_command_expected_bytes
        if timeout is None:
            timeout = self._send_command_timeout
        if max_retries is None:
            max_retries = self._send_command_max_retries

        if not isinstance(expected, (bytes, bytearray)) \
            or not isinstance(timeout, (float, int)) \
                or not isinstance(max_retries, int):
            raise ValueError(
                "Valid settings are required to use this method")

        success = False
        self._command_successful_expected = expected
        self._command_successful_event.clear()
        for _ in range(max_retries+1):
            self.write(data, write_byte_delay=write_byte_delay)
            success = self._command_successful_event.wait(timeout=timeout)
            if success:
                break
        self._command_successful_expected = None
        return success

    def clear(self):
        """Delete any data received up to now"""
        self.reset_input_buffer()
        self.reset_output_buffer()
        self._received_lines_queue = queue.Queue()

    def add_boot_info(self, started_str, complete_str, max_time,
                      wait_time_after_complete=0.0,
                      assume_booted_at_start=False):
        """Set config data used in monitoring boot status"""
        self._boot_started_bytes = started_str.encode()
        self._boot_complete_bytes = complete_str.encode()
        self._wait_time_after_complete = wait_time_after_complete
        self._max_boot_time = max_time
        if assume_booted_at_start:
            self.mark_booted()

    def mark_booted(self):
        self._boot_complete_wait_timer = None
        self._booted_event.set()
        self._last_booted_time = time.time()
        # print(f"[{self._target_name}] booted = True")

    def mark_not_booted(self):
        if self._boot_complete_wait_timer:
            self._boot_complete_wait_timer.cancel()
            self._boot_complete_wait_timer = None
        self._booted_event.clear()
        # print(f"[{self._target_name}] booted = False")

    def seconds_since_booted(self):
        if self._booted_event.is_set():
            return time.time()-self._last_booted_time
        return 0.0

    def wait_for_boot_complete(self):
        """Block while waiting for string marking boot complete
        up until the max boot time is exceeded,
        or return immediately if already received.

        Return:
            True is boot reached, otherwise False
        """
        booted = self._booted_event.wait(self._max_boot_time)
        # print(f"[{self._target_name}] wait booted = {booted}")
        return booted

    def _check_line(self, line):
        if self._command_successful_expected and \
                self._command_successful_expected in line:
            self._command_successful_event.set()
        if self._boot_started_bytes and self._boot_started_bytes in line:
            self.mark_not_booted()
        elif self._boot_complete_bytes and self._boot_complete_bytes in line:
            if self._wait_time_after_complete:
                self._boot_complete_wait_timer = Timer(
                    self._wait_time_after_complete, self.mark_booted)
                self._boot_complete_wait_timer.start()
            else:
                self.mark_booted()

    def _read_thread_func(self):
        line = bytearray()
        while self._read_thread_run:
            try:
                for b in super().read():
                    line.append(b)
                    if b in b'\n':
                        # print(f"[{self._target_name}] In: {line.decode()}")
                        self._check_line(line)
                        self._received_lines_queue.put(line)
                        line = bytearray()
            except Exception:
                pass

    def read(self):
        """Not to be used directly"""
        raise NotImplementedError

    def readline(self):
        """Not to be used directly"""
        raise NotImplementedError

    def readlines(self):
        """Not to be used directly"""
        raise NotImplementedError

    def read_received_lines(self):
        """Return all lines from queue without waiting

        Return:
            list of bytearray, one per line
        """
        received_lines = []
        while True:
            try:
                received_lines.append(self._received_lines_queue.get_nowait())
            except queue.Empty:
                return received_lines

    def read_until(self, expected, timeout=None, match_all_expected=True):
        """Read lines from queue until expected is matched or timeout reached

        Args:
            expected: bytes (or bytearray) or a list (or tuple) containing
              multiple entries of bytes (or bytearray) which define what
              to look for in each line read from queue.
            timeout: seconds to wait. If None (default) it will not block
              which means only lines received up to now will be checked.
            match_all_expected: If True (default) then try to find matches
              for all entries in expected. If False then return when
              one entry in expected is found.

        Return:
            tuple containing (success, received_lines, expected_found_at_i)
            success is a boolean.
              If success is True with match_all_expected = True
              then matches were found for all entries in expected
              If success is True with match_all_expected = False
              then a match was found for at least one entry in expected
              Otherwise success is False
            received_lines is a list of bytearray, one per line taken from
              the queue. If success is True then it contains all lines up
              until a match was found that resulted in success = True.
              ie. There may be more lines still in the queue.
              If success is False then it contains all lines received up
              until the function returned, including a timeout if set.
            expected_found_at_i is a dict in which each key is an entry
              in expected.
              If expected_found_at_i[expected[j]] is None,
              then expected[j] was not found.
              If expected_found_at_i[expected[j]] is an integer i,
              then expected[j] was found at index i in received_lines.
              ie. received_lines[i] matched expected[j]
              Just the first match is recorded, if subsequent matches occur
              in later lines then this is not recorded.
        """
        success = False
        expected_found_at_i = {}
        if isinstance(expected, (bytes, bytearray)):
            expected_found_at_i[expected] = None
        elif isinstance(expected, (list, tuple)):
            for e in expected:
                if isinstance(e, (bytes, bytearray)):
                    expected_found_at_i[e] = None
                else:
                    raise TypeError
        else:
            raise TypeError

        received_lines = []
        start_time = time.time()
        while True:
            try:
                received_line = self._received_lines_queue.get(
                    block=True if timeout else False, timeout=timeout)
                # print("Out:"+received_line.decode())
                received_lines.append(received_line)
                for e in expected_found_at_i:
                    if expected_found_at_i[e] is None and e in received_line:
                        expected_found_at_i[e] = len(received_lines)-1
            except queue.Empty:
                break
            if timeout and time.time() - start_time > timeout:
                break

            if match_all_expected:
                if None not in expected_found_at_i.values():
                    success = True
                    break
            else:
                if list(expected_found_at_i.values()).count(None) \
                        < len(expected_found_at_i):
                    success = True
                    break

        return success, received_lines, expected_found_at_i


def start_capture_serial():
    """Create a CaptureSerial object for each target"""
    for name, target_info in TargetInfo.by_name.items():
        serial_info = target_info.serial
        serial_info["name"] = name
        if serial_info["port"] == PtyForwarder.PTY_SLAVE_A_ALIAS:
            serial_info["ext_link"] = target_info.pty_forwarder["ext_link"]
        capture_serial = CaptureSerial(**serial_info)
        if target_info.boot:
            capture_serial.add_boot_info(**target_info.boot)
        capture_serial_by_name[name] = capture_serial


def close_capture_serial():
    """Close all CaptureSerial objects"""
    for name in capture_serial_by_name:
        capture_serial_by_name[name].close()
