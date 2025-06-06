import functools
import click
from app import connection, rawCommand, services


@click.group()
def start():
    pass


def conn_params(func):
    @click.option("--can", is_flag=True, help="Use CAN connection")
    @click.option(
        "--canif",
        default="socketcan",
        help='python-can interface name. eg. "pcan". Default = "socketcan"',
    )
    @click.option("--eth", is_flag=True, help="Use Ethernet connection")
    @click.option("--host", help="Host IP address for Ethernet connection")
    @click.option("--ecu", help="ECU Logical address [hex] for Ethernet connection")
    @click.option(
        "--source", help="Source logical address [hex] for Ethernet connection"
    )
    @click.option("--channel", help="Channel name for CAN connection")
    @click.option("--txid", help="CAN Tx ID")
    @click.option("--rxid", help="CAN Rx ID")
    @click.option("--config", help="Path to configuration file for CAN connection")
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        return func(*args, **kwargs)

    return wrapper


# UDS service Read Data By Identifier (0x22)
@start.command(help="UDS service Read Data By Identifier (0x22)")
@conn_params
@click.option("--did", help="Data Identifier [hex]")
def read(host, ecu, source, did, can, canif, eth, channel, txid, rxid, config):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.read(client, did)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        services.read(client, did)


# UDS service Write Data By Identifier (0x2e)
@start.command(help="UDS service Write Data By Identifier (0x2e)")
@conn_params
@click.option("--did", help="Data Identifier [hex]")
@click.option("--data", help="Data to write")
def write(host, ecu, source, did, data, can, canif, eth, channel, txid, rxid, config):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.write(client, did, data=data)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        services.write(client, did, data=data)


# UDS service Diagnostic Session Control (0x10)
@start.command(help="UDS service Diagnostic Session Control (0x10)")
@conn_params
@click.option(
    "--id",
    help="Session Identifier: [1: Default, 2: Programming, 3: Extended]",
)
def session(host, ecu, source, id, can, canif, eth, channel, txid, rxid, config):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.session(client, id)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        services.session(client, id)


# UDS service Security Access (0x27)
@start.command(help="UDS service Security Access (0x27)")
@conn_params
@click.option("--level", type=int, help="Security Level to be unlocked")
@click.option("--path", help="Shared key path for unlocking security")
def security(
    host, ecu, source, level, path, can, canif, eth, channel, txid, rxid, config
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.security(client, level, path)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        services.security(client, level, path)


# UDS service Routine Control (0x31)
@start.command(help="UDS service Routine Control (0x31)")
@conn_params
@click.option("--type", help="Service subfunction")
@click.option("--id", help="Subroutine identifier")
def routine(host, ecu, source, type, id, can, canif, eth, channel, txid, rxid, config):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.routine(client, id, type)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        services.routine(client, id, type)


# Enter raw command to send
@start.command(help="Enter raw command to send")
@conn_params
@click.option("--data", help="Diagnostic payload to be sent")
@click.option("--path", help="Path to binary")
def raw(
    host, ecu, source, data, can, canif, eth, channel, txid, rxid, config, path=None
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        rawCommand.raw(client, data, path)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source)
        rawCommand.raw(client, data, path)
