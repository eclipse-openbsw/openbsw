"""
Shared traversal helpers for the ``flync info`` reports.

Every report needs the same walk: ECU -> controller (including switch host controllers, which
``ECU.get_all_interfaces()`` does not cover) -> ethernet interface -> virtual interface / socket
container. These generators do that walk once so each report only has to format the result.
"""

from collections import defaultdict, namedtuple
from ipaddress import IPv4Network, IPv6Network
from typing import Iterator, Optional

import typer

from flync.core.datatypes.ipaddress import IPv4AddressEntry
from flync.model.flync_4_someip import SOMEIPServiceConsumer, SOMEIPServiceProvider
from flync_cli.utils.console import console

SocketEndpoint = namedtuple("SocketEndpoint", "ecu controller eth_iface container socket vci mac")
IPAssignment = namedtuple("IPAssignment", "ecu controller eth_iface vci address")
VlanMember = namedtuple("VlanMember", "ecu component_name component_type ips")


def require_ecu(model, ecu_name: str):
    """Return the named ECU, or exit 1 with a clear message if it does not exist in the model."""
    ecu = model.get_ecu_by_name(ecu_name)
    if ecu is None:
        console.print(f"⚠️ [bold red] ECU '{ecu_name}' does not exist in this FLYNC model.[/bold red]")
        raise typer.Exit(code=1)
    return ecu


def ecus_for(model, ecu_name: Optional[str]) -> list:
    """Return ``[the named ECU]`` if *ecu_name* is given (exiting 1 if it does not exist), else every ECU."""
    return [require_ecu(model, ecu_name)] if ecu_name else list(model.ecus)


def _with_subtree_compute_nodes(controller) -> Iterator:
    """Yield *controller* itself, then every compute node nested beneath it."""
    yield controller
    yield from controller.iter_subtree_compute_nodes()


def iter_ecu_controllers(ecu) -> Iterator:
    """
    Yield every interface owner of *ecu*.

    That is every controller, every switch host controller (a second source of interfaces), and every
    compute node nested beneath either — a compute node owns interfaces with their own addresses, so
    the reports would silently omit them otherwise. Controllers and compute nodes both expose ``name``
    and ``get_interfaces()``, which is all the callers need.
    """
    seen = set()
    for ctrl in ecu.controllers:
        seen.add(id(ctrl))
        yield from _with_subtree_compute_nodes(ctrl)
    for switch in ecu.get_all_switches():
        host = switch.host_controller
        if host is not None and id(host) not in seen:
            seen.add(id(host))
            yield from _with_subtree_compute_nodes(host)


def iter_ecu_interfaces(ecu) -> Iterator[tuple]:
    """Yield ``(owner, eth_iface)`` for every ethernet interface reachable from *ecu*, where the owner is a controller or a compute node."""
    for ctrl in iter_ecu_controllers(ecu):
        for eth_iface in ctrl.get_interfaces():
            yield ctrl, eth_iface


def iter_virtual_interfaces(iface_config) -> Iterator[tuple]:
    """Yield ``(virtual_interface, mac_address)`` for every VCI stacked on this interface."""
    for vci in iface_config.virtual_interfaces or []:
        yield vci, iface_config.mac_address


def _vci_and_mac_for_vlan(iface_config, vlan_id):
    """Return the (virtual_interface, mac_address) matching *vlan_id* on this interface, or (None, interface MAC)."""
    for vci, mac in iter_virtual_interfaces(iface_config):
        if vci.vlanid == vlan_id:
            return vci, mac
    return None, iface_config.mac_address


def socket_endpoints_for_ecu(ecu) -> Iterator[SocketEndpoint]:
    """Yield a :class:`SocketEndpoint` for every socket configured anywhere in *ecu*."""
    for ctrl, eth_iface in iter_ecu_interfaces(ecu):
        iface_config = eth_iface.interface_config
        for container in eth_iface.sockets or []:
            vci, mac = _vci_and_mac_for_vlan(iface_config, container.vlan_id)
            for socket in container.sockets or []:
                yield SocketEndpoint(ecu, ctrl, eth_iface, container, socket, vci, mac)


def iter_socket_endpoints(model, ecu_name: Optional[str] = None) -> Iterator[SocketEndpoint]:
    """Yield every :class:`SocketEndpoint` in the model, optionally restricted to one ECU."""
    for ecu in ecus_for(model, ecu_name):
        yield from socket_endpoints_for_ecu(ecu)


