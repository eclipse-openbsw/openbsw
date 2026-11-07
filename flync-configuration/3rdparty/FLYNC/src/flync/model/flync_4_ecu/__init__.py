"""
This package provides models for representing an ECU in the FLYNC architecture, including controllers, ports, switches, sockets,
and internal topology definitions.
"""

from . import controller as _controller_module
from .compute_node import ComputeNode
from .controller import (
    Controller,
    EthernetInterface,
    EthernetInterfaceConfig,
    VirtualControllerInterface,
)
from .controller_interface import ControllerInterface
from .controller_topology import ControllerConnectionUnion, ControllerTopology
from .ecu import ECU
from .internal_topology import InternalTopology
from .mac_multicast_endpoint import (
    AVTPMulticastEndpoint,
    MACMulticastEndpoint,
    MACMulticastEndpoints,
)
from .multicast_groups import MulticastGroupMembership
from .phy import BASET, BASET1, BASET1S, MII, RGMII, RMII, SGMII, XFI
from .port import ECUPort
from .router import RouteEntry
from .socket_container import SocketContainer
from .sockets import (
    IPv4AddressEndpoint,
    IPv6AddressEndpoint,
    Socket,
    SocketTCP,
    SocketUDP,
    TCPOption,
    UDPOption,
)
from .switch import (
    FrameMask,
    Switch,
    SwitchConfig,
    SwitchPort,
    TCAMRule,
    TrafficClass,
)
from .vlan_entry import MulticastGroup, VLANEntry

# ``Controller`` declares ``compute_nodes``, ``switches`` and ``controller_topology`` as
# forward references: it cannot import those modules itself, because switch.py imports Controller and
# compute_node.py imports both. Every module is loaded by this point, so inject the concrete types
# into controller.py's namespace and rebuild the model so pydantic can resolve them.
setattr(_controller_module, "ComputeNode", ComputeNode)
setattr(_controller_module, "Switch", Switch)
setattr(_controller_module, "ControllerTopology", ControllerTopology)
Controller.model_rebuild(force=True)

KEY = "ECU"
__all__ = [
    "AVTPMulticastEndpoint",
    "BASET",
    "BASET1",
    "BASET1S",
    "ComputeNode",
    "Controller",
    "ControllerConnectionUnion",
    "ControllerInterface",
    "ControllerTopology",
    "ECU",
    "ECUPort",
    "EthernetInterface",
    "EthernetInterfaceConfig",
    "FrameMask",
    "InternalTopology",
    "IPv4AddressEndpoint",
    "IPv6AddressEndpoint",
    "MACMulticastEndpoint",
    "MACMulticastEndpoints",
    "MII",
    "MulticastGroup",
    "MulticastGroupMembership",
    "RGMII",
    "RMII",
    "RouteEntry",
    "SGMII",
    "Socket",
    "SocketContainer",
    "SocketTCP",
    "SocketUDP",
    "Switch",
    "SwitchConfig",
    "SwitchPort",
    "TCAMRule",
    "TCPOption",
    "TrafficClass",
    "UDPOption",
    "VirtualControllerInterface",
    "VLANEntry",
    "XFI",
]
