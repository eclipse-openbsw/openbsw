from can import Bus
from doipclient import DoIPClient
from doipclient.connectors import DoIPClientUDSConnector
from udsoncan.client import Client
from udsoncan.connections import PythonIsoTpConnection
import isotp
import json


def createEthConnection(host, ecu, source):
    try:
        # Create a DoIPClient instance
        doipClient = DoIPClient(
            ecu_ip_address=host,
            ecu_logical_address=int(ecu, 16),
            protocol_version=3,
            client_logical_address=int(source, 16),
        )

        # Create a DoIPClientUDSConnector instance
        udsConnector = DoIPClientUDSConnector(doipClient)

        # Create a Client instance
        client = Client(udsConnector)

        return client

    except Exception as e:
        print(f"Error during UDS operation: {e}")


def createCanConnection(canif, channel, txid, rxid, config):
    try:
        # Load IsoTP parameters
        with open(config, "r") as f:
            isotpParams = json.load(f)

        bus = Bus(interface=canif, channel=channel)

        tp_addr = isotp.Address(
            isotp.AddressingMode.Normal_11bits, txid=txid, rxid=rxid
        )

        # Network layer addressing scheme
        stack = isotp.CanStack(bus=bus, address=tp_addr, params=isotpParams)

        # Network/Transport layer (IsoTP protocol)
        conn = PythonIsoTpConnection(stack)
        conn.open()

        client = Client(conn)
        print(type(client))

        return client

    except Exception as e:
        print(f"Error during UDS operation: {e}")
