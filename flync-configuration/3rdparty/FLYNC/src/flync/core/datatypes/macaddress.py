"""Defines MAC address model."""

from typing import Annotated, Optional

from pydantic import AfterValidator, BeforeValidator, Field
from pydantic_extra_types.mac_address import MacAddress

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.validators.address import before_validate_mac_address, validate_mac_multicast, validate_mac_unicast

FLYNCMacAddress = Annotated[MacAddress, BeforeValidator(before_validate_mac_address)]


def mac_to_int(mac: MacAddress) -> int:
    """Convert a MAC address to an integer."""
    hex_str = mac.replace(":", "")
    return int(hex_str, 16)


def is_mac_in_range(mac: MacAddress | str, start: MacAddress, end: MacAddress) -> bool:
    """Check if a MAC address is within a specified range."""
    if isinstance(mac, str):
        mac = MacAddress(mac)
    mac_int = mac_to_int(mac)
    start_int = mac_to_int(start)
    end_int = mac_to_int(end)
    return start_int <= mac_int <= end_int


class MACAddressEntry(FLYNCBaseModel):
    """
    Represents an MAC address entry for a network interface.

    Parameters
    ----------
    address : MacAddress
        Source MAC address to filter by. Format: "xx:xx:xx:xx:xx:xx"
    macmask : str
        The mask in MAC format. Format: "xx:xx:xx:xx:xx:xx". Defaults to ``"xx:xx:xx:xx:xx:xx"``.
    """

    address: FLYNCMacAddress = Field()
    macmask: Optional[str] = Field(default="xx:xx:xx:xx:xx:xx")


class MACAddressUnicast(MACAddressEntry):
    """
    Represents a Unicast MAC address entry for a network interface.

    Parameters
    ----------
    address : MacAddress
        Multicast MAC address. Format: ``"xx:xx:xx:xx:xx:xx"``.
    """

    address: Annotated[
        MacAddress,
        BeforeValidator(before_validate_mac_address),
        AfterValidator(validate_mac_unicast),
    ] = Field()


class MACAddressMulticast(MACAddressEntry):
    """
    Represents a Multicast MAC address entry for a network interface.

    Parameters
    ----------
    address : MacAddress
        Multicast MAC address. Format: ``"xx:xx:xx:xx:xx:xx"``.
    """

    address: Annotated[
        MacAddress,
        BeforeValidator(before_validate_mac_address),
        AfterValidator(validate_mac_multicast),
    ] = Field()
