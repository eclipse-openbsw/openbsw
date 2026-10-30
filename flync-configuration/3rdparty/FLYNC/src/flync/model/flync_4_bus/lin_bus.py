"""Defines the LIN bus model for FLYNC including its schedule tables."""

from typing import Annotated, List, Literal, Optional

from pydantic import BeforeValidator, Field, field_validator, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major, err_minor
from flync.core.validators.generic import none_to_empty_list
from flync.model.flync_4_nm import StateMembershipRef
from flync.model.flync_4_signal.frame import LINFrame

_ALLOWED_LIN_BAUD_RATES = {1_200, 2_400, 4_800, 9_600, 10_400, 19_200}


_LINProtocol = Literal["1.3", "2.0", "2.1", "2.2A"]


class LINScheduleEntry(FLYNCBaseModel):
    """
    Single entry in a LIN schedule table.

    Each entry specifies a frame to be transmitted and the slot period after which the next frame is scheduled.

    Parameters
    ----------
    frame_name : str
        Name of the :class:`~flync.model.flync_4_signal.frame.LINFrame` to transmit.
        Must reference a frame present in the owning :class:`LINBus`.
        Corresponds to the frame name in the LDF ``Schedule_tables`` entry.
    period : float
        Slot period in milliseconds.
        Corresponds to ``delay <period> ms`` in the LDF ``Schedule_tables`` block.
        Must be greater than zero.
    """

    frame_name: str = Field()
    period: Annotated[float, Field(gt=0)] = Field()


class LINScheduleTable(FLYNCBaseModel):
    """
    LIN schedule table.

    A named sequence of frame transmissions with associated slot periods.
    Corresponds to a single ``Schedule_tables`` entry in the LDF file.

    Parameters
    ----------
    name : str
        Unique name of the schedule table.
    description : str, optional
        Optional human-readable description.
    entries : list of :class:`LINScheduleEntry`
        Ordered list of frame-slot entries.
    """

    name: str = Field()
    description: Optional[str] = Field(default=None)
    entries: List[LINScheduleEntry] = Field(default_factory=list)


class LINBus(FLYNCBaseModel):
    """
    LIN bus configuration.

    Models the complete LIN bus, including the protocol header information needed for LDF file generation,
    the schedule tables, and the set of frames on the bus.  Node participation (master/slave role, NAD,
    timing parameters) is declared on the controller's LIN interface, not here.

    LDF file mapping:

    * ``lin_protocol_version`` → ``LIN_protocol_version``
    * ``lin_language_version`` → ``LIN_language_version``
    * ``baud_rate`` → ``LIN_speed`` (bits/s)
    * ``channel_name`` → ``Channel_name``
    * ``time_base`` → ``Time_base`` (ms)
    * ``jitter`` → ``Jitter`` (ms)
    * ``frames`` → ``Frames`` section
    * ``schedule_tables`` → ``Schedule_tables`` section
    * Signal encoding types and ``Signal_representation`` are derived from the ``Signal.factor``, ``Signal.offset``,
      ``Signal.lower_limit``, ``Signal.upper_limit``, ``Signal.unit``, and ``Signal.value_encoding`` fields during LDF export.

    Parameters
    ----------
    name : str
        Unique name of the LIN bus.
    description : str, optional
        Optional human-readable description.
    lin_protocol_version : Literal["1.3", "2.0", "2.1", "2.2A"]
        LIN protocol version for the ``LIN_protocol_version`` LDF header field.
    lin_language_version : Literal["1.3", "2.0", "2.1", "2.2A"]
        LIN description language version for the ``LIN_language_version`` LDF header field.
    baud_rate : int
        Bus bit rate in bits/s.
        Must be one of the standard LIN baud rates: 1 200, 2 400, 4 800, 9 600, 10 400, or 19 200 bits/s.
        Written as-is to the LDF ``LIN_speed`` field.
    channel_name : str, optional
        Optional LIN channel name for the LDF ``Channel_name`` field.
    time_base : float
        Scheduling time base in milliseconds.
        Defaults to ``5.0``. Corresponds to the LDF ``Time_base`` field.
    jitter : float
        Maximum scheduling jitter in milliseconds.
        Defaults to ``0.0``.  Corresponds to the LDF ``Jitter`` field.
    schedule_tables : list of :class:`LINScheduleTable`
        Named schedule tables for the LDF ``Schedule_tables`` section.
    frames : list of :class:`~flync.model.flync_4_signal.frame.LINFrame`
        Unconditional LIN frames for the LDF ``Frames`` section.
    state_memberships : list of \
    :class:`~flync.model.flync_4_nm.StateMembershipRef`, optional
        Assignments of this bus to a state management group.  A bus membership enrols the whole bus as one participant — the entire bus
        stays awake or sleeps as a unit, never expanded into per-attached-node participants.  It contributes one relevance bit by default
        (the bus name) and may reference several when the bus serves several functions.  LIN has no per-node NM, so a LIN bus always
        participates as a whole.
    """

    name: str = Field()
    description: Optional[str] = Field(default=None)
    lin_protocol_version: _LINProtocol = Field()
    lin_language_version: _LINProtocol = Field()
    baud_rate: int = Field()
    channel_name: Optional[str] = Field(default=None)
    time_base: float = Field(default=5.0)
    jitter: float = Field(default=0.0)
    schedule_tables: List[LINScheduleTable] = Field(default_factory=list)
    frames: List[LINFrame] = Field(default_factory=list)
    state_memberships: Annotated[
        Optional[List[StateMembershipRef]],
        BeforeValidator(none_to_empty_list),
    ] = Field(
        default_factory=list,
        description="Assignments of this bus to a state management group; the whole bus participates as one unit.",
    )

    @field_validator("baud_rate")
    @classmethod
    def validate_baud_rate(cls, value: int) -> int:
        """Ensure the baud rate is a standard LIN rate."""
        if value not in _ALLOWED_LIN_BAUD_RATES:
            raise err_minor(
                "baud_rate {value} is not a valid LIN baud rate. Allowed values: {allowed}",
                value=value,
                allowed=sorted(_ALLOWED_LIN_BAUD_RATES),
                category=Category.VALUE_RANGE,
                error_number="055",
            )
        return value

    @model_validator(mode="after")
    def validate_schedule_frame_references(self) -> "LINBus":
        """Ensure every schedule entry references a defined frame."""
        frame_names = {f.name for f in self.frames}
        for table in self.schedule_tables:
            for entry in table.entries:
                if entry.frame_name not in frame_names:
                    raise err_major(
                        "LINScheduleTable '{table}' references unknown frame '{frame}'. Defined frames: {defined}",
                        table=table.name,
                        frame=entry.frame_name,
                        defined=sorted(frame_names),
                        category=Category.REFERENCE,
                        error_number="056",
                    )
        return self
