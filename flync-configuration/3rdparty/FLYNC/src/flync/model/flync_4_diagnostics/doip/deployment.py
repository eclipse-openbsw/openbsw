"""
Defines the socket-level DoIP deployments: a DoIP entity (server) and the DoIP discovery
endpoint (vehicle identification / vehicle announcement), as specified by ISO 13400.

These carry the DoIP transport layer of a diagnostic connection - logical address and DoIP
timers. The UDS content they expose lives in a
:class:`~flync.model.flync_4_diagnostics.uds.server.UDSServer`.
"""

from typing import Annotated, Literal, Optional

from pydantic import Field

from flync.core.annotations.reference import Reference
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_diagnostics.uds.server import UDSServer

from .timings import DoIPTimingProfile


class DoIPServerDeployment(FLYNCBaseModel):
    """
    Deploys a DoIP entity (server) that answers UDS diagnostic requests on this socket.

    Belongs on a ``SocketTCP`` only - DoIP diagnostic messaging (ISO 13400) runs over TCP.

    Parameters
    ----------
    deployment_type : Literal["doip_server"]

    name : str
        Name of the deployment, typically the ECU or diagnostic entity it represents.

    logical_address : int
        The 16-bit DoIP logical address of this diagnostic entity. Must be unique across
        the whole system.

    uds_server : str
        Name of the :class:`~flync.model.flync_4_diagnostics.uds.server.UDSServer`
        (access profiles, supported services, DIDs, DTCs) exposed by this entity.

    doip_timings_profile : str, optional
        Name of the :class:`~flync.model.flync_4_diagnostics.doip.timings.DoIPTimingProfile`
        used by this entity. ``None`` uses the system default profile.
    """

    deployment_type: Literal["doip_server"] = Field(default="doip_server")
    name: str = Field()
    logical_address: Annotated[int, Field(ge=0x0000, le=0xFFFF)] = Field(description="the DoIP logical address of this diagnostic entity")
    uds_server: Annotated[str, Reference(source="_uds_server_ref")] = Field(description="the UDS server exposed by this entity")
    doip_timings_profile: Annotated[Optional[str], Reference(source="_timings_ref")] = Field(default=None)

    _uds_server_ref: Optional[UDSServer] = None
    _timings_ref: Optional[DoIPTimingProfile] = None

    def bind(self, servers_by_name: dict, timings_by_id: dict) -> None:
        """
        Resolve :attr:`uds_server` and :attr:`doip_timings_profile` against the system-wide catalogs.
        """

        server = servers_by_name.get(self.uds_server)
        if server is None:
            raise err_major(
                "DoIP server deployment '{deployment_name}' references unknown UDS server '{server_name}'",
                category=Category.REFERENCE,
                error_number="278",
                deployment_name=self.name,
                server_name=self.uds_server,
            )
        self._uds_server_ref = server

        if self.doip_timings_profile is None:
            return
        timings = timings_by_id.get(self.doip_timings_profile)
        if timings is None:
            raise err_major(
                "DoIP server deployment '{deployment_name}' references unknown DoIP timings profile '{profile_id}'",
                category=Category.REFERENCE,
                error_number="280",
                deployment_name=self.name,
                profile_id=self.doip_timings_profile,
            )
        self._timings_ref = timings


class DoIPDiscoveryDeployment(FLYNCBaseModel):
    """
    Deploys the DoIP discovery endpoint: vehicle identification requests and, optionally,
    vehicle announcement messages, as specified by ISO 13400.

    Belongs on a ``SocketUDP`` only - DoIP discovery runs over UDP.

    Parameters
    ----------
    deployment_type : Literal["doip_discovery"]

    name : str, optional
        Name of the deployment, used to identify it in validation messages.

    vehicle_identification : bool, optional
        Whether this endpoint answers VehicleIdentificationRequest messages. Defaults to ``True``.

    vehicle_announcement : bool, optional
        Whether this endpoint sends VehicleAnnouncementMessages on power-up. Defaults to ``True``.

    doip_timings_profile : str, optional
        Name of the :class:`~flync.model.flync_4_diagnostics.doip.timings.DoIPTimingProfile`
        used for the announcement timers. ``None`` uses the system default profile.
    """

    deployment_type: Literal["doip_discovery"] = Field(default="doip_discovery")
    name: Optional[str] = Field(default=None)
    vehicle_identification: bool = Field(default=True)
    vehicle_announcement: bool = Field(default=True)
    doip_timings_profile: Annotated[Optional[str], Reference(source="_timings_ref")] = Field(default=None)

    _timings_ref: Optional[DoIPTimingProfile] = None

    def bind(self, timings_by_id: dict) -> None:
        """
        Resolve :attr:`doip_timings_profile` against the system-wide catalog, when given.
        """

        if self.doip_timings_profile is None:
            return
        timings = timings_by_id.get(self.doip_timings_profile)
        if timings is None:
            raise err_major(
                "DoIP discovery deployment references unknown DoIP timings profile '{profile_id}'",
                category=Category.REFERENCE,
                error_number="279",
                profile_id=self.doip_timings_profile,
            )
        self._timings_ref = timings
