import click
from app import application


@click.group()
def start():
    pass


@start.command(help="UDS service Read Data By Identifier (0x22)")
@click.option("--can", is_flag=True, help="Use CAN connection")
@click.option("--eth", is_flag=True, help="Use Ethernet connection")
@click.option("--host", help="Host IP address for Ethernet connection")
@click.option("--ecu", help="ECU Logical address [hex] for Ethernet connection")
@click.option("--source", help="Source logical address [hex] for Ethernet connection")
@click.option("--did", help="Data Identifier [hex]")
@click.option("--channel", help="Channel name for CAN connection")
@click.option("--txid", help="CAN Tx ID")
@click.option("--rxid", help="CAN Rx ID")
@click.option("--config", help="Path to configuration file for CAN connection")
def read(host, ecu, source, did, can, eth, channel, txid, rxid, config):
    if can:
        # Handle CAN connection
        client = application.createCanConnection(
            channel, int(txid, 16), int(rxid, 16), config
        )
        application.read(client, did)
    elif eth:
        # Handle Ethernet connection
        client = application.createEthConnection(host, ecu, source)
        application.read(client, did)
