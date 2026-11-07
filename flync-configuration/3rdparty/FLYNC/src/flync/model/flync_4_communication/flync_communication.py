"""Defines the communication system-wide configuration in FLYNC"""

from typing import Annotated, List, Optional

from pydantic import BeforeValidator, Field

from flync.core.annotations.external import (
    External,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import none_to_empty_list, validate_or_remove
from flync.model.flync_4_diagnostics import DiagnosticsConfig
from flync.model.flync_4_ecu import TCPOption
from flync.model.flync_4_nm import StateManagementConfig
from flync.model.flync_4_someip import SOMEIPConfig

from .flync_channels import FLYNCChannelConfig


class FLYNCCommunicationConfig(FLYNCBaseModel):
    """
    The top-level configuration object that aggregates all reusable
    FLYNC settings for the whole system.

    Parameters
    ----------
    tcp_profiles : list of \
    :class:`~flync.model.flync_4_ecu.sockets.TCPOption`, optional
        List of TCP profiles that define the selectable TCP socket options.

    someip_config : :class:`~flync.model.flync_4_someip.SOMEIPConfig`, optional
        Configuration block that holds the global SOME/IP service interface definition, SOME/IP timings, and SD timings profiles
        used by every ECU in the system.

    diagnostics_config : :class:`~flync.model.flync_4_diagnostics.DiagnosticsConfig`, optional
        Configuration block that holds the diagnostics configuration of the whole system, one folder per protocol:
        ``diagnostics/doip/`` for the DoIP transport timings and ``diagnostics/uds/`` for the UDS servers and the
        DID/routine/DTC catalogs. Loaded from ``communication/diagnostics/``.

    channels : :class:`~flync.model.flync_4_communication\
.flync_channels.FLYNCChannelConfig`, optional
        Channel-level configuration grouping CAN buses, LIN buses, and Ethernet Container PDU definitions.
        Loaded from the ``communication/channels/`` directory.
        Each bus or container PDU is stored in its own file under the corresponding sub-folder (``can/``, ``lin/``, ``container_pdus/``).
        Absent when the ``communication/channels/`` directory does not exist.

    state_management : :class:`~flync.model.flync_4_nm.StateManagementConfig`, optional
        System-wide state management groups.
        Loaded from the ``communication/state_management/`` directory; the group registry lands in ``groups.flync.yaml``.
        Defaults to an empty registry when the ``communication/state_management/`` directory does not exist.
    """

    tcp_profiles: Annotated[
        Optional[List[TCPOption]],
        External(output_structure=OutputStrategy.SINGLE_FILE),
        BeforeValidator(validate_or_remove("TCP profiles", List[TCPOption])),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    someip_config: Annotated[
        Optional[SOMEIPConfig],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="someip",
        ),
        BeforeValidator(validate_or_remove("SOME/IP config", SOMEIPConfig)),
    ] = Field(
        default=None,
        description="contains the SOME/IP config for the entire system.",
    )
    diagnostics_config: Annotated[
        Optional[DiagnosticsConfig],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="diagnostics",
        ),
        BeforeValidator(validate_or_remove("Diagnostics config", DiagnosticsConfig)),
    ] = Field(
        default=None,
        description="contains the DoIP and UDS diagnostics config for the entire system.",
    )
    channels: Annotated[
        Optional[FLYNCChannelConfig],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="channels",
        ),
    ] = Field(
        default=None,
        description=("CAN buses, LIN buses and Ethernet Container PDUs, loaded from communication/channels/."),
    )
    state_management: Annotated[
        Optional[StateManagementConfig],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="state_management",
        ),
    ] = Field(
        default_factory=StateManagementConfig,
        description="System-wide state management groups, loaded from communication/state_management/.",
    )
