#Commenting out as the DIDs are not present in the Source code.
""" import pytest
import udsoncan
import udsoncan.services as uds
from udsoncan.exceptions import NegativeResponseException
from helpers.helper_functions import hexlify, ByteCodec


# Test: Checking positive response for did f100
def test_WDBI_F100(target_session):
    uds_client = target_session.uds_client()
    did = 0xF100
    val = "5547291a"
    didconfig = {0xF100: ByteCodec()}
    req = uds.WriteDataByIdentifier().make_request(did, bytes.fromhex(val), didconfig)
    assert hexlify(uds_client.send_request(req).get_payload()[0:3]) == "6e f1 00"


# Test: Checking positive response for did F190 (Writing 17 bytes of data)
def test_WDBI_F190(target_session):
    uds_client = target_session.uds_client()
    did = 0xF190
    val = "ff001213125263975630ffbb5636410099"
    didconfig = {0xF190: ByteCodec()}
    req = uds.WriteDataByIdentifier().make_request(did, bytes.fromhex(val), didconfig)
    assert hexlify(uds_client.send_request(req).get_payload()[0:3]) == "6e f1 90"


# Test: Checking NRC ISO_INVALID_FORMAT is received
def test_ISO_INVALID_FORMAT(target_session):
    uds_client = target_session.uds_client()
    did = 0xF190
    val = "361F"
    didconfig = {0xF190: ByteCodec()}
    req = uds.WriteDataByIdentifier().make_request(did, bytes.fromhex(val), didconfig)
    with pytest.raises(udsoncan.exceptions.NegativeResponseException) as exc_info:
        uds_client.send_request(req)
    assert hexlify(exc_info.value.response.get_payload()) == "7f 2e 13"


# Test: Checking NRC ISO_REQUEST_OUT_OF_RANGE is received
def test_ISO_REQUEST_OUT_OF_RANGE(target_session):
    uds_client = target_session.uds_client()
    did = 0xF191
    val = "361F1245"
    didconfig = {0xF191: ByteCodec()}
    req = uds.WriteDataByIdentifier().make_request(did, bytes.fromhex(val), didconfig)
    with pytest.raises(udsoncan.exceptions.NegativeResponseException) as exc_info:
        uds_client.send_request(req)
    assert hexlify(exc_info.value.response.get_payload()) == "7f 2e 31" """
