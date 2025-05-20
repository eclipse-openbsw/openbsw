import pytest
import udsoncan
import udsoncan.services as uds
from helpers.helper_functions import hexlify

# Test: Diagnostic response should be okay when a valid request is sent
def test_RDBI_valid_request(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    did = 0xCF02
    req = uds.ReadDataByIdentifier.make_request(
        [did], {did: udsoncan.AsciiCodec(4)})
    assert uds_client.send_request(req).get_payload()[0] == 0x62


# Test: Checking positive response for did F186
def test_RDBI_CF01(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    did = 0xCF01
    req = uds.ReadDataByIdentifier.make_request(
        [did], {did: udsoncan.AsciiCodec(4)})
    assert (
        hexlify(uds_client.send_request(req).get_payload())
        == "62 cf 01 01 02 00 02 22 02 16 0f 01 00 00 6d 2f 00 00 01 06 00 00 8f e0 00 00 01"
    )


# Test: Checking positive response for did F102
def test_RDBI_cf02(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    did = 0xCF02
    req = uds.ReadDataByIdentifier.make_request(
        [did], {did: udsoncan.AsciiCodec(4)})
    payload = uds_client.send_request(req).get_payload()
    # Just checking the start of the payload because
    # the remaining bytes vary depending on the ADC value
    assert payload[0:3] == bytes([0x62, 0xCF, 0x02])


# Test: Checking NRC ISO_REQUEST_OUT_OF_RANGE is received
def test_RDBI_ISO_REQUEST_OUT_OF_RANGE(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    did = 0xFFFF
    req = uds.ReadDataByIdentifier.make_request(
        [did], {did: udsoncan.AsciiCodec(2)})
    with pytest.raises(udsoncan.exceptions.NegativeResponseException) as excinfo:
        uds_client.send_request(req)
    assert excinfo.value.response.code == 0x31
    assert hexlify(excinfo.value.response.get_payload()) == "7f 22 31"


# Test: Checking NRC ISO_SERVICE_NOT_SUPPORTED is received
def test_RDBI_ISO_SERVICE_NOT_SUPPORTED(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    request_payload = bytes([0x21, 0xCF, 0x02])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert payload[2] == 0x11
    assert hexlify(payload) == "7f 21 11"


# Test: Checking NRC ISO_INVALID_FORMAT is received
def test_RDBI_ISO_INVALID_FORMAT(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    request_payload = bytes([0x22, 0xF1])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert payload[2] == 0x13
    assert hexlify(payload) == "7f 22 13"


if __name__ == "__main__":
    pytest.main()
