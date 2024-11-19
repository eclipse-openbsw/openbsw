import pytest
import udsoncan
import udsoncan.services as uds
from helpers.helper_functions import hexlify


# Test: The following session changes are from
# Default session to Extended session, should be ok
# Default session to Default session, should be ok
# Extended session to Extended session, should be ok
# Extended session to Default session, should be ok
@pytest.mark.parametrize(
    "id1,id2,ex_op",
    [
        (0x01, 0x03, "50 03 00 32 01 f4"),
        (0x01, 0x01, "50 01 00 32 01 f4"),
        (0x03, 0x03, "50 03 00 32 01 f4"),
        (0x03, 0x01, "50 01 00 32 01 f4"),
    ],
)
def test_session_change(target_session, id1, id2, ex_op):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    id = id1
    req = uds.DiagnosticSessionControl().make_request(id)
    assert uds_client.send_request(req).get_payload()[0] == 0x50
    id = id2
    req = uds.DiagnosticSessionControl().make_request(id)
    assert hexlify(uds_client.send_request(req).get_payload()) == ex_op


# Test: Testing with invalid subfunction, should return NRC ISO_SUBFUNCTION_NOT_SUPPORTED
def test_ISO_SUBFUNCTION_NOT_SUPPORTED(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    id = 0x10
    req = uds.DiagnosticSessionControl().make_request(id)
    with pytest.raises(udsoncan.exceptions.NegativeResponseException) as excinfo:
        uds_client.send_request(req)
    assert excinfo.value.response.code == 0x12
    assert hexlify(excinfo.value.response.get_payload()) == "7f 10 12"


# Test: Testing with invalid values for service (0x11) instead of 0x10, should receive NRC ISO_SERVICE_NOT_SUPPORTED
def test_ISO_SERVICE_NOT_SUPPORTED(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    request_payload = bytes([0x11, 0x01])
    uds_client.conn.send(request_payload)
    payload = uds_client.conn.wait_frame(timeout=2, exception=True)
    assert payload[2] == 0x11
    assert hexlify(payload) == "7f 11 11"


if __name__ == "__main__":
    pytest.main()
