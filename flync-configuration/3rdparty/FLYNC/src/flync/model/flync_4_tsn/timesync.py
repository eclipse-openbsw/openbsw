"""
Defines PTP configuration models in FLYNC.
"""

from typing import Annotated, List, Literal, Optional

from pydantic import BeforeValidator, Field

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.validators.generic import none_to_empty_list


class PTPTimeTransmitterConfig(FLYNCBaseModel):
    """
    Configuration for a PTP (Precision Time Protocol) time transmitter.

    This class defines the settings required for a device acting as a time transmitter in a PTP domain.

    Parameters
    ----------
    log_tx_period : int
        Logarithmic mean message interval for Sync messages, expressed as 2^x seconds.
        Valid range is -7 (128 messages/sec) to 1 (1 message every 2 seconds).

    two_step : bool, optional
        Flag indicating whether two-step synchronization is used.
        Default is True.

    tlv : list of str, optional
        Optional list of TLV (Type-Length-Value) extensions included in Sync messages.
    """

    type: Literal["time_transmitter"] = Field(default="time_transmitter")
    log_tx_period: int = Field(..., ge=-7, le=1)
    two_step: Optional[bool] = Field(default=True)
    tlv: Annotated[
        Optional[List[str]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])


class PTPTimeReceiverConfig(FLYNCBaseModel):
    """
    Configuration for a PTP (Precision Time Protocol) time receiver.

    This class defines the settings required for a device acting as a time receiver in a PTP domain.

    Parameters
    ----------
    sync_timeout : int
        Timeout for receiving Sync messages, in milliseconds or implementation-specific units.

    sync_followup_timeout : int
        Timeout in milliseconds for receiving Follow_Up messages after a Sync message.
    """

    type: Literal["time_receiver"] = Field(default="time_receiver")
    sync_timeout: int = Field()
    sync_followup_timeout: int = Field()


class PTPPdelayConfig(FLYNCBaseModel):
    """
    Configuration for PTP peer delay (PDelay) messages.

    This class defines the interval for transmitting peer delay measurement messages used to estimate path delay between devices.

    Parameters
    ----------
    log_tx_period : int
        Logarithmic mean message interval for PDelay messages (2^x seconds).
        Valid range: -4 to 3.
    """

    log_tx_period: int = Field(..., ge=-4, le=3)


class PTPPort(FLYNCBaseModel):
    """
    PTP port configuration for an ECU (Electronic Control Unit).

    This class defines the configuration of a PTP port that may act as a transmitter or receiver, and optionally support peer delay measurements.

    Parameters
    ----------
    domain_id : int
        PTP domain identifier used to separate multiple time sync domains.
        Must be greater than or equal to 0.

    src_port_identity : int
        Unique identity for the source port. Must be >= 0.

    sync_config : :class:`PTPTimeTransmitterConfig` or :class:`PTPTimeReceiverConfig`
        Sync behavior configuration for this port.

    pdelay_config : :class:`PTPPdelayConfig`, optional
        Configuration for peer delay (PDelay) measurements.
    """

    domain_id: int = Field(..., ge=0)
    src_port_identity: int = Field(..., ge=0)
    sync_config: PTPTimeTransmitterConfig | PTPTimeReceiverConfig = Field(discriminator="type")
    pdelay_config: Optional[PTPPdelayConfig] = Field(default=None)


class PTPConfig(FLYNCBaseModel):
    """
    Top-level PTP configuration for an ECU.

    Parameters
    ----------
    cmlds_linkport_enabled : bool, optional
        Enable the Common Mean Link Delay Service (CMLDS) on the physical Link Port that these PTP ports share (defaults to ``False``).
        True → share meanLinkDelay/neighborRateRatio across all domains using delayMechanism=COMMON_P2P.
        False → instance-specific peer delay.

    ptp_ports : list of :class:`PTPPort`, optional
        List of PTP port configurations contained in this ECU (defaults to ``[]``).
    """

    cmlds_linkport_enabled: bool = Field(default=False)
    ptp_ports: List[PTPPort] = Field([])
