# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import unittest
from process_mgmt import ProcessRunner
from time import sleep

TARGETNAME = "test_target"
CMDLINE = "sleep 1"


class Test_ProcessRunner_Basics(unittest.TestCase):
    """Test ProcessRunner basic methods"""

    def setUp(self):
        pass

    def test_pty_captureserial_run1(self):
        pr = ProcessRunner(
                TARGETNAME,
                command_line=CMDLINE,
                wait_for_exit=True)
        self.assertEqual(pr._p_obj.returncode, 0)

    def test_pty_captureserial_run2(self):
        pr = ProcessRunner(
                TARGETNAME,
                command_line=CMDLINE,
                wait_for_exit=False)
        self.assertIsNone(pr._p_obj.returncode)
        pr._p_obj.wait()
        self.assertEqual(pr._p_obj.returncode, 0)

    def test_pty_captureserial_run3(self):
        pr = ProcessRunner(
                TARGETNAME,
                command_line=CMDLINE,
                kill_at_end=True)
        self.assertIsNone(pr._p_obj.returncode)
        pr.stop()
        pr._p_obj.wait()
        self.assertEqual(pr._p_obj.returncode, 0)

    def test_pty_captureserial_run5(self):
        pr = ProcessRunner(
                TARGETNAME,
                command_line=CMDLINE,
                restart_if_exited=True)
        sleep(5)
        pr.stop()
        pr._p_obj.wait()
        self.assertEqual(pr._p_obj.returncode, 0)
        self.assertGreaterEqual(pr.launch_count, 4)
