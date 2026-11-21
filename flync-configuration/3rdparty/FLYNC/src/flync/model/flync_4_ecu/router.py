"""Defines the IP routing models and utilities for FLYNC controllers."""

from ipaddress import IPv4Address, IPv4Network, IPv6Address, IPv6Network

from pydantic import Field, model_validator
from pydantic.networks import IPvAnyAddress

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.datatypes.ipaddress import IPv4AddressEntry, IPv6AddressEntry
from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_ecu.sockets import IPv4AddressEndpoint, IPv6AddressEndpoint


class RouteEntry(FLYNCBaseModel):
    """
    Represents a static routing table entry.

    Parameters
    ----------
    destination : :class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` or :class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry`
        The destination network expressed as address and mask (e.g. ``address="10.0.0.0", ipv4netmask="255.255.255.0"``).

    default_gateway : :class:`~pydantic.networks.IPvAnyAddress`
        Gateway IP for this route.
        If the next hop is a router, this is that router's VCI address on the shared subnet.
        If the next hop is a controller interface (directly connected), this is the controller interface's IP address.

    egress_interface : str
        Name of the :class:`~flync.model.flync_4_ecu.controller.VirtualControllerInterface` on this interface
        through which this route is forwarded.
    """

    destination: IPv4AddressEntry | IPv6AddressEntry = Field()
    default_gateway: IPvAnyAddress = Field()
    egress_interface: str = Field()

    @model_validator(mode="after")
    def validate_gateway_family_matches_destination(self) -> "RouteEntry":
        """
        Raise ``err_major`` if ``default_gateway`` does not share the address family of ``destination``.

        A route that mixes IPv4 and IPv6 (e.g. an IPv4 destination with an IPv6 gateway) is not routable as a
        single entry and likely a configuration mistake.
        """
        mismatch = (isinstance(self.destination, IPv4AddressEntry) and isinstance(self.default_gateway, IPv6Address)) or (
            isinstance(self.destination, IPv6AddressEntry) and isinstance(self.default_gateway, IPv4Address)
        )
        if mismatch:
            raise err_major(
                "default_gateway '{gateway}' does not belong to the same address family as destination '{dest_address}'",
                category=Category.CONSISTENCY,
                error_number="247",
                gateway=self.default_gateway,
                dest_address=self.destination.address,
            )
        return self


def gateway_in_subnet(route: RouteEntry, vci) -> bool:
    """
    Return ``True`` if ``route.default_gateway`` falls within any address subnet configured on ``vci``.

    Parameters
    ----------
    route : :class:`RouteEntry`
        The routing table entry whose ``default_gateway`` is being validated.

    vci : :class:`~flync.model.flync_4_ecu.controller.VirtualControllerInterface`
        The virtual controller interface to check against. Its ``addresses`` list is used to derive the subnet(s).
    """
    for addr_entry in vci.addresses:
        if (
            isinstance(addr_entry, IPv4AddressEndpoint)
            and isinstance(route.default_gateway, IPv4Address)
            and route.default_gateway
            in IPv4Network(
                f"{addr_entry.address}/{addr_entry.ipv4netmask}",
                strict=False,
            )
        ) or (
            isinstance(addr_entry, IPv6AddressEndpoint)
            and isinstance(route.default_gateway, IPv6Address)
            and route.default_gateway
            in IPv6Network(
                f"{addr_entry.address}/{addr_entry.ipv6prefix}",
                strict=False,
            )
        ):
            return True

    return False
