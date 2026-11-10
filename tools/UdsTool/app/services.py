# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

from app import helper
import binascii
import udsoncan.services as uds
import udsoncan


def read(client, did):
    did = int(did, 16)

    # Create request
    req = uds.ReadDataByIdentifier.make_request([did], {did: udsoncan.AsciiCodec(4)})
    # Send a request to the ECU
    response = client.send_request(req)
    # Print the response
    return helper.handleResponse(response)


def write(client, did, data):
    did = int(did, 16)
    data_len = len(data)

    req = uds.WriteDataByIdentifier().make_request(
        did, data, {did: udsoncan.AsciiCodec(data_len)}
    )
    response = client.send_request(req)

    return helper.handleResponse(response)


def session(client, id):
    id = int(id, 16)
    req = uds.DiagnosticSessionControl().make_request(id)
    response = client.send_request(req)

    return helper.handleResponse(response)


def security(client, level, path):

    # Values used by the UdsOnCan library for requesting Seed
    RequestSeed = 0
    req = uds.SecurityAccess.make_request(level, RequestSeed)

    response = client.send_request(req)
    helper.handleResponse(response)
    seed = helper.extractSeed(response)

    # Values used by the UdsOnCan library
    SendKey = 1
    data = helper.calculateKey(seed, path)
    req = uds.SecurityAccess.make_request(level + 1, SendKey, data)
    response = client.send_request(req)

    return helper.handleResponse(response)


def routine(client, id, type, data=None):
    id = int(id, 16)
    type = int(type)
    if data is not None:
        data = binascii.unhexlify(data)

    req = uds.RoutineControl.make_request(id, type, data)
    response = client.send_request(req)

    return helper.handleResponse(response)


def requestDownload(client, payload):
    method = payload[0:2]
    memSizeLength = int(payload[2])
    addressLength = int(payload[3])
    address = payload[4 : 4 + (2 * addressLength)]
    memSize = payload[4 + (2 * addressLength) :]

    # Create MemoryLocation object
    mem = udsoncan.MemoryLocation(int(address, 16), int(memSize, 16), 32, 32)

    # Create DataIdentifier object
    dfi = udsoncan.DataFormatIdentifier(int(method[0], 16), int(method[1], 16))
    req = uds.RequestDownload.make_request(mem, dfi)
    response = client.send_request(req)

    return helper.handleResponse(response)


def transferData(client, payload, path=None):
    if path is None:
        counter = int(payload[0:2])
        data = binascii.unhexlify(payload[2:])
        req = uds.TransferData.make_request(counter, data)
        response = client.send_request(req)

        return helper.handleResponse(response)

    else:
        counter = int(payload[0:2])
        with open(path, "rb") as infile:
            binary_data = infile.read()
            bundles = helper.divideBinaryIntoBundles(binary_data)

        for i in range(len(bundles)):
            req = uds.TransferData.make_request(counter, bundles[i])
            response = client.send_request(req)
            helper.handleResponse(response)
            counter = (counter + 1) % 256


def transferExit(client):
    req = uds.RequestTransferExit.make_request()
    response = client.send_request(req)

    return helper.handleResponse(response)


def ecuReset(client, reset_type):
    data = int(reset_type, 16)
    req = uds.ECUReset.make_request(data)
    response = client.send_request(req)

    return helper.handleResponse(response)


def clearDiagnosticInformation(client, group_of_dtc='ffffff'):
    group_of_dtc = int(group_of_dtc, 16)
    req = uds.ClearDiagnosticInformation.make_request(group_of_dtc)
    response = client.send_request(req)

    return helper.handleResponse(response)


def readDtcInformation(
    client, subfunction, status_mask=None, severity_mask=None, dtc=None,
    snapshot_record_number=None
):
    subfunction = int(subfunction, 16)
    if status_mask is not None:
        status_mask = int(status_mask, 16)
    if severity_mask is not None:
        severity_mask = int(severity_mask, 16)
    if dtc is not None:
        dtc = int(dtc, 16)
    if snapshot_record_number is not None:
        snapshot_record_number = int(snapshot_record_number, 16)

    req = uds.ReadDTCInformation.make_request(
        subfunction, status_mask=status_mask, severity_mask=severity_mask,
        dtc=dtc, snapshot_record_number=snapshot_record_number
    )
    response = client.send_request(req)

    return helper.handleResponse(response)


def communicationControl(client, control_type, communication_type, node_id=None):
    control_type = int(control_type, 16)
    communication_type = int(communication_type, 16)
    if node_id is not None:
        node_id = int(node_id, 16)

    req = uds.CommunicationControl.make_request(control_type, communication_type, node_id)
    response = client.send_request(req)

    return helper.handleResponse(response)


def testerPresent(client, subfunction="00"):
    subfunction = int(subfunction, 16)
    if not 0 <= subfunction <= 0xFF:
        raise ValueError("TesterPresent subfunction must be a single byte")

    req = uds.TesterPresent.make_request()
    req.subfunction = subfunction & 0x7F
    req.suppress_positive_response = bool(subfunction & 0x80)
    response = client.send_request(req)

    if response is not None:
        return helper.handleResponse(response)


def controlDtcSetting(client, setting_type, data=None):
    setting_type = int(setting_type, 16)
    if data is not None:
        data = binascii.unhexlify(data)

    req = uds.ControlDTCSetting.make_request(setting_type, data)
    response = client.send_request(req)

    return helper.handleResponse(response)
