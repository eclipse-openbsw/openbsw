"""``flync info`` command group: inspect ECUs, controllers, switches, ports, sockets, IPs, SOME/IP services and VLANs."""

from typing import Optional

import typer
from typing_extensions import Annotated

from flync.model.flync_4_someip import SOMEIPServiceConsumer, SOMEIPServiceProvider
from flync_cli.utils.console import console
from flync_cli.utils.deprecation import warn_deprecated
from flync_cli.utils.model_views import (
    ecus_for,
    format_ip,
    iter_ip_assignments,
    require_ecu,
    socket_endpoints_for_ecu,
    socket_ip,
    someip_deployments_by_service,
    vlan_members,
)
from flync_cli.utils.styles import (
    STYLE_CONSUMERS,
    STYLE_CONTROLLER,
    STYLE_ECU,
    STYLE_INDEX,
    STYLE_INTERFACE,
    STYLE_IP,
    STYLE_MAC,
    STYLE_PORT_NO,
    STYLE_PROVIDERS,
    STYLE_ROLE,
    STYLE_SERVICE_ID,
    STYLE_SERVICE_NAME,
    STYLE_VLAN,
    make_table,
)
from flync_cli.utils.workspace import WorkspacePathArg, load_workspace

app = typer.Typer(help="Display FLYNC workspace information in a structured, user-friendly format.")

_COL_ECU_NAME = "ECU Name"
_COL_CONTROLLER_NAME = "Controller Name"
_COL_SWITCH_NAME = "Switch Name"

EcuNameOpt = Annotated[Optional[str], typer.Option("--ecu-name", "-e", help="Optional: filter info for a specific ECU name.")]


def _parse_service_id(value: str) -> int:
    """Parse a service id argument, accepting decimal or hex (``0x0101``) notation."""
    try:
        return int(value, 0)
    except ValueError as exc:
        raise typer.BadParameter(f"Expected an integer (decimal or hex, e.g. 0x0101): {value!r}") from exc


def _resolve_service_by_name(model, name: str) -> tuple[int, int]:
    """Return ``(service_id, major_version)`` for the SOME/IP service named *name*, or exit 1 if none matches."""
    for service in model.get_all_someip_services():
        if service.name == name:
            return service.id, service.major_version
    console.print(f"⚠️ [bold red] No SOME/IP service named '{name}' in this workspace.[/bold red]")
    raise typer.Exit(code=1)


# ---------------------------------------------------------------------------
# ecus / controllers / switches
# ---------------------------------------------------------------------------


def _print_named_list(items, tag):
    """Print a two-column Rich table listing items with their row number under the given column tag."""
    table = make_table()
    table.add_column("Num.", justify="right", style=STYLE_INDEX)
    table.add_column(tag, style=STYLE_ECU)
    for idx, item in enumerate(items, 1):
        table.add_row(str(idx), item)
    console.print(table)


def _print_all_ecus_grouped(ecu_list, tag, member_fn):
    """Print a Rich table showing every ECU alongside its controllers/switches."""
    table = make_table()
    table.add_column("Num.", justify="right", style=STYLE_INDEX)
    table.add_column(_COL_ECU_NAME, style=STYLE_ECU)
    table.add_column(tag, style=STYLE_INTERFACE)
    for idx, ecu in enumerate(ecu_list, 1):
        for member in member_fn(ecu):
            table.add_row(str(idx), ecu.name, member)
    console.print(table)


def _print_one_ecu(ecu, tag, member_fn):
    """Print a Rich table of one component type for a single ECU."""
    table = make_table()
    table.add_column(_COL_ECU_NAME, justify="right", style=STYLE_ECU)
    table.add_column(tag, style=STYLE_INTERFACE)
    for member in member_fn(ecu):
        table.add_row(ecu.name, member)
    console.print(table)


def _show_ecus(model) -> None:
    """Print a list of all ECU names."""
    _print_named_list(model.get_all_ecus(), _COL_ECU_NAME)


@app.command(name="ecus", help="List every ECU in the workspace.")
def ecus(path: WorkspacePathArg = None):
    """List every ECU in the workspace."""
    ws = load_workspace(path)
    _show_ecus(ws.flync_model)


def _controller_names(ecu):
    """Get a list of all Controller names."""
    return [c.name for c in ecu.get_all_controllers()]


def _show_controllers(model, ecu_name: Optional[str]) -> None:
    """Print out a list of all Controller names."""
    if ecu_name is not None:
        _print_one_ecu(require_ecu(model, ecu_name), _COL_CONTROLLER_NAME, _controller_names)
    else:
        _print_all_ecus_grouped(model.ecus, _COL_CONTROLLER_NAME, _controller_names)


