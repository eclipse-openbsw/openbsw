"""Defines the DoIP configuration, loaded from ``communication/diagnostics/doip/``."""

from typing import Annotated, Literal

from pydantic import Field

from flync.core.annotations import External, OutputStrategy
from flync.core.base_models import FLYNCBaseModel

from .timings import DoIPTimingProfileSet


class DoIPConfig(FLYNCBaseModel):
    """
    The system-wide DoIP (ISO 13400) transport configuration.

    Parameters
    ----------
    version : Literal["0.14"], optional
        The version of this config. Defaults to ``"0.14"``.

    timings : :class:`~flync.model.flync_4_diagnostics.doip.timings.DoIPTimingProfileSet`
        DoIP timing profiles, loaded from ``doip/timings.flync.yaml``. Required: a ``doip/``
        folder without that file describes no transport at all.
    """

    version: Literal["0.14"] = Field(default="0.14", description="the version of this config")
    timings: Annotated[
        DoIPTimingProfileSet,
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(description="the DoIP timing profiles")
