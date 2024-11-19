import pytest
import udsoncan
import udsoncan.services as uds
from helpers.helper_functions import hexlify


# Test to check Tester Present functionality
def test_3E00(target_session):
    assert target_session.capserial().wait_for_boot_complete()

    uds_client = target_session.uds_client()
    req = uds.TesterPresent.make_request()
    assert hexlify(uds_client.send_request(req).get_payload()) == "7e 00"
