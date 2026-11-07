"""
Validators for VLAN identifiers and unicast/multicast MAC and IP addresses.
"""

import re
from ipaddress import IPv4Address, IPv6Address
from typing import Any

import flync.core.utils.base_utils as utils
from flync.core.utils.exceptions import Category, err_minor, warn

VLAN_ID_RESERVED = 4095
VLAN_ID_MIN = 0
VLAN_ID_MAX = VLAN_ID_RESERVED

_MAC_NO_SEPARATOR_PATTERN = re.compile(r"^[0-9A-Fa-f]{12}$")


def before_validate_mac_address(value: Any) -> Any:
    """Pre-validation for MAC address fields with user-friendly error messages.

    Catches common mistakes before pydantic_extra_types processes the value:
    - Integer input (e.g. YAML parses 001122334455 as an int)
    - None
    - String without separators (e.g. "aabbccddeeff")
    """
    if not isinstance(value, str):
        raise ValueError("MAC address must be a string in the format 'xx:xx:xx:xx:xx:xx' or 'xx-xx-xx-xx-xx-xx'. ")
    if isinstance(value, str) and _MAC_NO_SEPARATOR_PATTERN.match(value):
        formatted = ":".join(value[i : i + 2] for i in range(0, 12, 2))  # noqa: E203 - black & flake8 formats colliding
        raise ValueError(f"MAC address '{value}' is missing separators. Use the format 'xx:xx:xx:xx:xx:xx' (e.g., '{formatted}').")
    return value


def validate_vlan_id(value):
    """
    Validate a VLAN identifier.

    ``None`` is treated as untagged and returned unchanged.
    Values in the range 0-4094 are accepted as-is.
    The reserved value 4095 is accepted but emits a warning via :func:`warn`.
    Anything outside 0-4095 raises a minor validation error.
    """

    if value is not None:
        if value < VLAN_ID_MIN or value > VLAN_ID_MAX:
            raise err_minor(
                f"VLAN ID must be in the range {VLAN_ID_MIN}-{VLAN_ID_MAX - 1} (use None for untagged); got {value}.",
                category=Category.VALUE_RANGE,
                error_number="002",
            )
        if value == VLAN_ID_RESERVED:
            warn(f"VLAN ID {VLAN_ID_RESERVED} is reserved by IEEE 802.1Q and should not be used.", category=Category.VALUE_RANGE, error_number="003")
    return value


def validate_mac_unicast(input: str) -> str:
    """
    Custom Validator for Unicast MAC addresses.

    Args:
        input (str): MAC address to validate.

    Raises:
        err_minor: Input is not a Unicast address based on the expected format.

    Returns:
        Any: Input is handed over.
    """

    is_unicast, msg = utils.is_mac_unicast(input)
    if not is_unicast:
        raise err_minor(msg, category=Category.FORMAT, error_number="004")
    return input


def validate_mac_multicast(input: str) -> Any:
    """
    Custom Validator for Multicast MAC addresses.

    Args:
        input (str): MAC address to validate.

    Raises:
        err_minor: Input is not a Multicast address based on the expected format.

    Returns:
        Any: Input is handed over.
    """

    is_multicast, msg = utils.is_mac_multicast(input)
    if not is_multicast:
        raise err_minor(msg, category=Category.FORMAT, error_number="005")
    return input


def validate_ip_multicast(input: IPv4Address | IPv6Address | str) -> Any:
    """
    Custom Validator for Multicast IP addresses.

    Args:
        input (:class:`IPv4Address` | :class:`IPv6Address`): IP address to validate.

    Raises:
        err_minor: Input is not a Multicast address based on the expected format.

    Returns:
        Any: Input is handed over.
    """

    is_multicast, msg = utils.is_ip_multicast(input)
    if not is_multicast:
        raise err_minor(msg, category=Category.FORMAT, error_number="006")
    return input


def validate_any_multicast_address(
    input: IPv4Address | IPv6Address | str,
) -> Any:
    """
    Custom Validator for Multicast MAC or IP addresses.

    Args:
        input (:class:`IPv4Address` | :class:`IPv6Address` | str): IP address or MAC Address to validate.

    Raises:
        err_minor: The address is not a multicast address.

    Returns:
        Any: Input is handed over.
    """

    is_ip, _ = utils.is_ip_address(input)
    if is_ip:
        validate_ip_multicast(input)
    if isinstance(input, str):
        is_mac, _ = utils.is_mac_address(input)
        if is_mac and isinstance(input, str):
            validate_mac_multicast(input)
    return input


def validate_multicast_list_only_ip(input_list: list):
    """
    Custom Validator for a list of Multicast IP addresses.

    Args:
        input_list (list): List of only Multicast IPs.

    Raises:
        err_minor: Any of the addresses in the list is not an IP multicast address.
    """

    for value in input_list:
        validate_ip_multicast(value)
    return input_list


def validate_multicast_list(input_list: list):
    """
    Custom Validator for a list of Multicast MAC or IP addresses.

    Args:
        input_list (list): List of Multicast IPs and MACs.

    Raises:
        err_minor: Any of the addresses in the list is not a multicast address.
    """

    for value in input_list:
        validate_any_multicast_address(value)
    return input_list
