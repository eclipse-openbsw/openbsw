import os
import socket
import time


# Test: Basic test with ping command to check if the ethernet connection is working
def test_ping(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    response = os.system(f"ping -c 4 {target_session.target_ip_address()}")
    assert response == 0


# Test for UDP - Send n (= num_packets) packets and
# check if the target demo app echoes back the sent data.
def test_throughput_UDP(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    udpsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    TARGET_ADDR = (target_session.target_ip_address(), 49444)
    RECV_BUF_SIZE = 65535

    num_packets = 1000
    num_bytes_send = 50

    for i in range(num_packets):
        data_to_send = str.encode(f"{i:04d}" + "#" * (num_bytes_send - 4))
        udpsocket.sendto(data_to_send, TARGET_ADDR)
        received_data, _ = udpsocket.recvfrom(RECV_BUF_SIZE)
        assert data_to_send == received_data, f"Mismatch occurred: sent {data_to_send}, received {received_data}"


# Test for TCP - Send n (= num_packets) packets and
# check if the target demo app echoes back the sent data.
def test_throughput_TCP(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    tcpsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    TARGET_ADDR = (target_session.target_ip_address(), 49555)

    RECV_BUF_SIZE = 65535

    # Timeout in seconds
    TIMEOUT = 1
    tcpsocket.connect(TARGET_ADDR)
    tcpsocket.settimeout(TIMEOUT)

    num_packets = 1000
    num_bytes_send = 50

    for i in range(num_packets):
        data_to_send = str.encode(f"{i%10}" * num_bytes_send)
        tcpsocket.send(data_to_send)
        received_data = b""
        start_time = time.time()

        while len(received_data) < len(data_to_send):
            if (time.time() - start_time) > TIMEOUT:
                break
            try:
                chunk, _ = tcpsocket.recvfrom(RECV_BUF_SIZE)
                received_data += chunk
            except:
                pass

            if len(received_data) < len(data_to_send):
                # Adding a small delay before reading again
                time.sleep(0.01)

        assert (
            data_to_send == received_data
        ), f"Mismatch occurred: sent {data_to_send}, received {received_data}"
