import os
import pty
import unittest
from capture_serial import CaptureSerial
from time import sleep, time

BASICDATA = b'Hello\n'
DATANOTSENT = b"Not expected"

BOOTSTARTSTR = 'START OF BOOT'
BOOTCOMPLETESTR = 'END OF BOOT'
BOOTMAXTIME = 0.1

CMDEXPECTED = 'command successful'
CMDTIMEOUT = 0.1
CMDRETRIES = 2


class Test_Pty_CaptureSerial_Basics(unittest.TestCase):
    """Test PTY CaptureSerial basic methods"""

    def setUp(self):
        # Open PTY
        self.master, self.slave = pty.openpty()

    def test_pty_captureserial_open(self):
        with CaptureSerial(os.ttyname(self.slave), timeout=1) as capserial:
            pass  # OK

    def test_pty_serial_write(self):
        with CaptureSerial(os.ttyname(self.slave), timeout=1) as capserial:
            with os.fdopen(self.master, "rb") as fd:
                capserial.write(BASICDATA)
                capserial.flush()
                out = fd.read(len(BASICDATA))
                self.assertEqual(BASICDATA, out)

    def test_pty_serial_write_delays(self):
        with CaptureSerial(os.ttyname(self.slave), timeout=1) as capserial:
            with os.fdopen(self.master, "rb") as fd:
                capserial.write(BASICDATA, write_byte_delay=0.01)
                capserial.flush()
                out = fd.read(len(BASICDATA))
                self.assertEqual(BASICDATA, out)


