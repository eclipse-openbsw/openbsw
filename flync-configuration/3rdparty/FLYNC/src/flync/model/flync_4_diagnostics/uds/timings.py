"""Defines UDS server timing profiles, loaded from ``communication/diagnostics/uds/timings.flync.yaml``."""

from typing import Annotated, List

from pydantic import Field, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import validate_list_items_unique


class UDSTimings(FLYNCBaseModel):
    """
    UDS server timing parameters, as defined by ISO 14229-2.

    Parameters
    ----------
    p2_server : int, optional
        Default UDS server response time in ms (``P2_server``). Defaults to ``50``.

    p2_star_server : int, optional
        Enhanced UDS server response time in ms after a pending response (``P2*_server``).
        Defaults to ``5000``.

    s3_server : int, optional
        Time in ms the server keeps a non-default diagnostic session active without tester
        presence (``S3_server``). Defaults to ``5000``.
    """

    p2_server: Annotated[int, Field(gt=0)] = Field(default=50)
    p2_star_server: Annotated[int, Field(gt=0)] = Field(default=5000)
    s3_server: Annotated[int, Field(gt=0)] = Field(default=5000)


class UDSTimingProfile(UDSTimings):
    """
    A named, reusable set of UDS server timings, referenced by a :class:`UDSServer`.

    Parameters
    ----------
    profile_id : str
        Unique identifier of this timing profile.

    p2_server : int, optional
        Default UDS server response time in ms (``P2_server``). Defaults to ``50``.

    p2_star_server : int, optional
        Enhanced UDS server response time in ms after a pending response (``P2*_server``).
        Defaults to ``5000``.

    s3_server : int, optional
        Time in ms the server keeps a non-default diagnostic session active without tester
        presence (``S3_server``). Defaults to ``5000``.
    """

    profile_id: str = Field()


class UDSTimingProfileSet(FLYNCBaseModel):
    """
    Container of UDS timing profiles, loaded from ``uds/timings.flync.yaml``.

    Parameters
    ----------
    profiles : list of :class:`UDSTimingProfile`, optional
        Timing profiles available for use by UDS servers.

    defaults : list of :class:`UDSTimingProfile`, optional
        Timing profiles used as system-wide defaults.
    """

    profiles: List[UDSTimingProfile] = Field(default_factory=list)
    defaults: List[UDSTimingProfile] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_profile_ids_unique(self) -> "UDSTimingProfileSet":
        """
        Raise when two timing profiles across ``profiles`` and ``defaults`` share a ``profile_id``.
        """

        validate_list_items_unique(
            [profile.profile_id for profile in self.profiles + self.defaults],
            "UDS timing profile ids",
        )
        return self

    def by_id(self) -> dict[str, UDSTimingProfile]:
        """
        Return every profile of this container, keyed by ``profile_id``.
        """

        return {profile.profile_id: profile for profile in self.profiles + self.defaults}