def ip_assignments_for_ecu(ecu) -> Iterator[IPAssignment]:
    """Yield an :class:`IPAssignment` for every address configured on any virtual interface of *ecu*."""
    for ctrl, eth_iface in iter_ecu_interfaces(ecu):
        for vci, _mac in iter_virtual_interfaces(eth_iface.interface_config):
            for address in vci.addresses:
                yield IPAssignment(ecu, ctrl, eth_iface, vci, address)


def iter_ip_assignments(model, ecu_name: Optional[str] = None) -> Iterator[IPAssignment]:
    """Yield every :class:`IPAssignment` in the model, optionally restricted to one ECU."""
    for ecu in ecus_for(model, ecu_name):
        yield from ip_assignments_for_ecu(ecu)


def subnet_for(address_entry) -> str:
    """Return the subnet (CIDR notation) an IPv4/IPv6 address entry belongs to."""
    if isinstance(address_entry, IPv4AddressEntry):
        return str(IPv4Network(f"{address_entry.address}/{address_entry.ipv4netmask}", strict=False))
    return str(IPv6Network(f"{address_entry.address}/{address_entry.ipv6prefix}", strict=False))


def format_ip(address_entry) -> str:
    """Return ``<address>/<prefix>`` for an IPv4 or IPv6 address entry.

    This is the canonical display format for every IP address shown in CLI tables.
    The prefix length is derived from the subnet mask (IPv4) or directly from the
    prefix field (IPv6), so the host bits are preserved — e.g. ``192.168.1.10/24``
    rather than the network address ``192.168.1.0/24``.
    """
    if isinstance(address_entry, IPv4AddressEntry):
        prefix = IPv4Network(f"0.0.0.0/{address_entry.ipv4netmask}").prefixlen
    else:
        prefix = address_entry.ipv6prefix
    return f"{address_entry.address}/{prefix}"


def socket_ip(socket, vci) -> str:
    """Return ``<address>/<prefix>`` for a socket's endpoint address.

    Looks up the matching :class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` /
    :class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry` from the virtual interface so
    the subnet mask is available.  Falls back to the bare IP string when *vci* is ``None``
    (untagged socket without a resolvable VCI).
    """
    if vci is not None:
        for addr_entry in vci.addresses or []:
            if str(addr_entry.address) == str(socket.endpoint_address):
                return format_ip(addr_entry)
    return str(socket.endpoint_address)


def _collect_controller_vlan_members(ecu, grouped: dict) -> None:
    """Collect VLAN members from controller interfaces (direct and compute-node VCIs) for *ecu*."""
    for ctrl, eth_iface in iter_ecu_interfaces(ecu):
        for vci, _mac in iter_virtual_interfaces(eth_iface.interface_config):
            ips = [format_ip(addr) for addr in vci.addresses]
            grouped[vci.vlanid].append(VlanMember(ecu, f"{ctrl.name}/{eth_iface.name}", "Controller Interface", ips))


def _collect_switch_vlan_members(ecu, grouped: dict) -> None:
    """Collect VLAN members from switch ports for *ecu*."""
    for switch in ecu.get_all_switches():
        for vlan in switch.vlans or []:
            for port_name in vlan.ports:
                grouped[vlan.id].append(VlanMember(ecu, port_name, "Switch Port", []))


def vlan_members(model, ecu_name: Optional[str] = None) -> dict:
    """
    Return ``{vlan_id: [VlanMember, ...]}`` across the (optionally filtered) ECUs.

    Members are controller-interface VCIs (direct and compute-node) and switch ports - the two places a
    VLAN is declared in the model.
    """

    grouped: dict = defaultdict(list)
    for ecu in ecus_for(model, ecu_name):
        _collect_controller_vlan_members(ecu, grouped)
        _collect_switch_vlan_members(ecu, grouped)
    return grouped


def someip_deployments_by_service(model) -> dict:
    """Return ``{(service_id, major_version): [(SocketEndpoint, deployment), ...]}`` across every ECU."""
    grouped: dict = defaultdict(list)
    for ecu in model.ecus:
        for endpoint in socket_endpoints_for_ecu(ecu):
            for deployment_union in endpoint.socket.deployments or []:
                deployment = deployment_union.root
                if isinstance(deployment, (SOMEIPServiceProvider, SOMEIPServiceConsumer)):
                    grouped[(deployment.service, deployment.major_version)].append((endpoint, deployment))
    return grouped