@app.command(name="controllers", help="List the controllers of every ECU, or of one ECU with --ecu-name.")
def controllers(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """List the controllers of every ECU, or of one ECU with --ecu-name."""
    ws = load_workspace(path)
    _show_controllers(ws.flync_model, ecu_name)


def _switch_names(ecu):
    """Get a list of all Controller names."""
    return [s.name for s in ecu.get_all_switches()]


def _show_switches(model, ecu_name: Optional[str]) -> None:
    """Print out a list of all Switch names."""
    if ecu_name is not None:
        _print_one_ecu(require_ecu(model, ecu_name), _COL_SWITCH_NAME, _switch_names)
    else:
        _print_all_ecus_grouped(model.ecus, _COL_SWITCH_NAME, _switch_names)


@app.command(name="switches", help="List the switches of every ECU, or of one ECU with --ecu-name.")
def switches(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """List the switches of every ECU, or of one ECU with --ecu-name."""
    ws = load_workspace(path)
    _show_switches(ws.flync_model, ecu_name)


# ---------------------------------------------------------------------------
# ports
# ---------------------------------------------------------------------------


def _show_ports(model, ecu_name: Optional[str]) -> None:
    """Print out a table of all ECU port names."""
    for ecu in ecus_for(model, ecu_name):
        port_names = [p.name for p in ecu.get_all_ports()]
        if not port_names:
            if ecu_name is not None:
                console.print(f"ECU '{ecu.name}' has no ports configured.")
            continue
        table = make_table(title=f"ECU = {ecu.name}")
        table.add_column("Port Name", style=STYLE_INTERFACE)
        for name in port_names:
            table.add_row(name)
        console.print(table)


@app.command(name="ports", help="List the ports of every ECU, grouped by ECU, or of one ECU with --ecu-name.")
def ports(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """List the ports of every ECU, grouped by ECU, or of one ECU with --ecu-name."""
    ws = load_workspace(path)
    _show_ports(ws.flync_model, ecu_name)


# ---------------------------------------------------------------------------
# ip
# ---------------------------------------------------------------------------


def _show_ip(model, ecu_name: Optional[str]) -> None:
    """Print out a list of all IP addresses."""
    assignments = list(iter_ip_assignments(model, ecu_name))
    if not assignments:
        console.print("No IP addresses configured in this workspace." if ecu_name is None else f"ECU '{ecu_name}' has no IP addresses configured.")
        return

    table = make_table()
    table.add_column(_COL_ECU_NAME, style=STYLE_ECU)
    table.add_column(_COL_CONTROLLER_NAME, style=STYLE_CONTROLLER)
    table.add_column("Interface", style=STYLE_INTERFACE)
    table.add_column("Virtual Interface", style=STYLE_INTERFACE)
    table.add_column("VLAN", style=STYLE_VLAN)
    table.add_column("IP Address", style=STYLE_IP)
    for a in assignments:
        table.add_row(
            a.ecu.name,
            a.controller.name,
            a.eth_iface.name,
            a.vci.name,
            str(a.vci.vlanid) if a.vci.vlanid is not None else "untagged",
            format_ip(a.address),
        )
    console.print(table)


@app.command(name="ip", help="List IP addresses across every ECU, with their VLAN and subnet.")
def ip(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """List IP addresses across every ECU, with their VLAN and subnet."""
    ws = load_workspace(path)
    _show_ip(ws.flync_model, ecu_name)


# ---------------------------------------------------------------------------
# sockets
# ---------------------------------------------------------------------------


def _print_socket_tables(ecu, endpoints) -> None:
    """Build a table of all sockets."""
    by_vlan: dict = {}
    for endpoint in endpoints:
        by_vlan.setdefault(endpoint.container.vlan_id, []).append(endpoint)

    for vlan_id in sorted(by_vlan, key=lambda v: (v is None, v)):
        label = vlan_id if vlan_id is not None else "untagged"
        table = make_table(title=f"ECU = {ecu.name}    VLAN {label}")
        table.add_column("Interface", style=STYLE_INTERFACE)
        table.add_column("Virtual Interface", style=STYLE_INTERFACE)
        table.add_column("MAC", style=STYLE_MAC)
        table.add_column("IP", style=STYLE_IP)
        table.add_column("Protocol", style=STYLE_VLAN)
        table.add_column("Port", style=STYLE_PORT_NO)
        table.add_column("Socket", style=STYLE_INDEX)
        for endpoint in by_vlan[vlan_id]:
            table.add_row(
                endpoint.eth_iface.name,
                endpoint.vci.name if endpoint.vci is not None else "-",
                str(endpoint.mac) if endpoint.mac is not None else "-",
                socket_ip(endpoint.socket, endpoint.vci),
                endpoint.socket.protocol.upper(),
                str(endpoint.socket.port_no),
                endpoint.socket.name,
            )
        console.print(table)


def _show_sockets(model, ecu_name: Optional[str]) -> None:
    """Print out a list of all Socket Endpoints."""
    printed_any = False
    for ecu in ecus_for(model, ecu_name):
        endpoints = list(socket_endpoints_for_ecu(ecu))
        if not endpoints:
            if ecu_name is not None:
                console.print(f"ECU '{ecu.name}' has no socket endpoints configured.")
            continue
        printed_any = True
        _print_socket_tables(ecu, endpoints)

    if not printed_any and ecu_name is None:
        console.print("No socket endpoints configured in this workspace.")


@app.command(name="sockets", help="Show socket endpoints grouped by ECU and VLAN, or of one ECU with --ecu-name.")
def sockets(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Show socket endpoints grouped by ECU and VLAN, or of one ECU with --ecu-name."""
    ws = load_workspace(path)
    _show_sockets(ws.flync_model, ecu_name)


# ---------------------------------------------------------------------------
# services / instances
# ---------------------------------------------------------------------------


def _show_services(model) -> None:
    """Print out a list of all SOME/IP services."""
    services = model.get_all_someip_services()
    if not services:
        console.print("No SOME/IP configuration in this workspace.")
        return

    deployments = someip_deployments_by_service(model)
    table = make_table()
    table.add_column("Service Name", style=STYLE_SERVICE_NAME)
    table.add_column("Service ID", style=STYLE_SERVICE_ID)
    table.add_column("Major Version", style=STYLE_SERVICE_ID)
    table.add_column("Providers", style=STYLE_PROVIDERS)
    table.add_column("Consumers", style=STYLE_CONSUMERS)
    for service in services:
        entries = deployments.get((service.id, service.major_version), [])
        providers = sorted({endpoint.ecu.name for endpoint, dep in entries if isinstance(dep, SOMEIPServiceProvider)})
        consumers = sorted({endpoint.ecu.name for endpoint, dep in entries if isinstance(dep, SOMEIPServiceConsumer)})
        table.add_row(service.name, f"{service.id:#06x}", str(service.major_version), ", ".join(providers) or "-", ", ".join(consumers) or "-")
    console.print(table)


@app.command(name="services", help="List every SOME/IP service, its ID/version, and its providing/consuming ECUs.")
def services(path: WorkspacePathArg = None):
    """List every SOME/IP service, its ID/version, and its providing/consuming ECUs."""
    ws = load_workspace(path)
    _show_services(ws.flync_model)


def _show_instances(model, service_id: int, major_version: int) -> None:
    """Print out a list of all Service Instances."""
    key = (service_id, major_version)
    services_by_key = model.get_someip_services_by_identity()
    service = services_by_key.get(key)
    if service is None:
        console.print(f"⚠️ [bold red] No SOME/IP service with id={service_id:#06x} and major_version={major_version} in this workspace.[/bold red]")
        if services_by_key:
            console.print("[dim]Available services:[/dim]")
            for (svc_id, major), svc in sorted(services_by_key.items()):
                console.print(f"  [cyan]{svc.name}[/cyan]  id={svc_id:#06x}  major_version={major}")
        raise typer.Exit(code=1)

    deployments = someip_deployments_by_service(model).get(key, [])
    table = make_table(title=f"Service {service.name}  (id={service.id:#06x}, major_version={service.major_version})")
    table.add_column("ECU", style=STYLE_ECU)
    table.add_column("Controller", style=STYLE_CONTROLLER)
    table.add_column("Interface", style=STYLE_INTERFACE)
    table.add_column("Role", style=STYLE_ROLE)
    table.add_column("VLAN", style=STYLE_VLAN)
    table.add_column("IP", style=STYLE_IP)
    table.add_column("Port", style=STYLE_PORT_NO)
    table.add_column("Instance ID", style=STYLE_SERVICE_ID)
    for endpoint, deployment in deployments:
        role = "Provider" if isinstance(deployment, SOMEIPServiceProvider) else "Consumer"
        vlan = endpoint.container.vlan_id
        table.add_row(
            endpoint.ecu.name,
            endpoint.controller.name,
            endpoint.eth_iface.name,
            role,
            str(vlan) if vlan is not None else "untagged",
            socket_ip(endpoint.socket, endpoint.vci),
            str(endpoint.socket.port_no),
            str(deployment.instance_id),
        )
    console.print(table)
    if not deployments:
        console.print("[dim]No consumer/provider deployments found for this service instance.[/dim]")


@app.command(name="instances", help="Show the consumer/provider deployments of one SOME/IP service instance.")
def instances(
    service_id: Annotated[str, typer.Argument(help="Service ID, decimal or hex (e.g. 0x0101).")],
    major_version: Annotated[int, typer.Argument(help="Major version of the service interface.")],
    path: WorkspacePathArg = None,
):
    """Show the consumer/provider deployments of one SOME/IP service instance."""
    ws = load_workspace(path)
    _show_instances(ws.flync_model, _parse_service_id(service_id), major_version)


# ---------------------------------------------------------------------------
# vlans
# ---------------------------------------------------------------------------


def _resolve_vlan_ids(grouped: dict, ecu_name: Optional[str], vlan_id: Optional[int]) -> Optional[list]:
    """Return the VLAN ids to display, or None if there is nothing to show (already reported)."""
    if vlan_id is not None:
        if vlan_id not in grouped:
            scope = f" for ECU '{ecu_name}'" if ecu_name else ""
            console.print(f"⚠️ [bold red] VLAN {vlan_id} is not configured{scope} in this workspace.[/bold red]")
            raise typer.Exit(code=1)
        return [vlan_id]

    if not grouped:
        console.print("No VLANs configured in this workspace.")
        return None
    return sorted(grouped, key=lambda v: (v is None, v))


def _add_vlan_member_rows(table, members) -> None:
    """Add one table row per VLAN member."""
    for member in members:
        ips = "\n".join(member.ips) if member.ips else "-"
        table.add_row(member.ecu.name, member.component_name, member.component_type, ips)


def _show_vlans(model, ecu_name: Optional[str], vlan_id: Optional[int]) -> None:
    """Print out a list of all VLAN configurations."""
    grouped = vlan_members(model, ecu_name)
    vlan_ids = _resolve_vlan_ids(grouped, ecu_name, vlan_id)
    if vlan_ids is None:
        return

    for vid in vlan_ids:
        label = vid if vid is not None else "untagged"
        table = make_table(title=f"VLAN {label}")
        table.add_column(_COL_ECU_NAME, style=STYLE_ECU)
        table.add_column("Component", style=STYLE_INTERFACE)
        table.add_column("Type", style=STYLE_VLAN)
        table.add_column("IPs", style=STYLE_IP)
        _add_vlan_member_rows(table, grouped[vid])
        console.print(table)


@app.command(name="vlans", help="Show VLAN membership grouped by VLAN, across the workspace or one ECU.")
def vlans(
    path: WorkspacePathArg = None,
    vlan_id: Annotated[Optional[int], typer.Option("--vlan-id", help="Optional: show only this VLAN.")] = None,
    ecu_name: EcuNameOpt = None,
):
    """Show VLAN membership grouped by VLAN, across the workspace or one ECU."""
    ws = load_workspace(path)
    _show_vlans(ws.flync_model, ecu_name, vlan_id)


# ---------------------------------------------------------------------------
# Deprecated aliases (hidden from --help)
# ---------------------------------------------------------------------------


@app.command(name="list-ecus", hidden=True, deprecated=True)
def _list_ecus_alias(path: WorkspacePathArg = None):
    """Deprecated alias for ``flync info ecus``."""
    warn_deprecated("info list-ecus", "info ecus")
    ws = load_workspace(path)
    _show_ecus(ws.flync_model)


@app.command(name="list-controllers", hidden=True, deprecated=True)
def _list_controllers_alias(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Deprecated alias for ``flync info controllers``."""
    warn_deprecated("info list-controllers", "info controllers")
    ws = load_workspace(path)
    _show_controllers(ws.flync_model, ecu_name)


@app.command(name="list-switches", hidden=True, deprecated=True)
def _list_switches_alias(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Deprecated alias for ``flync info switches``."""
    warn_deprecated("info list-switches", "info switches")
    ws = load_workspace(path)
    _show_switches(ws.flync_model, ecu_name)


@app.command(name="list-ports", hidden=True, deprecated=True)
def _list_ports_alias(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Deprecated alias for ``flync info ports``."""
    warn_deprecated("info list-ports", "info ports")
    ws = load_workspace(path)
    _show_ports(ws.flync_model, ecu_name)


@app.command(name="list-ips", hidden=True, deprecated=True)
def _list_ips_alias(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Deprecated alias for ``flync info ip``."""
    warn_deprecated("info list-ips", "info ip")
    ws = load_workspace(path)
    _show_ip(ws.flync_model, ecu_name)


@app.command(name="list-sockets", hidden=True, deprecated=True)
def _list_sockets_alias(path: WorkspacePathArg = None, ecu_name: EcuNameOpt = None):
    """Deprecated alias for ``flync info sockets``."""
    warn_deprecated("info list-sockets", "info sockets")
    ws = load_workspace(path)
    _show_sockets(ws.flync_model, ecu_name)


@app.command(name="list-services", hidden=True, deprecated=True)
def _list_services_alias(path: WorkspacePathArg = None):
    """Deprecated alias for ``flync info services``."""
    warn_deprecated("info list-services", "info services")
    ws = load_workspace(path)
    _show_services(ws.flync_model)
