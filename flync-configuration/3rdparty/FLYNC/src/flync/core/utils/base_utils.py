"""Base Utils that can be useful throughout the whole FLYNC Library and toolchain."""

import os
from ipaddress import IPv4Address, IPv6Address, ip_address
from pathlib import Path
from typing import Tuple, Type, TypeVar

import yaml
from pydantic_extra_types.mac_address import MacAddress
from rich import print as rprint

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_fatal


def read_yaml(path: str | os.PathLike | Path):
    """
    Read a YAML file.

    Args:
        path (str | os.PathLike | Path): Path to the YAML file

    Raises:
        err_fatal: If path is not for YAML file.

    Returns:
        Any: Retrieved data from YAML.
    """

    if not isinstance(path, Path):
        path = Path(path)
    if not path.exists():
        raise FileNotFoundError(f"{path} not existent!")

    if path.suffix not in [".yaml", ".yml"]:
        raise err_fatal("Not a YAML file!", category=Category.FORMAT, error_number="001")
    try:
        with open(path, "r", encoding="utf-8") as yml:
            file = yaml.safe_load(yml)
            rprint(f"[green]File Loaded: {path}[/green]")
            return file
    except Exception as e:
        rprint(f"[red]{e}[/red]")


def get_yaml_paths(base_path: str | os.PathLike) -> list:
    """
    Collect absolute paths to yaml files from a base_path.

    Args:
        base_path (str | os.PathLike): Base Path to FLYNC Config.

    Returns:
        list: List of absolute file paths to yaml files.
    """

    base_path = Path(base_path)
    path_list = []
    if base_path.is_dir():
        for ending in ["*.yaml", "*.yml"]:
            path_list += [f.resolve().as_posix() for f in base_path.rglob(ending)]
    elif base_path.is_file() and base_path.suffix in [".yaml", ".yml"]:
        path_list.append(base_path.absolute().as_posix())

    return path_list


def is_mac_address(input: str) -> Tuple[bool, str]:
    """
    Helper to check if an input is a valid MAC address based on pydantic validator.

    Args:
        input (str): input string that should be checked.

    Returns:
        bool: Returns the result of check as a boolean as well as a message that could be used in logging or exception handling.
        If the result boolean is true, the provided input is a MAC address.
        If the boolean is false, it is not.
    """

    try:
        if not isinstance(input, str):
            return False, f"{input!r} is not a MAC address."
        MacAddress.validate_mac_address(input.encode())
        is_mac = True
        msg = f"{input} is a MAC address."
    except Exception:
        is_mac = False
        msg = f"{input} is not a MAC address."
    return is_mac, msg


def is_mac_unicast(input: str) -> Tuple[bool, str]:
    """
    Helper to check if a MAC address is unicast. Unicast if first byte's least significant bit is 0.

    Args:
        input (str): input string that should be checked.

    Returns:
        Union[bool, str]: Returns the result of check as a boolean as well as a message that could be used in logging or exception handling.
        If the result boolean is true, the provided input is a unicast MAC address. If the boolean is false, it is not.
    """

    is_unicast = False
    msg = msg = f"{input} is not a Unicast MAC address."

    mac_bytes = input.replace(":", "").replace("-", "")  # Remove separators
    first_byte = int(mac_bytes[:2], 16)  # Convert the first byte to an integer

    if first_byte & 1 == 0:
        msg = f"{input} is a Unicast MAC address."
        is_unicast = True

    return is_unicast, msg


def is_mac_multicast(input: str) -> Tuple[bool, str]:
    """
    Method to check if a MAC address is multicast. Multicast if first byte's least significant bit is 1.

    Args:
        input (str): input string that should be checked.

    Returns:
        Union[bool, str]: Returns the result of check as a boolean as well as a message that could be used in logging or exception
        handling. If the result boolean is true, the provided input is a MAC multicast address. If the boolean is false, it is not.

    """

    is_multicast = False
    msg = f"{input} is not a MAC Multicast. The first byte's least significant bit should be 1."

    mac_bytes = input.replace(":", "").replace("-", "")  # Remove separators
    first_byte = int(mac_bytes[:2], 16)  # Convert the first byte to an integer

    if first_byte & 1 == 1:
        is_multicast = True
        msg = f"{input} is a MAC Multicast."

    return is_multicast, msg


