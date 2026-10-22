"""
VLAN configuration models for switches.

Defines :class:`VLANEntry` (a single VLAN configuration on a switch) and :class:`MulticastGroup` (a multicast destination tied to a set of switch
ports inside that VLAN).
"""

from typing import Annotated, List

from pydantic import (
    AfterValidator,
    Field,
    field_serializer,
    field_validator,
)
from pydantic.networks import IPvAnyAddress

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.datatypes.macaddress import FLYNCMacAddress
from flync.core.validators.address import validate_any_multicast_address, validate_vlan_id
from flync.core.validators.generic import none_to_empty_list


class MulticastGroup(FLYNCBaseModel):
    """
    Represents a multicast group configuration.

    This class defines a multicast group by associating a multicast destination address with a set of switch ports that participate in the group.

    Parameters
    ----------
    address : :class:`IPv4Address` or :class:`IPv6Address` or :class:`MacAddress`
        The multicast address. Must be a valid MAC or IP multicast address.

    ports : list of str
        A list of switch port names that are part of the multicast group.
    """

    address: IPvAnyAddress | FLYNCMacAddress = Field()
    ports: List[str] = Field()

    @field_validator("address", mode="after")
    @classmethod
    def validate_multicast_address(cls, v):
        """
        Validate that ``address`` is an IP or MAC multicast address.
        """

        return validate_any_multicast_address(v)

    @field_serializer("address")
    def serialize_address(self, address):
        """
        Serialize the multicast address as a string.
        """

        return str(address)


class VLANEntry(FLYNCBaseModel):
    """
    Represents a VLAN entry for a switch.

    Parameters
    ----------
    name : str
        Human-readable name for the VLAN.

    id : int
        VLAN ID. Values 0-4094 are accepted; 4095 is reserved by IEEE 802.1Q and emits a warning when used.

    default_priority : int
        Default frame priority for the VLAN (0-7).

    ports : list of str
        List of switch port names members of this VLAN.

    multicast : list of :class:`MulticastGroup`, optional
        List of multicast group configurations associated with this VLAN.
    """

    name: str = Field()
    id: Annotated[int, AfterValidator(validate_vlan_id)] = Field(...)
    default_priority: int = Field(..., ge=0, le=7)
    ports: List[str] = Field()
    multicast: List[MulticastGroup] | None = Field(default=[])

    @field_validator("multicast", mode="before")
    @classmethod
    def normalize_multicast(cls, v):
        """
        Coerce a ``None`` multicast list to an empty list.
        """

        return none_to_empty_list(v)
