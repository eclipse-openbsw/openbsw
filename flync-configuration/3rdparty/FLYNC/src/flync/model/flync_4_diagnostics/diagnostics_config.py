"""Defines the top-level diagnostics configuration, loaded from ``communication/diagnostics/``."""

from typing import Annotated, Literal, Optional

from pydantic import BeforeValidator, Field, model_validator

from flync.core.annotations import External, NamingStrategy, OutputStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import validate_or_remove

from .doip.doip_config import DoIPConfig
from .uds.uds_config import UDSConfig


class DiagnosticsConfig(FLYNCBaseModel):
    """
    Root object of a system's diagnostics configuration, one folder per diagnostic protocol.

    Everything FLYNC models today is UDS (ISO 14229) carried over DoIP (ISO 13400), so the
    two sub-configs are :attr:`doip` for the transport and :attr:`uds` for the diagnostic
    services; a further protocol would be added as a sibling folder.

    Parameters
    ----------
    version : Literal["0.14"], optional
        The version of this config. Defaults to ``"0.14"``.

    doip : :class:`~flync.model.flync_4_diagnostics.doip.doip_config.DoIPConfig`, optional
        DoIP transport configuration, loaded from ``diagnostics/doip/``. Absent when that
        directory does not exist.

    uds : :class:`~flync.model.flync_4_diagnostics.uds.uds_config.UDSConfig`, optional
        UDS configuration - servers plus the DID/routine/DTC catalogs - loaded from
        ``diagnostics/uds/``. Absent when that directory does not exist.
    """

    version: Literal["0.14"] = Field(default="0.14", description="the version of this config")

    doip: Annotated[
        Optional[DoIPConfig],
        External(output_structure=OutputStrategy.FOLDER, naming_strategy=NamingStrategy.FIELD_NAME),
        BeforeValidator(validate_or_remove("DoIP config", DoIPConfig)),
    ] = Field(default=None, description="contains the DoIP transport config for the entire system.")
    uds: Annotated[
        Optional[UDSConfig],
        External(output_structure=OutputStrategy.FOLDER, naming_strategy=NamingStrategy.FIELD_NAME),
        BeforeValidator(validate_or_remove("UDS config", UDSConfig)),
    ] = Field(default=None, description="contains the UDS config for the entire system.")

    @model_validator(mode="after")
    def validate_at_least_one_protocol(self) -> "DiagnosticsConfig":
        """
        Reject a diagnostics config that carries neither protocol.

        Raised as a plain ``ValueError`` on purpose: the workspace loader reads it as "this
        folder is not a diagnostics config" and leaves the field unset, which is what an
        absent (or empty) ``communication/diagnostics/`` directory must produce - rather
        than a phantom config that would then be written back to disk.
        """

        if self.doip is None and self.uds is None:
            raise ValueError("a diagnostics config must contain a 'doip' and/or a 'uds' sub-config")
        return self

    def doip_timings_by_id(self) -> dict:
        """
        Return every DoIP timing profile of the system, keyed by ``profile_id``.
        """

        return self.doip.timings.by_id() if self.doip else {}

    def uds_servers_by_name(self) -> dict:
        """
        Return every UDS server of the system, keyed by name.
        """

        return self.uds.servers_by_name() if self.uds else {}
