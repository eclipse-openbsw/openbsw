"""
State management modelling for FLYNC.

A :class:`StateManagementGroup` is a thin coordination construct: it owns an
identity, a reference to an ordinary NM PDU (``pdu_usage:
network_management``), and a reference to a shared, bus-abstract timing
profile (:class:`GroupTiming`, reusable across groups). It owns no
communication binding and no member list - memberships are declared
entity-side (ECU / controller / bus) via :class:`StateMembershipRef` and the
effective member set per group is derived during validation.

Semantics: each participant references one or more
state bits in the group's relevance vector, each with exactly two states -
*requested* and *released*. An entity may participate in N groups; it may
enter sleep only when ALL bits and groups it registered with have released it.
A bus-level membership makes the bus ONE participant: while requested the
whole bus is kept awake, when released it may sleep as a unit; that one
participant may reference several bits when the bus is needed for several
functions. The bus cannot speak NM itself, so a proxy is derived during
validation and never modelled: for a CAN bus the proxy is the entity that
feeds the group's NM frame onto that bus; for a LIN bus - which carries no NM
message - the proxy is a LIN master that drives the LIN sleep, either as the
source of the group state (e.g. a central gateway) or after receiving it on
another bus.

A CAN bus supports either variant, chosen per bus - node-level (its ECUs /
controllers, per function) or bus-level (the whole bus); a LIN bus is
bus-level only; point-to-point Ethernet is node-level. A CAN bus never mixes
both variants (enforced in validation); LIN is exempt from that rule. A LIN
master may additionally hold its own membership in the same group (e.g. a
central gateway that also participates with its own functions). Richer state
dimensions (e.g. eFuse states) are explicitly future scope.


A multidrop segment is node-level like any other Ethernet: its nodes are IP
hosts running their own NM, so they hold their own memberships.

Memberships are transport-independent: NM semantics are never expressed via
socket- or frame-level config. The group's NM PDU is bound to transports
through the ordinary, untouched mechanisms (Ethernet PDU socket deployments,
CAN sender/receiver frames); validation only cross-checks that a member's
role has a matching binding (participant: TX and RX path, observer: RX
path). Until app-level membership lands with flync_4_app, controller-level
membership is the documented recommendation for multi-controller ECUs.
"""

from typing import TYPE_CHECKING, Annotated, Dict, List, Literal, NamedTuple, Optional

from pydantic import BeforeValidator, Field, model_validator

from flync.core.annotations import External, OutputStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.generic import none_to_empty_list, validate_list_items_unique

if TYPE_CHECKING:  # pragma: no cover
    from flync.model.flync_model import FLYNCModel


class EffectiveMember(NamedTuple):
    """
    One resolved membership after merging all declaration sites.

    Parameters
    ----------
    group : str
        Name of the referenced state management group.
    entity_kind : Literal["ecu", "controller", "app", "bus"]
        Declaration site kind. ``app`` is reserved for the flync_4_app
        iteration; no walk produces it yet.
    entity_path : str
        Human-readable path of the declaring entity,
        e.g. ``"GatewayEcu"``, ``"GatewayEcu/can_1"``, or ``"BodyCan"``.
    ecu_name : str, optional
        Host ECU of the declaring entity (equal to ``entity_path`` for
        ecu-kind members). ``None`` for bus members - a bus has no host ECU.
    role : Literal["participant", "observer"]
        Membership role.
    relevance_bit : str, optional
        State bit referenced by the member; defaulted to the declaring entity's
        name for participants. ``None`` for observers, which reference no bit.
    """

    group: str
    entity_kind: Literal["ecu", "controller", "app", "bus"]
    entity_path: str
    ecu_name: Optional[str]
    role: Literal["participant", "observer"]
    relevance_bit: Optional[str]


