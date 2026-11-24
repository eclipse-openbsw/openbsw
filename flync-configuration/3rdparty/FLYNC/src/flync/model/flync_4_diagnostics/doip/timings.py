"""Defines DoIP timing profiles, loaded from ``communication/diagnostics/doip/timings.flync.yaml``."""

from typing import Annotated, List

from pydantic import Field, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import validate_list_items_unique


class DoIPTimings(FLYNCBaseModel):
    """
    DoIP protocol timers, as defined by ISO 13400.

    Parameters
    ----------
    a_doip_ctrl : int, optional
        Maximum time in ms to wait for a diagnostic response over DoIP (``A_DoIP_Ctrl``).
        Defaults to ``2000``.

    t_tcp_general_inactivity : int, optional
        General inactivity timeout in ms for an open DoIP TCP connection. Defaults to ``300000``.

    t_tcp_initial_inactivity : int, optional
        Timeout in ms for the first diagnostic message after a TCP connection is opened.
        Defaults to ``2000``.

    t_tcp_alive_check : int, optional
        Timeout in ms to wait for a response to an alive check request. Defaults to ``500``.

    a_doip_announce_num : int, optional
        Number of vehicle announcement messages sent after DoIP entity power-up. Defaults to ``3``.

    a_doip_announce_interval : int, optional
        Interval in ms between successive vehicle announcement messages. Defaults to ``500``.

    a_doip_announce_wait : int, optional
        Initial delay in ms before the first vehicle announcement message is sent.
        Defaults to ``500``.
    """

    a_doip_ctrl: Annotated[int, Field(gt=0)] = Field(default=2000)
    t_tcp_general_inactivity: Annotated[int, Field(gt=0)] = Field(default=300000)
    t_tcp_initial_inactivity: Annotated[int, Field(gt=0)] = Field(default=2000)
    t_tcp_alive_check: Annotated[int, Field(gt=0)] = Field(default=500)
    a_doip_announce_num: Annotated[int, Field(gt=0)] = Field(default=3)
    a_doip_announce_interval: Annotated[int, Field(gt=0)] = Field(default=500)
    a_doip_announce_wait: Annotated[int, Field(gt=0)] = Field(default=500)


class DoIPTimingProfile(DoIPTimings):
    """
    A named, reusable set of DoIP timers, referenced by a DoIP socket deployment.

    Parameters
    ----------
    profile_id : str
        Unique identifier of this timing profile.

    a_doip_ctrl : int, optional
        Maximum time in ms to wait for a diagnostic response over DoIP (``A_DoIP_Ctrl``).
        Defaults to ``2000``.

    t_tcp_general_inactivity : int, optional
        General inactivity timeout in ms for an open DoIP TCP connection. Defaults to ``300000``.

    t_tcp_initial_inactivity : int, optional
        Timeout in ms for the first diagnostic message after a TCP connection is opened.
        Defaults to ``2000``.

    t_tcp_alive_check : int, optional
        Timeout in ms to wait for a response to an alive check request. Defaults to ``500``.

    a_doip_announce_num : int, optional
        Number of vehicle announcement messages sent after DoIP entity power-up. Defaults to ``3``.

    a_doip_announce_interval : int, optional
        Interval in ms between successive vehicle announcement messages. Defaults to ``500``.

    a_doip_announce_wait : int, optional
        Initial delay in ms before the first vehicle announcement message is sent.
        Defaults to ``500``.
    """

    profile_id: str = Field()


class DoIPTimingProfileSet(FLYNCBaseModel):
    """
    Container of DoIP timing profiles, loaded from ``doip/timings.flync.yaml``.

    Parameters
    ----------
    profiles : list of :class:`DoIPTimingProfile`, optional
        Timing profiles available for use by DoIP deployments.

    defaults : list of :class:`DoIPTimingProfile`, optional
        Timing profiles used as system-wide defaults.
    """

    profiles: List[DoIPTimingProfile] = Field(default_factory=list)
    defaults: List[DoIPTimingProfile] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_profile_ids_unique(self) -> "DoIPTimingProfileSet":
        """
        Raise when two timing profiles across ``profiles`` and ``defaults`` share a ``profile_id``.
        """

        validate_list_items_unique(
            [profile.profile_id for profile in self.profiles + self.defaults],
            "DoIP timing profile ids",
        )
        return self

    def by_id(self) -> dict[str, DoIPTimingProfile]:
        """
        Return every profile of this container, keyed by ``profile_id``.
        """

        return {profile.profile_id: profile for profile in self.profiles + self.defaults}
