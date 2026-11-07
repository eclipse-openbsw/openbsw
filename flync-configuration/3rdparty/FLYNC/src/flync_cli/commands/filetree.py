"""``flync filetree`` command: exports the expected filetree of a FLYNC configuration to a txt file."""

from typing import Optional

import typer
from pydantic import BaseModel
from typing_extensions import Annotated

from flync.model import FLYNCModel
from flync.model.flync_4_communication import FLYNCChannelConfig, FLYNCCommunicationConfig
from flync.model.flync_4_diagnostics import DiagnosticsConfig, DoIPConfig, UDSConfig
from flync.model.flync_4_ecu import ECU, ComputeNode, Controller, EthernetInterface
from flync.model.flync_4_someip import SOMEIPConfig
from flync.model.flync_4_topology import FLYNCTopology
from flync.sdk.helpers.debug import print_field_subtree, print_flync_structure
from flync_cli.utils.console import console

app = typer.Typer()

# Each value is either:
#   - a BaseModel subclass  →  passed directly to print_flync_structure
#   - a (parent_cls, field_name) tuple  →  passed to print_field_subtree
#     (used for leaf item types that have no External sub-fields of their own)
_CLASS_MAP: dict[str, type[BaseModel] | tuple[type[BaseModel], str]] = {
    # ── root ──────────────────────────────────────────────────────────
    "flync_model": FLYNCModel,
    # ── ECU tree ──────────────────────────────────────────────────────
    "ecu": ECU,
    "ports": (ECU, "ports"),
    "switches": (ECU, "switches"),
    "ecu_topology": (ECU, "topology"),
    "ecu_metadata": (ECU, "ecu_metadata"),
    "mac_multicast_endpoints": (ECU, "mac_multicast_endpoints"),
    "controller": Controller,
    "controller_metadata": (Controller, "controller_metadata"),
    "ethernet_interface": EthernetInterface,
    "interface_config": (EthernetInterface, "interface_config"),
    "sockets": (EthernetInterface, "sockets"),
    "can_interfaces": (Controller, "can_interfaces"),
    "lin_interfaces": (Controller, "lin_interfaces"),
    "compute_node": ComputeNode,
    "compute_nodes": (Controller, "compute_nodes"),
    "virtual_switches": (Controller, "switches"),
    "controller_topology": (Controller, "controller_topology"),
    # ── communication tree ────────────────────────────────────────────
    "communication": FLYNCCommunicationConfig,
    "tcp_profiles": (FLYNCCommunicationConfig, "tcp_profiles"),
    "someip": SOMEIPConfig,
    "sd_config": (SOMEIPConfig, "sd_config"),
    "services": (SOMEIPConfig, "services"),
    "someip_timings": (SOMEIPConfig, "someip_timings"),
    "diagnostics": DiagnosticsConfig,
    "doip": DoIPConfig,
    "doip_timings": (DoIPConfig, "timings"),
    "uds": UDSConfig,
    "uds_timings": (UDSConfig, "timings"),
    "uds_servers": (UDSConfig, "servers"),
    "dids": (UDSConfig, "dids"),
    "routines": (UDSConfig, "routines"),
    "dtcs": (UDSConfig, "dtcs"),
    "channels": FLYNCChannelConfig,
    "pdus": (FLYNCChannelConfig, "pdus"),
    "can": (FLYNCChannelConfig, "can_buses"),
    "lin": (FLYNCChannelConfig, "lin_buses"),
    "ethernet_pdu_containers": (FLYNCChannelConfig, "ethernet_pdu_containers"),
    # ── topology ──────────────────────────────────────────────────────
    "topology": FLYNCTopology,
    "system_topology": (FLYNCTopology, "system_topology"),
    # ── metadata ──────────────────────────────────────────────────────
    "metadata": (FLYNCModel, "metadata"),
}


def _show_filetree(cls_name: Optional[str]):
    """Write the field structure of a FLYNC model or one of its named sub-trees to a txt file and return its path.

    With no ``cls_name``, renders the full FLYNCModel. Otherwise looks up ``cls_name`` in ``_CLASS_MAP`` and
    renders either the mapped BaseModel subclass directly or, for leaf item types, the named field of the
    mapped parent class.
    """
    if cls_name is None:
        out = print_flync_structure()
    else:
        entry = _CLASS_MAP.get(cls_name)
        if entry is None:
            console.print(f"[red]Unknown class '{cls_name}'. Choose from: {', '.join(_CLASS_MAP)}[/red]")
            raise typer.Exit(code=1)

        if isinstance(entry, tuple):
            parent_cls, field_name = entry
            out = print_field_subtree(parent_cls, field_name)
        else:
            out = print_flync_structure(model_cls=entry)

    console.print("Filetree exported to ", out)
    return out


@app.command(help="Export the expected filetree of a FLYNC configuration to a txt file.")
def filetree(
    cls_name: Annotated[
        Optional[str],
        typer.Option(
            "--class",
            help=("Sub-tree to visualise. " f"Choices: {', '.join(_CLASS_MAP)}. " "Defaults to the full FLYNCModel."),
        ),
    ] = None,
):
    """Export the expected filetree of a FLYNC model or one of its named sub-trees."""
    _show_filetree(cls_name)