class AnnouncementPhaseTiming(FLYNCBaseModel):
    """
    Timing of a group's announcement phase.

    Whenever a bus participant changes the group's state - for instance when it
    (re)joins the network AND when it begins the go-to-sleep progression, i.e.
    on state changes generally, not only after a wake-up - it re-announces
    itself for a bounded window so every member can resynchronize the derived
    member set before normal operation resumes. The phase is optional: a group
    that does not model it simply omits ``announcement``.

    Parameters
    ----------
    duration_ms : int
        Total duration of the announcement phase in milliseconds, during which
        every member transmits so the member set can resynchronize. Must be greater than 0.

    burst_count : int, optional
        Number of NM PDUs sent back-to-back at the start of the phase (the
        initial burst) at ``burst_cycle_time_ms`` before the normal cadence
        resumes. Must be set together with ``burst_cycle_time_ms``; the burst
        must fit within ``duration_ms``. Must be greater than 0.

    burst_cycle_time_ms : int, optional
        Faster transmission period used for those initial burst PDUs to
        propagate the state change quickly, before falling back to the group's
        ``cycle_time_ms``. Must be set together with ``burst_count`` and be
        shorter than ``cycle_time_ms``. Must be greater than 0.
    """

    duration_ms: int = Field(gt=0)
    burst_count: Optional[int] = Field(default=None, gt=0)
    burst_cycle_time_ms: Optional[int] = Field(default=None, gt=0)

    @model_validator(mode="after")
    def check_burst_pair(self):
        """Raise a major error when only one of the two burst fields is set."""
        burst_set = (self.burst_count is not None, self.burst_cycle_time_ms is not None)
        if any(burst_set) and not all(burst_set):
            raise err_major(
                "announcement phase timing: burst_count and burst_cycle_time_ms must be set together (set both or neither)",
                category=Category.VALUE_RANGE,
                error_number="203",
            )
        return self


class SleepTiming(FLYNCBaseModel):
    """
    Timing of a group's go-to-sleep progression.

    Parameters
    ----------
    timeout_ms : int
        Time in milliseconds without any NM PDU on the channel after which an
        already-released entity advances along the sleep progression toward the
        final wait phase. This silence timer does NOT decide release: while any
        bit is still requested the network stays awake regardless of it. The
        timer restarts on every NM PDU sent or received, so ongoing NM traffic
        keeps the group awake; actual sleep follows only once every registered
        bit is released and ``wait_before_sleep_ms`` has elapsed. Must be greater than 0.

    wait_before_sleep_ms : int
        Time in milliseconds a node waits, after the network has gone quiet
        and all its registered bits are released, before it finally enters
        sleep - the last delay of the sleep progression. Must be greater than 0.
    """

    timeout_ms: int = Field(gt=0)
    wait_before_sleep_ms: int = Field(gt=0)


class GroupTiming(FLYNCBaseModel):
    """
    A reusable, bus-abstract NM timing profile.

    A profile is defined once in the registry and referenced by name from any
    number of groups (``StateManagementGroup.timing_profile``), so groups that
    share the same timing reuse a single definition instead of repeating the
    values. Timing is never overridden per bus or per member; feasibility per
    bus segment is a validation concern. The parameters are grouped by phase:
    the normal-operation cycle time stays at the top level, while the
    announcement phase and the go-to-sleep progression each carry their own
    sub-model.

    Parameters
    ----------
    name : str
        Unique name of the profile (e.g. ``standard``, ``fast``).

    description : str, optional
        Optional human-readable description.

    cycle_time_ms : int
        Cyclic transmission period of the NM PDU in milliseconds during normal
        operation. Must be greater than 0.

    announcement : AnnouncementPhaseTiming, optional
        Timing of the announcement phase that runs on state changes (e.g.
        joining the network or entering the sleep progression). Optional -
        omit it for a group that does not model an announcement phase.

    sleep : SleepTiming
        Timing of the go-to-sleep progression (silence timeout and final
        wait).

    extensions : dict of str to str, optional
        Optional map of extension keys and values for OEM- or tool-specific
        timing parameters, carried through untouched (never interpreted).
    """

    name: str = Field()
    description: Optional[str] = Field(default=None)
    cycle_time_ms: int = Field(gt=0)
    announcement: Optional[AnnouncementPhaseTiming] = Field(default=None)
    sleep: SleepTiming = Field()
    extensions: Optional[Dict[str, str]] = Field(default=None)

    @model_validator(mode="after")
    def check_consistency(self):
        """Validate the cross-phase timer relationships (sleep timeout, and the optional announcement burst)."""
        if self.sleep.timeout_ms <= self.cycle_time_ms:
            raise err_major(
                "timing profile '{name}': sleep.timeout_ms must be greater than cycle_time_ms (got timeout_ms={timeout}, cycle_time_ms={cycle})",
                name=self.name,
                timeout=self.sleep.timeout_ms,
                cycle=self.cycle_time_ms,
                category=Category.VALUE_RANGE,
                error_number="204",
            )
        announcement = self.announcement
        if announcement is not None and announcement.burst_cycle_time_ms is not None:
            if announcement.burst_cycle_time_ms >= self.cycle_time_ms:
                raise err_major(
                    "timing profile '{name}': announcement.burst_cycle_time_ms ({burst}) must be shorter than cycle_time_ms ({cycle})",
                    name=self.name,
                    burst=announcement.burst_cycle_time_ms,
                    cycle=self.cycle_time_ms,
                    category=Category.VALUE_RANGE,
                    error_number="205",
                )
            if announcement.burst_count * announcement.burst_cycle_time_ms > announcement.duration_ms:
                raise err_major(
                    "timing profile '{name}': the announcement burst ({count} x {burst} ms) does not fit within the "
                    "announcement duration_ms ({duration})",
                    name=self.name,
                    count=announcement.burst_count,
                    burst=announcement.burst_cycle_time_ms,
                    duration=announcement.duration_ms,
                    category=Category.VALUE_RANGE,
                    error_number="206",
                )
        return self


