"""This module contains classes to model datatypes"""

from .base import Datatype
from .bitmask import Bitmask
from .bitrange import BitRange
from .duration import DurationMs, parse_duration_ms, serialize_duration_ms
from .ethertypes import Ethertype, serialize_ethertype, validate_ethertype_input
from .ipaddress import (
    IPv4AddressEntry,
    IPv4Multicast,
    IPv6AddressEntry,
    IPv6Multicast,
)
from .macaddress import FLYNCMacAddress, MACAddressEntry, MACAddressMulticast, MACAddressUnicast
from .value_range import ValueRange
from .value_table import ValueTable

__all__ = [
    "serialize_ethertype",
    "Bitmask",
    "BitRange",
    "Datatype",
    "DurationMs",
    "Ethertype",
    "IPv4AddressEntry",
    "IPv6AddressEntry",
    "IPv4Multicast",
    "IPv6Multicast",
    "FLYNCMacAddress",
    "MACAddressEntry",
    "MACAddressUnicast",
    "MACAddressMulticast",
    "parse_duration_ms",
    "serialize_duration_ms",
    "validate_ethertype_input",
    "ValueRange",
    "ValueTable",
]
