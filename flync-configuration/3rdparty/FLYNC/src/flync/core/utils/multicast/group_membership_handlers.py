"""Collects IPv6 solicited-node multicast group memberships for FLYNC endpoints."""

from itertools import product

from flync.core.utils.base_utils import find_all
from flync.model.flync_4_ecu import (
    ECU,
    MulticastGroupMembership,
    VirtualControllerInterface,
)


def _mgm_data_key(g: "MulticastGroupMembership"):
    """
    Return a tuple of data fields used for deduplication.

    Pydantic's default __eq__ compares __pydantic_private__ as well, which causes infinite recursion when ControllerInterface._connected_component
    forms a cycle (e.g. via ECUPortToControllerInterface or SwitchPortToControllerInterface).
    Comparing only the data fields is sufficient for uniqueness checks.
    """

    return (str(g.group), g.vlan, g.mode, str(g.src_ip) if g.src_ip else None)


def collect_ipv6_solicited_node_rx(
    ecu: ECU,
) -> dict[str, MulticastGroupMembership]:
    """
    Collects all the MulticastGroupMembership instances for the solicited-node multicast group in the given ECU.
    """

    rx_group_keys = set()
    rx_groups = []
    update_ecu_multicast = {}

    for viface in find_all(ecu.controllers, VirtualControllerInterface):
        for ip in viface.addresses:
            if ip.address.version != 6:
                continue
            multicast_addr = ip.derive_multicast_address()
            group = MulticastGroupMembership(
                group=multicast_addr,
                description="",
                mode="rx",
                vlan=viface.vlanid,
                src_ip=ip.address,
                solicited_node_multicast=True,
            )
            group._interface = ecu.get_interface_for_ip(str(ip.address))
            key = _mgm_data_key(group)
            if key not in rx_group_keys:
                rx_group_keys.add(key)
                rx_groups.append(group)
            update_ecu_multicast.update({ecu.name: group})
    return update_ecu_multicast


def collect_ipv6_solicited_node_tx(
    ecu: ECU,
    rx_multicasts: list[MulticastGroupMembership],
) -> dict[str, MulticastGroupMembership]:
    """
    Collects all the MulticastGroupMembership instances for the solicited-node multicast group in the given ECU.
    """

    tx_group_keys = set()
    tx_groups = []
    update_ecu_multicast = {}
    vi_controller_interfaces = find_all(ecu.controllers, VirtualControllerInterface)
    for multicast, vi in product(rx_multicasts, vi_controller_interfaces):
        if vi.vlanid != multicast.vlan:
            continue
        for ip in vi.addresses:
            if ip.address.version != 6:
                continue
            addr = ip.address
            if multicast.src_ip == addr:
                continue
            group = MulticastGroupMembership(
                group=multicast.group,
                description="",
                mode="tx",
                vlan=multicast.vlan,
                src_ip=addr,
                solicited_node_multicast=True,
            )
            group._interface = ecu.get_interface_for_ip(str(addr))
            key = _mgm_data_key(group)
            if key not in tx_group_keys:
                tx_group_keys.add(key)
                tx_groups.append(group)
            update_ecu_multicast.update({ecu.name: group})
    return update_ecu_multicast