class StateManagementGroup(FLYNCBaseModel):
    """
    A user-defined state management group.

    The group is the single owner of identity and transport reference, and
    references a shared timing profile by name. It intentionally has NO member
    list: memberships are declared on the participating entities
    (:class:`StateMembershipRef`) and resolved during system validation.

    Parameters
    ----------
    name : str
        Unique name of the group (e.g. ``COMFORT``, ``VEHICLE``).

    nm_pdu : str
        Name reference to an existing :class:`~flync.model.flync_4_signal.pdu.StandardPDU`
        with ``pdu_usage: network_management``. The PDU carries the group's
        relevance vector as an ordinary bitmask-encoded signal.

    timing_profile : str
        Name reference to a :class:`GroupTiming` profile defined in the
        registry. Several groups may reference the same profile.

    description : str, optional
        Optional human-readable description.

    extensions : dict of str to str, optional
        Optional map of extension keys and values for OEM- or tool-specific
        parameters. FLYNC carries them through untouched - they are never
        interpreted by the model or validation - so an OEM can attach its own
        per-group settings without a schema change.
    """

    name: str = Field()
    nm_pdu: str = Field()
    timing_profile: str = Field()
    description: Optional[str] = Field(default=None)
    extensions: Optional[Dict[str, str]] = Field(default=None)
    _effective_members: List[EffectiveMember] = []


class StateMembershipRef(FLYNCBaseModel):
    """
    Entity-side assignment of an ECU, controller, or bus to a state
    management group.

    Entities declare membership ONLY - never timing and never the NM PDU;
    everything the group owns stays in the central group registry. A bus
    membership enrols the bus as a single participant (default bit: the bus
    name) - it is never expanded into per-attached-ECU participants.

    Parameters
    ----------
    group : str
        Name reference to a :class:`StateManagementGroup` defined in
        ``communication/state_management/groups.flync.yaml``.

    role : Literal["participant", "observer"], optional
        ``participant`` (default) references a state bit and takes part in the
        wake/sleep decision. ``observer`` receives the group's NM PDU and
        acts on it (e.g. a switch pulling a wake line) but references no bit
        and never influences the group's sleep decision. The role is also the
        permission model: only participants may request the group state,
        because requesting means setting a relevance bit.

    relevance_bits : list of str, optional
        Names of the state bits in the group PDU's relevance vector this
        participant references. A participant may reference several - one per
        vehicle function it takes part in - listed in a single membership
        instead of repeating the block per bit. Defaults to a single bit named
        after the declaring entity. Must be absent for observers.

    extensions : dict of str to str, optional
        Optional map of extension keys and values for OEM- or tool-specific
        per-membership parameters, carried through untouched (never
        interpreted).
    """

    group: str = Field()
    role: Literal["participant", "observer"] = Field(default="participant")
    relevance_bits: Optional[List[str]] = Field(default=None)
    extensions: Optional[Dict[str, str]] = Field(default=None)

    @model_validator(mode="after")
    def observer_owns_no_bit(self):
        """Raise a major error when an observer membership defines relevance bits."""
        if self.role == "observer" and self.relevance_bits:
            raise err_major(
                f"observer membership in group '{self.group}' must not define relevance_bits - only participants reference bits",
                category=Category.CONSISTENCY,
                error_number="207",
            )
        return self

    @model_validator(mode="after")
    def no_duplicate_bits(self):
        """Raise a major error when the same relevance bit is listed more than once."""
        if self.relevance_bits and len(self.relevance_bits) != len(set(self.relevance_bits)):
            raise err_major(
                f"membership in group '{self.group}' lists a relevance bit more than once in relevance_bits",
                category=Category.UNIQUENESS,
                error_number="208",
            )
        return self


