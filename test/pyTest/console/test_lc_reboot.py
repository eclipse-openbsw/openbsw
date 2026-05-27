# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************


def test_console_lc_reboot(target_session):

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    capserial.clear()

    capserial.send_command(b"lc reboot\n")
    capserial.mark_not_booted()
    assert capserial.wait_for_boot_complete()
