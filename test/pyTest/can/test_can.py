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
    rx_msg = can_bus.recv()
    can_bus.shutdown()

    assert rx_msg.arbitration_id == msg.arbitration_id + 1
    assert rx_msg.data == msg.data