class StateManagementConfig(FLYNCBaseModel):
    """
    System-wide state management configuration.

    Serialized to the ``communication/state_management/`` directory: the group
    registry lands in ``groups.flync.yaml`` and the reusable timing profiles in
    ``timing_profiles.flync.yaml``. Designed to absorb future siblings (e.g.
    eFuse state dependencies) as additional files in the same directory without
    structural change.

    Parameters
    ----------
    groups : list of :class:`StateManagementGroup`, optional
        The central registry of state management groups. Defaults to an empty list.

    timing_profiles : list of :class:`GroupTiming`, optional
        The reusable NM timing profiles referenced by the groups. Defaults to an empty list.
    """

    groups: Annotated[
        List[StateManagementGroup],
        External(output_structure=OutputStrategy.SINGLE_FILE),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    timing_profiles: Annotated[
        List[GroupTiming],
        External(output_structure=OutputStrategy.SINGLE_FILE),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])

    @model_validator(mode="after")
    def validate_unique_names(self):
        """Raise a major error when two groups, or two timing profiles, share a name."""
        validate_list_items_unique([group.name for group in self.groups], "state management groups (name)")
        validate_list_items_unique([profile.name for profile in self.timing_profiles], "state management timing profiles (name)")
        return self


def collect_effective_members(model: "FLYNCModel") -> Dict[str, List[EffectiveMember]]:
    """
    Collect effective members from every source and group them by group name.

    ECU and controller memberships are pre-populated during
    ``ECU.model_post_init()`` into ``ecu._state_effective_members``.
    Bus memberships (buses have no ``model_post_init``) are collected here
    and merged with the ECU-level ones. The caller assigns the result to
    each ``StateManagementGroup._effective_members``.

    App level is added once flync_4_app lands - the walk gains one loop,
    nothing else changes.
    """

    members: Dict[str, List[EffectiveMember]] = {}
    for ecu in model.ecus:
        for m in getattr(ecu, "_state_effective_members", []):
            members.setdefault(m.group, []).append(m)
    for bus in _iter_buses(model):
        for ref in bus.state_memberships or []:
            _add_bus_member(members, ref, bus.name)
    return members


def _add_bus_member(members: Dict[str, List[EffectiveMember]], ref, bus_name: str) -> None:
    """Append bus-level EffectiveMember entries - mirrors ECU.__add_effective_state_member for buses."""

    if ref.role == "observer":
        members.setdefault(ref.group, []).append(EffectiveMember(ref.group, "bus", bus_name, None, ref.role, None))
        return
    bits = ref.relevance_bits or [bus_name.rsplit("/", 1)[-1]]
    for bit in bits:
        members.setdefault(ref.group, []).append(EffectiveMember(ref.group, "bus", bus_name, None, ref.role, bit))


def _iter_buses(model: "FLYNCModel"):
    """Yield every CAN and LIN bus declared under ``communication.channels``."""

    channels = getattr(model.communication, "channels", None) if model.communication else None
    if channels is None:
        return
    for buses in (channels.can_buses, channels.lin_buses):
        yield from buses or []
