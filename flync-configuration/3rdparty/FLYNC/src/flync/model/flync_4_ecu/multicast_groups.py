"""
Multicast group membership model for virtual controller interfaces.

Defines :class:`MulticastGroupMembership`, describing participation of a virtual controller interface in a single multicast group (IPv4, IPv6 or
MAC) along with the direction (tx/rx), VLAN and optional source IP.
"""

from ipaddress import IPv4Address, IPv6Address
from typing import Annotated, Literal, Optional

from pydantic import AfterValidator, Field, model_validator
from pydantic.networks import IPvAnyAddress

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes.macaddress import FLYNCMacAddress
from flync.core.utils.exceptions import Category, err_minor
from flync.core.validators.address import validate_any_multicast_address, validate_vlan_id
from flync.model.flync_4_ecu.controller import EthernetInterfaceConfig


class MulticastGroupMembership(FLYNCBaseModel):
    """
    Represents a multicast group membership for virtual controller interfaces.

    Parameters
    ----------
    group : :class:`~pydantic.networks.IPvAnyAddress` or :class:`~MacAddress`
        Multicast group address.
    description : str, optional
        Description of the multicast group membership.
    mode : Literal["tx"] or Literal["rx"], optional
        Mode of multicast group membership.
    vlan : int, optional
        VLAN ID associated with the multicast group membership.
        Use ``None`` for untagged.
    src_ip : :class:`~pydantic.networks.IPvAnyAddress`, optional
        Source IP address. Only applicable for "tx" mode.
    solicited_node_multicast : bool, optional
        Whether this is a solicited node multicast address (defaults to ``False``).
    """

    group: Annotated[
        IPvAnyAddress | FLYNCMacAddress,
        AfterValidator(validate_any_multicast_address),
    ] = Field()
    description: Optional[str] = Field(default="")
    mode: Literal["tx"] | Literal["rx"] = Field(default="rx")
    vlan: Annotated[Optional[int], AfterValidator(validate_vlan_id)] = Field(default=0)
    src_ip: Optional[IPvAnyAddress] = Field(default=None)
    solicited_node_multicast: Optional[bool] = Field(default=False)
    _interface: EthernetInterfaceConfig | None = None

    @model_validator(mode="after")
    def validate_src_ip_set_on_tx_ip_groups(self):
        if (isinstance(self.group, (IPv4Address | IPv6Address))) and self.mode == "tx" and not self.src_ip:
            raise err_minor(
                f"Multicast group membership for {self.group} ({self.mode} / VLAN {self.vlan} ) could not be defined."
                "The field 'src_ip' must be defined for IP multicast senders!",
                category=Category.REQUIRED,
                error_number="080",
            )
        return self

    @property
    def interface(self):
        """Return the controller interface this membership belongs to."""
        if self._interface is None:
            raise AttributeError(
                "This MulticastGroupMembership is not assigned to a controller interface yet. "
                "Assign it to a controller interface before accessing the 'interface' property."
            )
        return self._interface