class Test_Pty_CaptureSerial_Read(unittest.TestCase):
    """Test PTY CaptureSerial read methods"""

    def setUp(self):
        self.master, self.slave = pty.openpty()
        self.capserial = CaptureSerial(os.ttyname(self.slave), timeout=1)

    def tearDown(self):
        self.capserial.close()

    def test_pty_captureserial_read_exception(self):
        with self.assertRaises(NotImplementedError):
            self.capserial.read()

    def test_pty_captureserial_readline_exception(self):
        with self.assertRaises(NotImplementedError):
            self.capserial.readline()

    def test_pty_captureserial_readlines_exception(self):
        with self.assertRaises(NotImplementedError):
            self.capserial.readlines()

    def test_pty_captureserial_read_received_lines_1(self):
        with os.fdopen(self.master, "wb") as fd:
            lines = self.capserial.read_received_lines()
            self.assertEqual(lines, [])

    def test_pty_captureserial_read_received_lines_2(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(BASICDATA)
            fd.flush()

            sleep(0.1)
            lines = self.capserial.read_received_lines()
            self.assertEqual(lines, [BASICDATA])

    def test_pty_captureserial_read_received_lines_2(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(BASICDATA)
            fd.write(BASICDATA)
            fd.write(BASICDATA)
            fd.flush()

            sleep(0.1)
            lines = self.capserial.read_received_lines()
            self.assertEqual(lines, [BASICDATA, BASICDATA, BASICDATA])

    def test_pty_captureserial_read_received_lines_clear(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(BASICDATA)
            fd.write(BASICDATA)
            fd.write(BASICDATA)
            fd.flush()

            self.capserial.clear()

            fd.write(BASICDATA)
            fd.write(BASICDATA)
            fd.flush()

            sleep(0.1)
            lines = self.capserial.read_received_lines()
            self.assertEqual(lines, [BASICDATA, BASICDATA])

    def test_pty_captureserial_read_until_1(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(BASICDATA)
            fd.flush()

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                BASICDATA, timeout=0.1)
            self.assertTrue(success)
            self.assertEqual(expected_found_at_i[BASICDATA], 0)
            self.assertEqual(BASICDATA, lines[0])

    def test_pty_captureserial_read_until_2(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(BASICDATA)
            fd.flush()

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                DATANOTSENT, timeout=0.1)
            self.assertFalse(success)
            self.assertIsNone(expected_found_at_i[DATANOTSENT])
            self.assertEqual(BASICDATA, lines[0])

    def test_pty_captureserial_read_until_3(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(b"***LINE1***\n")
            fd.write(b"******LINE2******\n")
            fd.write(b"*********LINE3*********\n")
            fd.flush()

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                b"LINE2", timeout=0.1)
            self.assertTrue(success)
            self.assertEqual(expected_found_at_i[b"LINE2"], 1)
            self.assertIn(b"LINE2", lines[1])
            self.assertEqual(len(lines), 2)

            (success, more_lines, expected_found_at_i) = \
                self.capserial.read_until(DATANOTSENT, timeout=0.1)
            self.assertFalse(success)
            self.assertEqual(len(more_lines), 1)
            self.assertIn(b"LINE3", more_lines[0])

    def test_pty_captureserial_read_until_4(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(b"***LINE1***\n")
            fd.write(b"******LINE2******\n")
            fd.write(b"*********LINE3*********\n")
            fd.flush()

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                [b"LINE1", b"LINE2", b"LINE3"], timeout=0.1)
            self.assertTrue(success)
            self.assertEqual(expected_found_at_i[b"LINE1"], 0)
            self.assertEqual(expected_found_at_i[b"LINE2"], 1)
            self.assertEqual(expected_found_at_i[b"LINE3"], 2)
            self.assertIn(b"LINE1", lines[0])
            self.assertIn(b"LINE2", lines[1])
            self.assertIn(b"LINE3", lines[2])
            self.assertEqual(len(lines), 3)

    def test_pty_captureserial_read_until_5(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(b"***LINE1***\n")
            fd.write(b"******LINE2******\n")
            fd.write(b"*********LINE3*********\n")
            fd.flush()

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                [b"LINE1", b"LINE2", b"LINE3"],
                timeout=0.1, match_all_expected=False)
            self.assertTrue(success)
            self.assertEqual(expected_found_at_i[b"LINE1"], 0)
            self.assertIsNone(expected_found_at_i[b"LINE2"])
            self.assertIsNone(expected_found_at_i[b"LINE3"])
            self.assertIn(b"LINE1", lines[0])
            self.assertEqual(len(lines), 1)

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                [b"LINE1", b"LINE2", b"LINE3"],
                timeout=0.1, match_all_expected=False)
            self.assertTrue(success)
            self.assertIsNone(expected_found_at_i[b"LINE1"])
            self.assertEqual(expected_found_at_i[b"LINE2"], 0)
            self.assertIsNone(expected_found_at_i[b"LINE3"])
            self.assertIn(b"LINE2", lines[0])
            self.assertEqual(len(lines), 1)

            (success, lines, expected_found_at_i) = self.capserial.read_until(
                [b"LINE1", b"LINE2", b"LINE3"],
                timeout=0.1, match_all_expected=False)
            self.assertTrue(success)
            self.assertIsNone(expected_found_at_i[b"LINE1"])
            self.assertIsNone(expected_found_at_i[b"LINE2"])
            self.assertEqual(expected_found_at_i[b"LINE3"], 0)
            self.assertIn(b"LINE3", lines[0])
            self.assertEqual(len(lines), 1)

    def test_pty_captureserial_read_until_6(self):
        with os.fdopen(self.master, "wb") as fd:
            fd.write(b"***LINE1***\n")
            fd.write(b"******LINE2******\n")
            fd.write(b"*********LINE3*********\n")
            fd.flush()

            sleep(0.1)
            (success, lines, expected_found_at_i) = self.capserial.read_until(
                [b"LINE1", b"LINE2", b"LINE3"], timeout=None)
            self.assertTrue(success)
            self.assertEqual(expected_found_at_i[b"LINE1"], 0)
            self.assertEqual(expected_found_at_i[b"LINE2"], 1)
            self.assertEqual(expected_found_at_i[b"LINE3"], 2)
            self.assertIn(b"LINE1", lines[0])
            self.assertIn(b"LINE2", lines[1])
            self.assertIn(b"LINE3", lines[2])
            self.assertEqual(len(lines), 3)


class Test_Pty_CaptureSerial_Boot(unittest.TestCase):
    """Test PTY CaptureSerial boot monitoring methods"""

    def setUp(self):
        self.master, self.slave = pty.openpty()
        self.capserial = CaptureSerial(os.ttyname(self.slave), timeout=1)

    def tearDown(self):
        self.capserial.close()

    def test_pty_captureserial_add_boot_info_1(self):

        self.capserial.add_boot_info(
            BOOTSTARTSTR, BOOTCOMPLETESTR, BOOTMAXTIME)
        self.assertFalse(self.capserial.wait_for_boot_complete())

    def test_pty_captureserial_add_boot_info_2(self):

        self.capserial.add_boot_info(
            BOOTSTARTSTR, BOOTCOMPLETESTR, BOOTMAXTIME,
            assume_booted_at_start=True)
        self.assertTrue(self.capserial.wait_for_boot_complete())

    def test_pty_captureserial_add_boot_info_3(self):

        self.capserial.add_boot_info(
            BOOTSTARTSTR, BOOTCOMPLETESTR, BOOTMAXTIME)

        with os.fdopen(self.master, "wb") as fd:

            fd.write(BOOTCOMPLETESTR.encode())
            fd.write(b"\n")
            fd.flush()

            self.assertTrue(self.capserial.wait_for_boot_complete())

            fd.write(b"***"+BOOTSTARTSTR.encode()+b"***\n")
            fd.flush()
            sleep(0.1)

            self.assertFalse(self.capserial.wait_for_boot_complete())

            fd.write(b"***")
            fd.write(BOOTCOMPLETESTR.encode())
            fd.write(b"***\n")
            fd.flush()

            self.assertTrue(self.capserial.wait_for_boot_complete())

            self.capserial.mark_not_booted()
            self.assertFalse(self.capserial.wait_for_boot_complete())

            fd.write(BOOTCOMPLETESTR.encode()+b"\n")
            fd.flush()

            self.assertTrue(self.capserial.wait_for_boot_complete())

    def test_pty_captureserial_add_boot_info_4(self):

        WAITAFTERBOOTCOMPLETESTR = 1.0
        self.capserial.add_boot_info(
            BOOTSTARTSTR, BOOTCOMPLETESTR,
            BOOTMAXTIME+WAITAFTERBOOTCOMPLETESTR,
            wait_time_after_complete=WAITAFTERBOOTCOMPLETESTR)

        with os.fdopen(self.master, "wb") as fd:

            fd.write(BOOTCOMPLETESTR.encode())
            fd.write(b"\n")
            fd.flush()

            starttime = time()
            self.assertTrue(self.capserial.wait_for_boot_complete())
            elapsed_time = time() - starttime
            self.assertGreater(elapsed_time, WAITAFTERBOOTCOMPLETESTR)


class Test_Pty_CaptureSerial_SendCommand(unittest.TestCase):
    """Test PTY CaptureSerial send_command method"""

    def setUp(self):
        self.master, self.slave = pty.openpty()
        self.capserial = CaptureSerial(
            os.ttyname(self.slave),
            timeout=1,
            send_command_expected=CMDEXPECTED,
            send_command_timeout=CMDTIMEOUT,
            send_command_max_retries=CMDRETRIES)

    def tearDown(self):
        self.capserial.close()

    def test_pty_captureserial_add_send_command_info_1(self):

        with os.fdopen(self.master, "wb") as fd:

            fd.write(CMDEXPECTED.encode())
            fd.write(b"\n")
            fd.flush()

            success = self.capserial.send_command(
                b"do something",
                expected=CMDEXPECTED.encode())
            self.assertTrue(success)


if __name__ == '__main__':
    unittest.main()
