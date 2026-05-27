# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import pytest
import udsoncan
import udsoncan.services as uds
from helpers.helper_functions import hexlify


# Test to check Tester Present functionality
def test_3E00(target_session, uds_transport):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client(uds_transport)
    req = uds.TesterPresent.make_request()
    assert hexlify(uds_client.send_request(req).get_payload()) == "7e 00"