def is_ip_address(input: IPv4Address | IPv6Address | str) -> Tuple[bool, str]:
    """
    Helper to check if an input is a valid IP address.

    Args:
        input (:class:`IPv4Address` | :class:`IPv6Address` | str): input
        string that should be checked.

    Returns:
        bool: Returns the result of check as a boolean as well as a message that could be used in logging or exception handling.
        If the result boolean is true, the provided input is an IP address. If the boolean is false, it is not.
    """

    try:
        ip_address(input)
        is_ip = True
        msg = f"{input} is an IP Address."
    except ValueError:
        is_ip = False
        msg = f"{input} is not an IP Address."
    return is_ip, msg


def is_ip_multicast(
    input: IPv4Address | IPv6Address | str,
) -> Tuple[bool, str]:
    """
    Method to check if a string is an IP multicast address.

    Args:
        input (:class:`IPv4Address` | :class:`IPv6Address`): input string that should be checked.

    Returns:
        Union[bool, str]: Returns the result of check as a boolean as well as a message that could be used in logging or exception handling.
        If the result boolean is true, the provided input is an IP multicast address. If the boolean is false, it is not.

    """

    is_multicast = False
    address = ip_address(input)
    if address.is_multicast:
        is_multicast = True
        msg = f"{input} is an IP Multicast."
    else:
        is_multicast = False
        msg = f"{input} is not an IP Multicast. See RFC 3171 for details."
    return is_multicast, msg


def get_duplicates_in_list(input: list) -> list:
    """
    Find duplicates in a list.

    Args:
        input (list): a list where duplicates are suspected.

    Returns:
        list: returns a list of the duplicates that were found.
    """

    seen = set()
    dupes = set()

    for value in input:
        if value in seen:
            dupes.add(value)
        else:
            seen.add(value)

    return list(dupes)


def check_obj_in_list(obj, list):
    """
    Helper function: To check if the object is in the list or not
    """

    flag = False
    for c in list:
        if c is None:
            continue
        if (c.type == obj.type) and (
            (c.type == "switch_port" and obj.name == c.name and obj.get_switch().name == c.get_switch().name)
            or (c.type == "controller_interface" and obj.name == c.name and obj.get_controller().name == c.get_controller().name)
            or (c.type == "ecu_port" and obj.name == c.name)
        ):
            flag = True

    return flag


def deep_iter(obj, stop_class):
    """
    Helper function: To deeply iterate through an object and its children. The iteration will stop if an object of type stop_class is reached.

    Args:
        obj: The object to iterate through.
        stop_class: The class type at which the iteration should stop.
    Returns:
        Generator: A generator that yields objects of the specified type.
    """

    seen = set()
    stack = [obj]
    while stack:
        current = stack.pop()
        oid = id(current)
        if oid in seen:
            continue
        seen.add(oid)

        # single yield site
        yield current

        # do not traverse children of stop_class
        if isinstance(current, stop_class):
            continue

        if isinstance(current, dict):
            # push children (LIFO); order not guaranteed
            stack.extend(current.values())
        elif isinstance(current, (list, tuple)):
            stack.extend(current)
        else:
            try:
                stack.extend(vars(current).values())
            except TypeError:
                # cur has no __dict__
                pass


T = TypeVar("T", bound=FLYNCBaseModel)


def find_all(base, target_class: Type[T]) -> list[T]:
    """
    Helper to get all fields of certain type in nested data.

    Args:
        base: The base object to start searching from.
        target_class: The class to search for.

    Returns:
        list: A list of all instances of the target class found within the
        base object.
    """

    return [o for o in deep_iter(base, target_class) if isinstance(o, target_class)]
