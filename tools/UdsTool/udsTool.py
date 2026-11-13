# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

import functools
import click
from app import connection, rawCommand, services


@click.group(
    help="Use :: udstool [OPTIONS] --help :: for more options on the commands."
)
def start():
    pass


def conn_params(func):
    @click.option("--can", is_flag=True, help="Use CAN connection")
    @click.option(
        "--canif",
        default="socketcan",
        help='python-can interface name. eg. "pcan". Default = "socketcan"',
    )
    @click.option("--channel", help="Channel name for CAN connection")
    @click.option("--txid", help="CAN Tx ID")
    @click.option("--rxid", help="CAN Rx ID")
    @click.option("--config", help="Path to configuration file for CAN connection")
    @click.option("--eth", is_flag=True, help="Use Ethernet connection")
    @click.option("--host", help="Host IP address for Ethernet connection")
    @click.option("--ecu", help="ECU Logical address [hex] for Ethernet connection")
    @click.option(
        "--source", help="Source logical address [hex] for Ethernet connection"
    )
    @click.option(
        "--doip",
        help="DoIP protocol version. Default = 2 (ISO 2012)",
        default=2,
    )
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        if kwargs.get("can"):
            missing = [
                option
                for option in ("channel", "txid", "rxid")
                if not kwargs.get(option)
            ]
            if missing:
                raise click.UsageError(
                    "Missing CAN option(s): " + ", ".join("--" + option for option in missing)
                )
        if kwargs.get("eth"):
            missing = [
                option
                for option in ("host", "ecu", "source")
                if not kwargs.get(option)
            ]
            if missing:
                raise click.UsageError(
                    "Missing Ethernet option(s): " + ", ".join("--" + option for option in missing)
                )
        return func(*args, **kwargs)
    return wrapper


# UDS service Read Data By Identifier (0x22)
@start.command(help="UDS service Read Data By Identifier (0x22)")
@conn_params
@click.option("--did", help="Data Identifier [hex]")
def read(host, ecu, source, did, can, canif, eth, channel, txid, rxid, config, doip):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.read(client, did)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.read(client, did)


