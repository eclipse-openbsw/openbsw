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

    # Create request
    req = uds.WriteDataByIdentifier().make_request(
        did, data, {did: udsoncan.AsciiCodec(data_len)}
    )

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)


def session(client, id):
    id = int(id, 16)

    req = uds.DiagnosticSessionControl().make_request(id)

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)


def security(client, level, path):

    # Values used by the UdsOnCan library for requesting Seed
    RequestSeed = 0
    req = uds.SecurityAccess.make_request(level, RequestSeed)

    # Send a request to the ECU
    response = client.send_request(req)

    helper.handleResponse(response)
    seed = helper.extractSeed(response)

    # Values used by the UdsOnCan library
    SendKey = 1
    data = helper.calculateKey(seed, path)
    req = uds.SecurityAccess.make_request(level + 1, SendKey, data)

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)


def routine(client, id, type, data=None):
    id = int(id, 16)
    type = int(type)
    if data is not None:
        data = binascii.unhexlify(data)

    # Create request
    req = uds.RoutineControl.make_request(id, type, data)

    # Send a request to the ECU
    response = client.send_request(req)

    if response.get_payload()[4] != 0:
        payload = binascii.hexlify(response.get_payload()).decode("ascii")
        raise helper.RoutineFailedException(f"Routine {hex(id)} failed: {payload}")

    # Print the response
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

    # Create request
    req = uds.RequestDownload.make_request(mem, dfi)

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)


def transferData(client, payload, path=None):
    if path is None:
        counter = int(payload[0:2])
        data = binascii.unhexlify(payload[2:])

        # Create request
        req = uds.TransferData.make_request(counter, data)

        # Send a request to the ECU
        response = client.send_request(req)
        # Print the response
        return helper.handleResponse(response)

    else:
        counter = int(payload[0:2])
        with open(path, "rb") as infile:
            binary_data = infile.read()
            bundles = helper.divideBinaryIntoBundles(binary_data)

        for i in range(len(bundles)):
            req = uds.TransferData.make_request(counter, bundles[i])
            # Send a request to the ECU
            response = client.send_request(req)

            # Print the response
            helper.handleResponse(response)
            counter = (counter + 1) % 256


def transferExit(client):
    # Create request
    req = uds.RequestTransferExit.make_request()

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)


def ecuReset(client, data):
    data = int(data)

    # Create request
    req = uds.ECUReset.make_request(data)

    # Send a request to the ECU
    response = client.send_request(req)

    # Print the response
    return helper.handleResponse(response)
