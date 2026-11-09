"""Defines MAC multicast endpoints (including AVTP) used by FLYNC ECUs."""

from typing import Annotated, List, Literal, Optional

from pydantic import AfterValidator, Field, RootModel, field_validator
from pydantic_extra_types.mac_address import MacAddress

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes.macaddress import FLYNCMacAddress, is_mac_in_range
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.address import validate_mac_multicast, validate_vlan_id


class MACMulticastEndpoint(FLYNCBaseModel):
    """
    Represents a multicast endpoint that is bound to a specific controller.

    Parameters
    ----------

    name : str
        Name of the multicast endpoint.
    mac_address : MacAddress
        MAC address of the controller that this endpoint is bound to.
    protocol : str
        Protocol that is expected on this endpoint.
    ethertype : int, optional
        EtherType that is expected on this endpoint (defaults to ``None``). Must be between \
            0x0000 and 0xFFFF if provided.
    vlan_id : int, optional
        VLAN ID that is expected on this endpoint (defaults to ``None``). Must be between \
            0 and 4095 if provided.
    multicast_tx : list of MacAddress, optional
        List of multicast addresses that this endpoint should transmit to (defaults to ``[]``).\
            Each address must be a valid multicast MAC address.
    """

    name: str = Field(description="Name of the multicast endpoint.")
    mac_address: FLYNCMacAddress = Field(description="MAC address of the controller that this endpoint is \
            bound to.")
    protocol: str = Field(description="Protocol that is expected on this endpoint.")
    ethertype: Optional[int] = Field(
        description="EtherType that is expected on this endpoint.",
        ge=0x0000,
        le=0xFFFF,
        default=None,
    )
    vlan_id: Annotated[Optional[int], AfterValidator(validate_vlan_id)] = Field(
        description="VLAN ID expected on this endpoint (``None`` for untagged).",
        default=None,
    )
    multicast_tx: Optional[List[Annotated[FLYNCMacAddress, AfterValidator(validate_mac_multicast)]]] = Field(
        description="List of multicast addresses that this endpoint should \
            transmit to.",
        default=[],
    )


class AVTPMulticastEndpoint(MACMulticastEndpoint):
    """
    Represents an AVTP multicast endpoint that is bound to a specific controller.
    This is a specialized version of MACMulticastEndpoint with fixed EtherType and protocol values, and additional validation to \
        ensure that multicast addresses are within the AVTP range.

    Parameters
    ----------
    ethertype : Literal[0x22F0]
        EtherType for AVTP, fixed to 0x22F0.
    protocol : Literal["avtp"] | Literal["AVTP"]
        Protocol for AVTP, fixed to "avtp" (case-insensitive).
    """

    ethertype: Literal[0x22F0] = Field(default=0x22F0)
    protocol: Literal["avtp"] | Literal["AVTP"] = Field()

    @field_validator("multicast_tx", mode="after")
    def validate_avtp_multicast_range(cls, v):
        """
        Validate that all multicast addresses in the list are within \
            the AVTP multicast range."""

        for mac in v:
            if not is_mac_in_range(
                mac,
                MacAddress("91:E0:F0:00:00:00"),
                MacAddress("91:E0:F0:00:00:FF"),
            ):
                raise err_major(
                    f"AVTP multicast address {str(mac).upper()} is out of the valid range 91:E0:F0:00:00:00 - 91:E0:F0:00:00:FF.",
                    category=Category.VALUE_RANGE,
                    error_number="079",
                )
        return v


class MACEndpointUnion(RootModel):
    """
    Union type for MAC multicast endpoints, discriminated by the 'ethertype'
    field.

    Possible types
    --------------
    - :class:`~AVTPMulticastEndpoint`: If ethertype is 0x22F0, the endpoint is treated as an AVTP multicast endpoint.
    """

    root: AVTPMulticastEndpoint = Field(discriminator="protocol")


class MACMulticastEndpoints(FLYNCBaseModel):
    """
    Represents a collection of multicast endpoints for an ECU.

    Parameters
    ----------
    endpoints : List[:class:`~MACEndpointUnion`]
        List of multicast endpoints associated with the ECU.
    """

    endpoints: List[MACEndpointUnion] = Field(default_factory=list)
