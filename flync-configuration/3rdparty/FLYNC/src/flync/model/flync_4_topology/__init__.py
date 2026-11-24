"""
This package provides topology models for FLYNC
"""

from .bus_topology import BusAttachmentPoint, BusTopology, CANBusTopology, LINBusTopology
from .ethernet_multidrop import EthernetMultidropConnection, EthernetMultidropNode, PLCACycle
from .ethernet_topology import EthernetPointToPointConnection, EthernetTopology, FLYNCTopology

#: Former name of :class:`EthernetPointToPointConnection`.
ExternalConnection = EthernetPointToPointConnection

KEY = "TOP"
__all__ = [
    "BusAttachmentPoint",
    "BusTopology",
    "CANBusTopology",
    "EthernetPointToPointConnection",
    "EthernetMultidropConnection",
    "EthernetMultidropNode",
    "ExternalConnection",
    "EthernetTopology",
    "FLYNCTopology",
    "LINBusTopology",
    "PLCACycle",
]
