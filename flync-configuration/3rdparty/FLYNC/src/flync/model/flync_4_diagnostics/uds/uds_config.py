"""Defines the UDS configuration, loaded from ``communication/diagnostics/uds/``."""

from typing import Annotated, List, Literal

from pydantic import Field, model_validator

from flync.core.annotations import External, OutputStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import validate_list_items_unique

from .data_identifier import DataIdentifier
from .dtc import DiagnosticTroubleCode
from .routine import Routine
from .server import UDSServer
from .timings import UDSTimingProfileSet


class UDSConfig(FLYNCBaseModel):
    """
    The system-wide UDS (ISO 14229) configuration: the UDS servers and the catalogs they
    draw their DIDs, routines, and DTCs from.

    Parameters
    ----------
    version : Literal["0.14"], optional
        The version of this config. Defaults to ``"0.14"``.

    timings : :class:`~flync.model.flync_4_diagnostics.uds.timings.UDSTimingProfileSet`
        UDS timing profiles, loaded from ``uds/timings.flync.yaml``. Required: every UDS
        server references one of these profiles.

    servers : list of :class:`~flync.model.flync_4_diagnostics.uds.server.UDSServer`, optional
        UDS servers, one per file under ``uds/servers/``.

    dids : list of \
    :class:`~flync.model.flync_4_diagnostics.uds.data_identifier.DataIdentifier`, optional
        DID catalog, one per file under ``uds/dids/``.

    routines : list of :class:`~flync.model.flync_4_diagnostics.uds.routine.Routine`, optional
        Routine catalog, one per file under ``uds/routines/``.

    dtcs : list of \
    :class:`~flync.model.flync_4_diagnostics.uds.dtc.DiagnosticTroubleCode`, optional
        DTC catalog, one per file under ``uds/dtcs/``.
    """

    version: Literal["0.14"] = Field(default="0.14", description="the version of this config")
    timings: Annotated[
        UDSTimingProfileSet,
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(description="the UDS timing profiles")
    servers: Annotated[List[UDSServer], External()] = Field(default_factory=list, description="list of UDS servers")
    dids: Annotated[List[DataIdentifier], External()] = Field(default_factory=list, description="list of DIDs")
    routines: Annotated[List[Routine], External()] = Field(default_factory=list, description="list of routines")
    dtcs: Annotated[List[DiagnosticTroubleCode], External()] = Field(default_factory=list, description="list of DTCs")

    @model_validator(mode="after")
    def validate_server_names_unique(self) -> "UDSConfig":
        validate_list_items_unique([server.name for server in self.servers], "UDS server names")
        return self

    @model_validator(mode="after")
    def validate_did_ids_unique(self) -> "UDSConfig":
        validate_list_items_unique([did.name for did in self.dids], "DID names")
        validate_list_items_unique([did.did for did in self.dids], "DID identifiers")
        return self

    @model_validator(mode="after")
    def validate_routine_ids_unique(self) -> "UDSConfig":
        validate_list_items_unique([routine.name for routine in self.routines], "routine names")
        validate_list_items_unique([routine.rid for routine in self.routines], "routine identifiers")
        return self

    @model_validator(mode="after")
    def validate_dtc_ids_unique(self) -> "UDSConfig":
        validate_list_items_unique([dtc.name for dtc in self.dtcs], "DTC names")
        validate_list_items_unique([dtc.dtc for dtc in self.dtcs], "DTC identifiers")
        return self

    @model_validator(mode="after")
    def bind_servers(self) -> "UDSConfig":
        """
        Resolve every :class:`UDSServer`'s timings/DID/DTC/routine name references against
        this object's catalogs.
        """

        timings_by_id = self.timings.by_id()
        dids_by_name = {did.name: did for did in self.dids}
        routines_by_name = {routine.name: routine for routine in self.routines}
        dtcs_by_name = {dtc.name: dtc for dtc in self.dtcs}
        for server in self.servers:
            server.bind(timings_by_id, dids_by_name, dtcs_by_name, routines_by_name)
        return self

    def servers_by_name(self) -> dict[str, UDSServer]:
        """
        Return every UDS server of this config, keyed by name.
        """

        return {server.name: server for server in self.servers}