# UDS service Write Data By Identifier (0x2e)
@start.command(help="UDS service Write Data By Identifier (0x2e)")
@conn_params
@click.option("--did", help="Data Identifier [hex]")
@click.option("--data", help="Data to write")
def write(
    host, ecu, source, did, data, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.write(client, did, data=data)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.write(client, did, data=data)


# UDS service Diagnostic Session Control (0x10)
@start.command(help="UDS service Diagnostic Session Control (0x10)")
@conn_params
@click.option(
    "--id",
    help="Session Identifier: [1: Default, 2: Programming, 3: Extended]",
)
def session(host, ecu, source, id, can, canif, eth, channel, txid, rxid, config, doip):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.session(client, id)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.session(client, id)


# UDS service Security Access (0x27)
@start.command(help="UDS service Security Access (0x27)")
@conn_params
@click.option("--level", type=int, help="Security Level to be unlocked")
@click.option("--path", help="Shared key path for unlocking security")
def security(
    host, ecu, source, level, path, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.security(client, level, path)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.security(client, level, path)


# UDS service Routine Control (0x31)
@start.command(help="UDS service Routine Control (0x31)")
@conn_params
@click.option("--type", help="Service subfunction")
@click.option("--id", help="Subroutine identifier")
@click.option("--data", help="Routine option record as hexadecimal bytes")
def routine(
    host, ecu, source, type, id, data, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.routine(client, id, type, data=data)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.routine(client, id, type, data=data)


# UDS service Clear Diagnostic Information (0x14)
@start.command(help="UDS service Clear Diagnostic Information (0x14)")
@conn_params
@click.option("--group-of-dtc", default="ffffff", show_default=True, help="Group of DTC [hex, 3 bytes]")
def clear_diagnostic_information(
    host, ecu, source, group_of_dtc, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.clearDiagnosticInformation(client, group_of_dtc)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.clearDiagnosticInformation(client, group_of_dtc)


# UDS service Read DTC Information (0x19)
@start.command(help="UDS service Read DTC Information (0x19)")
@conn_params
@click.option("--subfunction", required=True, help="ReadDTCInformation subfunction [hex]: 02, 04, 08, 0A")
@click.option("--status-mask", help="DTC status mask [hex], required for 02 and 08")
@click.option("--severity-mask", help="DTC severity mask [hex], required for 08")
@click.option("--dtc", help="DTC number [hex, 3 bytes], required for 04")
@click.option("--snapshot-record-number", help="Snapshot record number [hex], required for 04")
def read_dtc_information(
    host, ecu, source, subfunction, status_mask, severity_mask, dtc,
    snapshot_record_number, can, canif, eth, channel, txid, rxid, config, doip
):
    try:
        subfunction_value = int(subfunction, 16)
    except ValueError:
        raise click.BadParameter("must be hexadecimal", param_hint="--subfunction")
    required_options = {
        0x02: ("status-mask",),
        0x04: ("dtc", "snapshot-record-number"),
        0x08: ("severity-mask", "status-mask"),
        0x0A: (),
    }
    if subfunction_value not in required_options:
        raise click.BadParameter("supported values are 02, 04, 08, 0A", param_hint="--subfunction")
    values = {
        "status-mask": status_mask,
        "severity-mask": severity_mask,
        "dtc": dtc,
        "snapshot-record-number": snapshot_record_number,
    }
    for option, value in values.items():
        if value is None:
            if option in required_options[subfunction_value]:
                raise click.BadParameter("required for this subfunction", param_hint="--" + option)
            continue
        if option not in required_options[subfunction_value]:
            raise click.BadParameter("not used by this subfunction", param_hint="--" + option)
        try:
            number = int(value, 16)
        except ValueError:
            raise click.BadParameter("must be hexadecimal", param_hint="--" + option)
        maximum = 0xFFFFFF if option == "dtc" else 0xFF
        if not 0 <= number <= maximum:
            raise click.BadParameter("must be between 0 and {:X}".format(maximum), param_hint="--" + option)

    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.readDtcInformation(
            client, subfunction, status_mask, severity_mask=severity_mask,
            dtc=dtc, snapshot_record_number=snapshot_record_number
        )
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.readDtcInformation(
            client, subfunction, status_mask, severity_mask=severity_mask,
            dtc=dtc, snapshot_record_number=snapshot_record_number
        )


# UDS service Communication Control (0x28)
@start.command(help="UDS service Communication Control (0x28)")
@conn_params
@click.option("--control-type", required=True, help="Communication control type [hex]")
@click.option("--communication-type", required=True, help="Communication type [hex]")
@click.option(
    "--node-id",
    help="Optional node identification number [hex], required for enhanced address information control types",
)
def communication_control(
    host,
    ecu,
    source,
    control_type,
    communication_type,
    node_id,
    can,
    canif,
    eth,
    channel,
    txid,
    rxid,
    config,
    doip,
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.communicationControl(client, control_type, communication_type, node_id)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.communicationControl(client, control_type, communication_type, node_id)


# UDS service Tester Present (0x3e)
@start.command(help="UDS service Tester Present (0x3e)")
@conn_params
@click.option(
    "--subfunction",
    default="00",
    show_default=True,
    help="TesterPresent subfunction [hex]: 00 = positive response, 80 = suppress positive response",
)
def tester_present(
    host, ecu, source, subfunction, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.testerPresent(client, subfunction)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.testerPresent(client, subfunction)


# UDS service Control DTC Setting (0x85)
@start.command(help="UDS service Control DTC Setting (0x85)")
@conn_params
@click.option("--setting-type", required=True, help="DTC setting type [hex]")
@click.option("--data", help="Optional DTC setting control option record as hexadecimal bytes")
def control_dtc_setting(
    host, ecu, source, setting_type, data, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.controlDtcSetting(client, setting_type, data)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.controlDtcSetting(client, setting_type, data)


# UDS service ECU Reset (0x11)
@start.command(help="UDS service ECU Reset (0x11)")
@conn_params
@click.option("--reset-type", required=True, help="Reset type [hex]")
def reset(
    host, ecu, source, reset_type, can, canif, eth, channel, txid, rxid, config, doip
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        services.ecuReset(client, reset_type)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        services.ecuReset(client, reset_type)


# Enter raw command to send
@start.command(help="Enter raw command to send")
@conn_params
@click.option("--data", help="Diagnostic payload to be sent")
@click.option("--path", help="Path to binary")
def raw(
    host,
    ecu,
    source,
    data,
    can,
    canif,
    eth,
    channel,
    txid,
    rxid,
    config,
    doip,
    path=None,
):
    if can:
        # Handle CAN connection
        client = connection.createCanConnection(
            canif, channel, int(txid, 16), int(rxid, 16), config
        )
        rawCommand.raw(client, data, path)
    elif eth:
        # Handle Ethernet connection
        client = connection.createEthConnection(host, ecu, source, doip)
        rawCommand.raw(client, data, path)


if __name__ == "__main__":
    start()
