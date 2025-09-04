import pytest

@pytest.mark.parametrize("can_id", [
    0x123,
    0x100,
])
@pytest.mark.parametrize("data", [
    [1, 2, 3, 4, 5, 6, 7, 8],
    [0xA1, 0xB2, 0xC3, 0xD4, 0xE5, 0xF6, 0xA7, 0xB8]
])
def test_can_send_command(target_session, can_id, data):
    # Initialize CAN bus first
    can_bus = target_session.can_bus()
    can_bus.set_filters([{"can_id": can_id, "can_mask": 0x7FF}])

    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    data_bytes = bytes(data)
    data_str = " ".join(str(x) for x in data)

    send_command = f"can send {hex(can_id)} {data_str}\n"
    send_expected = [
        b"ok",
        f'Received console command "can send {hex(can_id)} {data_str}"'.encode(),
        b"Console command succeeded",
    ]

    capserial.clear()
    capserial.send_command(send_command.encode())
    success, output, _ = capserial.read_until(send_expected, timeout=2, match_all_expected=True)
    assert success, f"can send failed: {output}"

    rx_msg = can_bus.recv(timeout=1.0)
    if rx_msg is None:
        raise TimeoutError("No CAN message received within timeout")

    assert rx_msg.arbitration_id == can_id, \
        f"Expected CAN ID {hex(can_id)}, got {hex(rx_msg.arbitration_id)}"
    assert rx_msg.data == data_bytes, \
        f"Expected data {data_bytes.hex()}, got {rx_msg.data.hex()}"

    can_bus.shutdown()