import pytest

@pytest.mark.parametrize("block_id,value", [
    ("2561", "aabbccdd"),
    ("2817", "01ff"),
])
def test_storage_write_and_read(target_session, block_id, value):
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    write_command = f"storage write {block_id} {value}\n"
    write_expected = [
        b"Writing ok",
        b"ok",
        f'Received console command "storage write {block_id} {value}"'.encode(),
        b"Console command succeeded",
    ]
    capserial.clear()
    capserial.send_command(write_command.encode())
    success, output, _ = capserial.read_until(write_expected, timeout=2)
    assert success, f"Write failed for block_id={block_id}, value={value}"

    # Reset the device
    target_session.restart()
    assert capserial.wait_for_boot_complete()

    # Read back to verify
    read_command = f"storage read {block_id} {len(value)//2}\n"
    read_expected = [
        b"Reading ok",
        b"ok",
        f'Received console command "storage read {block_id} {len(value)//2}"'.encode(),
        b"Console command succeeded",
    ]
    capserial.clear()
    capserial.send_command(read_command.encode())
    success, output, _ = capserial.read_until(read_expected, timeout=2)
    assert success, f"Read failed for block_id={block_id}"

    check_output(output, value)

@pytest.mark.parametrize("block_id,fill_value,fill_size", [
    ("2561", "ab", 4),
    ("2817", "01", 5),
])
def test_storage_fill_and_read(target_session, block_id, fill_value, fill_size):
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()

    fill_command = f"storage fill {block_id} 0x{fill_value} {fill_size}\n"
    fill_expected = [
        b"Fill ok",
        b"ok",
        f'Received console command "storage fill {block_id} 0x{fill_value} {fill_size}"'.encode(),
        b"Console command succeeded",
    ]
    capserial.clear()
    capserial.send_command(fill_command.encode())
    success, output, _ = capserial.read_until(fill_expected, timeout=2)
    assert success, f"Fill failed for block_id={block_id}, value={fill_value}, size={fill_size}"

    target_session.restart()
    assert capserial.wait_for_boot_complete()

    read_command = f"storage read {block_id} {fill_size}\n"
    read_expected = [
        b"Reading ok",
        b"ok",
        f'Received console command "storage read {block_id} {fill_size}"'.encode(),
        b"Console command succeeded",
    ]
    capserial.clear()
    capserial.send_command(read_command.encode())
    success, output, _ = capserial.read_until(read_expected, timeout=2)
    assert success, f"Read after fill failed for block_id={block_id}"

    expected_filled = fill_value * fill_size
    check_output(output, expected_filled)

def test_storage_read_non_configured_block(target_session):
    """Test reading from a non-configured block id should fail."""
    invalid_block_id = "1234"  # Use a block id that is not configured
    read_command = f"storage read {invalid_block_id} 2\n"
    capserial = target_session.capserial()
    assert capserial.wait_for_boot_complete()
    capserial.clear()
    capserial.send_command(read_command.encode())
    # Read output and check for any error keywords
    error_keywords = [b"failed", b"ERROR", b"invalid"]
    success, output, _ = capserial.read_until(error_keywords, timeout=2)
    found = any(any(keyword in line for keyword in error_keywords) for line in output)
    assert found, f"Expected error for non-configured block id, got output: {output}"

def check_output(output, value):
    found = False
    for line in output:
        if b'0    :' in line:
            print(f"line: {line}")
            # Extract the part after the colon, remove all whitespace, and compare
            hex_part = line.split(b':', 1)[1].decode(errors='ignore')
            # Remove all whitespace (spaces, tabs, etc.)
            hex_digits = ''.join(hex_part.split()).strip()
            if hex_digits.startswith(value):
                found = True
                break
    assert found, f"Expected value '{value}' not found in output: {output}"
