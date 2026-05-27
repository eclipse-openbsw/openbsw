# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import can


def test_canListener(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    msg = can.Message(
        arbitration_id=0x123,
        data=[0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88],
        is_extended_id=False,
    )
    can_bus = target_session.can_bus()
    can_bus.set_filters([{"can_id": 0x124, "can_mask": 0x7FF}])
    can_bus.send(msg)
    rx_msg = can_bus.recv(2.0)  # Waits for 2 seconds to receive message from 0x124
    can_bus.shutdown()
    assert rx_msg is not None, "CAN message not received within the timeout"
    assert rx_msg.arbitration_id == msg.arbitration_id + 1
    assert rx_msg.data == msg.data
