"""Defines the ECU Port model for FLYNC"""

from __future__ import annotations

from typing import TYPE_CHECKING, List, Literal, Optional

from pydantic import Field, PrivateAttr, model_validator

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.ecu import ECU

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_ecu.phy import (
    BASET,
    BASET1,
    BASET1S,
    MII,
    RGMII,
    RMII,
    SGMII,
    XFI,
)


class ECUPort(FLYNCBaseModel):
    """
    Represents an ECU port and its configuration.

    This class encapsulates both the media-dependent (MDI) and media-independent (MII) interface configurations for a port.

    Parameters
    ----------
    name : str
        Name of the ECU port.

    mdi_config : :class:`~flync.model.flync_4_ecu.phy.BASET1` or :class:`~flync.model.flync_4_ecu.phy.BASET1S` or \
    :class:`~flync.model.flync_4_ecu.phy.BASET`
        Media-dependent interface configuration, such as BASE-T1, BASE-T1S, or BASE-T.

    mii_config : :class:`~flync.model.flync_4_ecu.phy.MII` or :class:`~flync.model.flync_4_ecu.phy.RMII` or \
    :class:`~flync.model.flync_4_ecu.phy.SGMII` or :class:`~flync.model.flync_4_ecu.phy.RGMII`, optional
        Media-independent interface configuration, such as MII or RMII.

    Private Attributes
    ------------------
    _ecu :
        The ECU of which the ECU Port is a part of.
    _connected_component:
        The switch port, controller interface or ecu port connected to the controller interface. This attribute
        is managed internally and is not part of the public API.
    _type:
        The type of the object generated. Set to ecu_port.
    """

    name: str = Field()
    mdi_config: BASET1 | BASET1S | BASET = Field(
        default_factory=BASET1,
        discriminator="mode",
        description="how to use this",
    )
    mii_config: Optional[MII | RMII | SGMII | RGMII | XFI] = Field(default=None, discriminator="type")
    _ecu: "ECU" | None = PrivateAttr(default=None)
    _connected_components: List = []
    _type: Literal["ecu_port"] = PrivateAttr(default="ecu_port")

    @property
    def ecu(self) -> "ECU" | None:
        return self._ecu

    @property
    def type(self):
        return self._type

    @property
    def connected_components(self):
        return self._connected_components

    @model_validator(mode="after")
    def verify_mdi_and_mii_config_have_same_speed(self):
        """
        Ensure that, when both MII and MDI configurations are present, their ``speed`` fields match.
        """

        if self.mii_config is not None and self.mii_config.speed != self.mdi_config.speed:
            raise err_major(
                f"MII and MDI config should have the same speed in ECU Ports. Port {self.name}", category=Category.CONSISTENCY, error_number="081"
            )
        return self

    def get_internal_connected_component(self, ecus):
        """
        Return the component inside the ECU connected  to the ECU Port.
        """

        return next(
            (c for c in self._connected_components if c.type != "ecu_port"),
            None,
        )
