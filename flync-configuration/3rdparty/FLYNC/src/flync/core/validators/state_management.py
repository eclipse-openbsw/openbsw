"""
Cross-model validation for state management groups.

Invoked from a ``model_validator`` on :class:`~flync.model.flync_model.FLYNCModel`.

The derived effective member set per group lives on each
:class:`~flync.model.flync_4_nm.state_management.StateManagementGroup`
as ``_effective_members`` (``PrivateAttr``). The members are collected by
:func:`~flync.model.flync_4_nm.state_management.collect_effective_members`.
ECU and controller-level memberships are pre-populated during
``ECU.model_post_init()``; bus-level memberships are added by that function.
"""

from typing import TYPE_CHECKING, Dict, NamedTuple, Set

from flync.core.utils.exceptions import Category, err_major, warn
from flync.core.validators.forwarder import (
    _build_can_frame_catalog_by_bus_id,
    _build_pdu_catalog,
    _iter_sockets_on_controller,
)
from flync.model.flync_4_nm.state_management import (
    _iter_buses,
    collect_effective_members,
)
from flync.model.flync_4_signal.pdu import ContainerPDU
from flync.model.flync_4_signal.pdu_deployment import PDUReceiver, PDUSender
from flync.model.flync_4_signal.value_encoding import BitmaskFlags

if TYPE_CHECKING:  # pragma: no cover
    from flync.model.flync_model import FLYNCModel


class _ValidationContext(NamedTuple):
    """Model-wide catalogs shared by every group check, built once per model."""

    model: "FLYNCModel"
    timing_profiles: Set[str]
    lin_bus_names: Set[str]
    pdu_catalog: dict
    frame_by_bus_id: dict
    buses_by_name: dict
    ecus_by_name: dict
    sent_frame_ids_by_bus: Dict[str, Set[int]]
    attached_buses_by_ecu: Dict[str, Set[str]]
    paths_cache: Dict[str, "tuple[Set[str], Set[str]]"]


def _build_context(model: "FLYNCModel", cfg) -> _ValidationContext:
    """Assemble the group-independent catalogs (PDUs, frames, bus topology, timing profiles)."""

    channels = getattr(model.communication, "channels", None) if model.communication else None
    sent_frame_ids_by_bus, attached_buses_by_ecu = _bus_topology_index(model)
    return _ValidationContext(
        model=model,
        timing_profiles={profile.name for profile in (cfg.timing_profiles if cfg else [])},
        lin_bus_names={bus.name for bus in (channels.lin_buses or [])} if channels else set(),
        pdu_catalog=_build_pdu_catalog(model),
        frame_by_bus_id=_build_frame_catalog_by_bus_id(model),
        buses_by_name={bus.name: bus for bus in _iter_buses(model)},
        ecus_by_name={ecu.name: ecu for ecu in model.ecus},
        sent_frame_ids_by_bus=sent_frame_ids_by_bus,
        attached_buses_by_ecu=attached_buses_by_ecu,
        paths_cache={},
    )


def _paths(ctx: _ValidationContext, ecu_name: str) -> "tuple[Set[str], Set[str]]":
    """Return an ECU's ``(tx, rx)`` PDU-name sets, cached (paths are group-independent)."""

    cached = ctx.paths_cache.get(ecu_name)
    if cached is None:
        cached = _pdu_paths_by_ecu(ctx.ecus_by_name[ecu_name], ctx.pdu_catalog, ctx.frame_by_bus_id)
        ctx.paths_cache[ecu_name] = cached
    return cached


def validate_state_management(model: "FLYNCModel") -> None:
    """Run all state management rules. Called from FLYNCModel."""

    cfg = model.communication.state_management if model.communication else None
    members = collect_effective_members(model)
    groups = {g.name: g for g in (cfg.groups if cfg else [])}
    _check_referenced_groups_exist(members, groups)
    # No groups means no members either (any member would have raised above),
    # so skip the potentially large catalog passes — this keeps the validator
    # free for every workspace that predates the feature.
    if not groups:
        return
    ctx = _build_context(model, cfg)
    for name, group in groups.items():
        group._effective_members = members.get(name, [])
        _validate_group(name, group, ctx)


def _check_referenced_groups_exist(members, groups) -> None:
    """Every group named in a ``state_memberships`` entry must exist in the registry."""

    for group_name in members:
        if group_name not in groups:
            raise err_major(
                "state_memberships reference undefined state management group '{group}' — "
                "define it in communication/state_management/groups.flync.yaml",
                group=group_name,
                category=Category.REFERENCE,
                error_number="189",
            )


def _validate_group(name, group, ctx: _ValidationContext) -> None:
    """Run every per-group rule: basics, relevance bits, single bus variant, reachability, wake lines, redundancy, timing."""

    participants = [m for m in group._effective_members if m.role == "participant"]
    _check_group_basics(name, group, participants, ctx)
    # Basics has already resolved the NM PDU, so the lookup here always hits.
    _check_relevance_bits(name, group, participants, ctx.pdu_catalog.get(group.nm_pdu))
    _check_single_variant_per_bus(name, participants, ctx)
    _check_group_reachability(name, group, group._effective_members, ctx)
    _warn_redundant_memberships(name, participants)
    _check_timing_feasibility(ctx.model, group)


def _check_group_basics(name, group, participants, ctx: _ValidationContext) -> None:
    """A group needs a participant, a resolvable network-management nm_pdu, and a defined timing profile."""

    if not participants:
        raise err_major(
            "state management group '{group}' has no participant — assign at least one entity via state_memberships",
            group=name,
            category=Category.REQUIRED,
            error_number="190",
        )
    pdu = ctx.pdu_catalog.get(group.nm_pdu)
    if pdu is None:
        raise err_major(
            "state management group '{group}': nm_pdu '{pdu}' not found under communication.channels",
            group=name,
            pdu=group.nm_pdu,
            category=Category.REFERENCE,
            error_number="191",
        )
    if pdu.pdu_usage != "network_management":
        raise err_major(
            "state management group '{group}': PDU '{pdu}' has pdu_usage '{usage}', expected 'network_management'",
            group=name,
            pdu=group.nm_pdu,
            usage=pdu.pdu_usage,
            category=Category.REFERENCE,
            error_number="192",
        )
    if group.timing_profile not in ctx.timing_profiles:
        raise err_major(
            "state management group '{group}': timing_profile '{profile}' not found — "
            "define it in communication/state_management/timing_profiles.flync.yaml",
            group=name,
            profile=group.timing_profile,
            category=Category.REFERENCE,
            error_number="193",
        )


def _check_group_reachability(name, group, effective, ctx: _ValidationContext) -> None:
    """
    Cross-check each member's role against its transport bindings.

    A LIN bus is checked via its master-as-proxy; a CAN bus via its NM frame
    binding plus an attached sender; an ECU / controller member via its own
    TX/RX paths. Validation only — a missing binding is an error, never
    implicitly created.
    """

    for m in effective:
        if m.entity_kind == "bus" and m.entity_path in ctx.lin_bus_names:
            _check_lin_bus_reachability(name, group, m.entity_path, ctx)
        elif m.entity_kind == "bus":
            _check_bus_reachability(name, group, ctx.buses_by_name.get(m.entity_path), ctx.sent_frame_ids_by_bus)
        elif m.ecu_name in ctx.ecus_by_name:
            _check_ecu_member_reachability(name, group, m, ctx)


def _check_ecu_member_reachability(name, group, member, ctx: _ValidationContext) -> None:
    """
    A participant needs a TX path (announce its own state) AND an RX path
    (observe the group state — without it, it could never sleep correctly);
    an observer needs an RX path only.
    """

    tx, rx = _paths(ctx, member.ecu_name)
    if member.role == "participant" and group.nm_pdu not in tx:
        raise _reachability_error(name, group, member, "sender")
    if group.nm_pdu not in rx:  # required for participants and observers alike
        raise _reachability_error(name, group, member, "receiver")


def _warn_redundant_memberships(name, participants) -> None:
    """Emit warnings (never errors) for redundant-but-harmless declarations."""

    _warn_controller_under_ecu(name, participants)


def _warn_controller_under_ecu(name, participants) -> None:
    """Warn when a controller participant's host ECU is already a whole-ECU participant."""

    ecu_level = {m.ecu_name for m in participants if m.entity_kind == "ecu"}
    for m in participants:
        if m.entity_kind == "controller" and m.ecu_name in ecu_level:
            warn(
                f"state management group '{name}': '{m.entity_path}' is redundant — "
                f"its host ECU '{m.ecu_name}' is already a whole-ECU participant",
                category=Category.CONSISTENCY,
                error_number="194",
            )


def _check_single_variant_per_bus(name, participants, ctx: _ValidationContext) -> None:
    """
    A CAN bus is modelled with ONE membership variant: bus-level (the whole bus
    as one participant) OR node-level (its ECUs / controllers as individual
    participants) — never both, that would model the same bus twice. LIN buses
    are exempt: LIN has no per-node NM, so an attached ECU's own membership is
    necessarily about another transport (e.g. the master's Ethernet side) and
    may coexist with the bus-level membership — the central-gateway case.
    """

    bus_level = {m.entity_path for m in participants if m.entity_kind == "bus"} - ctx.lin_bus_names
    if not bus_level:
        return
    for m in participants:
        if m.entity_kind not in ("ecu", "controller") or m.ecu_name is None:
            continue
        clash = sorted(ctx.attached_buses_by_ecu.get(m.ecu_name, set()) & bus_level)
        if clash:
            raise err_major(
                "state management group '{group}': bus '{bus}' has a bus-level membership (the whole bus as one "
                "participant) but node-level member '{node}' on it also participates — choose one variant per bus "
                "(whole-bus or per-node), not both",
                group=name,
                bus=clash[0],
                node=m.entity_path,
                category=Category.CONSISTENCY,
                error_number="195",
            )


def _reachability_error(group_name, group, member, direction):
    """
    Build the reachability error for a member missing a transport path.

    ``direction`` is ``"sender"`` (TX) or ``"receiver"`` (RX); the message names
    the missing binding on both the Ethernet and the CAN/LIN side.
    """

    label = "TX" if direction == "sender" else "RX"
    deployment = "pdu_sender" if direction == "sender" else "pdu_receiver"
    missing = "its host ECU '{ecu}' has neither a {deployment} socket deployment nor a CAN/LIN {direction}_frames entry carrying it"
    if member.role == "participant" and direction == "receiver":
        missing = "it cannot observe the group state and would never sleep correctly; " + missing
    return err_major(
        "state management group '{group}': {role} '{member}' has no {label} path for NM PDU '{pdu}' — " + missing,
        group=group_name,
        role=member.role,
        member=member.entity_path,
        label=label,
        pdu=group.nm_pdu,
        ecu=member.ecu_name,
        deployment=deployment,
        direction=direction,
        category=Category.REFERENCE,
        error_number="196",
    )


def _bus_topology_index(model: "FLYNCModel"):
    """
    One pass over every CAN and LIN interface.

    Returns ``(sent_frame_ids_by_bus, attached_buses_by_ecu)``: the frame ids any
    attached interface transmits per bus, and the set of buses each ECU attaches
    to. Both feed group-independent checks, so they are built once per model.
    """

    sent_frame_ids_by_bus: Dict[str, Set[int]] = {}
    attached_buses_by_ecu: Dict[str, Set[str]] = {}
    for ecu in model.ecus:
        for controller in ecu.controllers:
            for iface in (controller.can_interfaces or []) + (controller.lin_interfaces or []):
                _index_bus_interface(iface, ecu.name, sent_frame_ids_by_bus, attached_buses_by_ecu)
    return sent_frame_ids_by_bus, attached_buses_by_ecu


def _index_bus_interface(iface, ecu_name, sent_frame_ids_by_bus, attached_buses_by_ecu) -> None:
    """Record one interface's bus attachment and the frame ids it transmits."""

    bus_ref = getattr(iface, "bus_ref", None)
    if bus_ref is None:
        return
    attached_buses_by_ecu.setdefault(ecu_name, set()).add(bus_ref)
    for ref in getattr(iface, "sender_frames", None) or []:
        sent_frame_ids_by_bus.setdefault(bus_ref, set()).add(ref.frame_ref)


def _build_frame_catalog_by_bus_id(model: "FLYNCModel"):
    """
    Return a ``(bus_name, frame_id)``-keyed dict of every CAN and LIN frame.

    Extends the CAN catalog from the forwarder validators with LIN frames
    (keyed by ``lin_id``) so LIN masters resolve their sender/receiver frames
    the same way CAN interfaces do.
    """

    catalog = dict(_build_can_frame_catalog_by_bus_id(model))
    channels = getattr(model.communication, "channels", None) if model.communication else None
    for bus in channels.lin_buses or [] if channels else []:
        for frame in bus.frames or []:
            catalog[(bus.name, frame.lin_id)] = frame
    return catalog


def _pdu_paths_by_ecu(ecu, pdu_catalog, frame_by_bus_id) -> "tuple[Set[str], Set[str]]":
    """
    Return ``(tx, rx)`` — the names of every PDU the ECU sends respectively
    receives on any path.

    Ethernet: socket deployments of type pdu_sender (TX) / pdu_receiver (RX),
    following one level of Container PDU indirection. CAN / LIN:
    sender_frames (TX) / receiver_frames (RX) of every CAN or LIN interface,
    resolved to the packed PDUs of the referenced frames. These are the
    ordinary transport bindings of an ordinary PDU — state management adds
    no binding mechanism of its own.
    """

    tx: Set[str] = set()
    rx: Set[str] = set()
    for controller in ecu.controllers:
        _collect_socket_pdus(controller, tx, rx, pdu_catalog)
        _collect_frame_pdus(controller, tx, rx, frame_by_bus_id)
    return tx, rx


def _collect_socket_pdus(controller, tx: Set[str], rx: Set[str], pdu_catalog) -> None:
    """Add the Ethernet socket-deployed PDUs of a controller to ``tx`` / ``rx``."""

    for socket in _iter_sockets_on_controller(controller):
        for dep_root in socket.deployments or []:
            dep = dep_root.root
            if isinstance(dep, PDUSender):
                _add_pdu_with_contained(tx, dep.pdu_ref, pdu_catalog)
            elif isinstance(dep, PDUReceiver):
                _add_pdu_with_contained(rx, dep.pdu_ref, pdu_catalog)


def _collect_frame_pdus(controller, tx: Set[str], rx: Set[str], frame_by_bus_id) -> None:
    """Add the CAN/LIN sender/receiver-frame PDUs of a controller to ``tx`` / ``rx``."""

    for iface in (controller.can_interfaces or []) + (controller.lin_interfaces or []):
        for frame_ref in getattr(iface, "sender_frames", None) or []:
            _add_frame_pdus(tx, frame_ref, frame_by_bus_id)
        for frame_ref in getattr(iface, "receiver_frames", None) or []:
            _add_frame_pdus(rx, frame_ref, frame_by_bus_id)


def _add_pdu_with_contained(target: Set[str], pdu_ref: str, pdu_catalog) -> None:
    """Add a deployed PDU ref plus, for Container PDUs, every contained PDU ref."""

    target.add(pdu_ref)
    carrier = pdu_catalog.get(pdu_ref)
    if isinstance(carrier, ContainerPDU):
        target.update(contained.pdu_ref for contained in carrier.contained_pdus)


def _add_frame_pdus(target: Set[str], frame_ref, frame_by_bus_id) -> None:
    """Add every PDU packed in the referenced CAN or LIN frame."""

    frame = frame_by_bus_id.get((frame_ref.bus_ref, frame_ref.frame_ref))
    if frame is not None:
        target.update(inst.pdu_ref for inst in frame.packed_pdus or [])


def _check_lin_bus_reachability(group_name, group, bus_name, ctx: _ValidationContext) -> None:
    """
    LIN branch of the reachability check.

    A LIN bus carries no NM PDU and no relevance vector — the LIN master
    unilaterally commands the bus to sleep, no slave approval needed (LIN has
    no NM message exchange and no coordination protocol; a wake-up may be
    initiated by any node). A LIN bus participant is therefore valid when an
    attached LIN master's ECU KNOWS the group state: either it RECEIVES the
    group's NM PDU on another binding (Ethernet / CAN), or it SENDS the PDU
    there itself — then it is a source of the group state (e.g. a central
    gateway) and needs no other bus to learn it from.
    """

    if _lin_master_knows_group_state(bus_name, group.nm_pdu, ctx):
        return
    raise err_major(
        "state management group '{group}': LIN bus participant '{bus}' has no master whose ECU receives or sends NM PDU "
        "'{pdu}' on another bus — the LIN master must know the group state (as its source, or by receiving it) "
        "to drive the bus sleep",
        group=group_name,
        bus=bus_name,
        pdu=group.nm_pdu,
        category=Category.REFERENCE,
        error_number="197",
    )


def _lin_master_knows_group_state(bus_name, pdu_ref, ctx: _ValidationContext) -> bool:
    """True if a LIN master of ``bus_name`` runs on an ECU that receives or sends ``pdu_ref`` on another bus."""

    for ecu in ctx.model.ecus:
        if _ecu_is_lin_master_with_group_state(ecu, bus_name, pdu_ref, ctx):
            return True
    return False


def _ecu_is_lin_master_with_group_state(ecu, bus_name, pdu_ref, ctx: _ValidationContext) -> bool:
    """
    True if ``ecu`` masters LIN bus ``bus_name`` and knows ``pdu_ref`` on
    another binding — receiving it (learns the state elsewhere) or sending it
    (is a source of the state, e.g. a central gateway).
    """

    for controller in ecu.controllers:
        for iface in controller.lin_interfaces or []:
            if getattr(iface, "bus_ref", None) == bus_name and getattr(iface, "node_type", None) == "master":
                tx, rx = _paths(ctx, ecu.name)
                if pdu_ref in rx or pdu_ref in tx:
                    return True
    return False


def _check_bus_reachability(group_name, group, bus, sent_frame_ids_by_bus) -> None:
    """
    Bus branch of the reachability check: the group's NM PDU must have a frame
    binding on the bus and at least one attached ECU must send that frame.
    """

    if bus is None:  # defensive: cannot happen on a validated model
        return
    nm_frames = [frame for frame in bus.frames or [] if any(inst.pdu_ref == group.nm_pdu for inst in frame.packed_pdus or [])]
    if not nm_frames:
        raise err_major(
            "state management group '{group}': bus participant '{bus}' has no frame carrying NM PDU '{pdu}' — "
            "a bus nobody feeds NM into can never be released correctly",
            group=group_name,
            bus=bus.name,
            pdu=group.nm_pdu,
            category=Category.REFERENCE,
            error_number="198",
        )
    nm_frame_ids = {frame.can_id if hasattr(frame, "can_id") else frame.lin_id for frame in nm_frames}
    if not (nm_frame_ids & sent_frame_ids_by_bus.get(bus.name, set())):
        raise err_major(
            "state management group '{group}': no ECU attached to bus participant '{bus}' sends NM PDU '{pdu}' — "
            "a bus nobody feeds NM into can never be released correctly",
            group=group_name,
            bus=bus.name,
            pdu=group.nm_pdu,
            category=Category.REFERENCE,
            error_number="199",
        )


def _check_relevance_bits(name, group, participants, nm_pdu) -> None:
    """
    Every participant's relevance bit must name a flag of the group's NM PDU.

    The relevance vector is an ordinary bitmask-encoded signal on the NM PDU,
    and each flag label names one vehicle function. The signal model carries
    no tag marking WHICH signal is the relevance vector, and more than one
    signal on an NM PDU may use a bitmask encoding (a control vector sits
    beside the relevance vector). Rather than guess by signal name, the
    valid-bit set is the union of every bitmask-flag label across all of the
    PDU's signals: this reliably rejects a mistyped bit — the purpose of the
    check — while never rejecting a bit that any real flag on the PDU spells.

    When the NM PDU models no bitmask flags at all, the relevance vector is
    simply not described, so there is nothing to check against and the rule
    stays silent (bit encoding is optional, like every other value encoding).
    """

    valid_bits = _bitmask_flag_labels(nm_pdu)
    if not valid_bits:
        return
    for member in participants:
        if member.relevance_bit not in valid_bits:
            raise err_major(
                "state management group '{group}': member '{member}' claims relevance bit '{bit}' "
                "which is not a flag of NM PDU '{pdu}' — declare the bit in the PDU's relevance "
                "vector or correct the relevance_bits entry",
                group=name,
                member=member.entity_path,
                bit=member.relevance_bit,
                pdu=group.nm_pdu,
                category=Category.VALUE_RANGE,
                error_number="200",
            )


def _bitmask_flag_labels(nm_pdu) -> Set[str]:
    """Return every bitmask-flag label across the PDU's signals — its relevance-/control-vector bits."""

    labels: Set[str] = set()
    for instance in getattr(nm_pdu, "signals", None) or []:
        encoding = instance.signal.value_encoding
        if isinstance(encoding, BitmaskFlags):
            labels.update(flag.label for flag in encoding.flags)
    return labels


# Deliberate lower-bound approximation of a CAN frame's on-wire bit count: a
# fixed protocol overhead plus the payload. It intentionally ignores bit-stuffing
# and inter-frame spacing — the check below is a floor plausibility test (does the
# frame fit in one cycle at all?), not exact bus scheduling. A frame that fails
# even this optimistic estimate can never meet the configured cadence.
_CAN_FRAME_OVERHEAD_BITS = 47  # SOF + arbitration + control + CRC + ACK + EOF (standard 11-bit id)


def _check_timing_feasibility(model, group) -> None:
    """
    Warn (never error) when the NM timing on a CAN bus is implausible — ONE check
    per NM frame, from two angles:

    (a) the bus physically achieves the frame's cyclic cadence — the frame's
        configured ``timing.cyclic_timings`` when present, the group's
        ``cycle_time_ms`` otherwise — where the transmission time is
        approximated from the bus baud rate and the frame payload length (see
        ``_CAN_FRAME_OVERHEAD_BITS`` — a deliberate lower bound that ignores
        bit-stuffing and inter-frame spacing);
    (b) a configured frame cyclic timing agrees with the group's referenced
        timing profile — both describe the same cadence, so a mismatch means
        the configuration contradicts itself.

    The check applies to CAN buses only — the buses that actually
    carry an NM frame. LIN is out of scope by design (it carries no NM frame;
    the bus participates via its master-as-proxy) and so is Ethernet (no
    single bus baud rate, and its bandwidth makes any realistic NM cycle
    trivially feasible). Anything that leaves timing undefined — an
    unresolvable timing profile (already reported by another check) or a bus
    without a baud rate — is skipped silently.
    """

    cycle_time_ms = _resolve_cycle_time_ms(model, group)
    if cycle_time_ms is None:
        return
    channels = getattr(model.communication, "channels", None) if model.communication else None
    if channels is None:
        return
    for bus in channels.can_buses or []:
        _warn_can_leg_timing(group, bus, cycle_time_ms)


def _resolve_cycle_time_ms(model, group):
    """Resolve the group's timing profile to its ``cycle_time_ms``, or ``None`` if unresolvable."""

    cfg = model.communication.state_management if model.communication else None
    profile = next(
        (p for p in (cfg.timing_profiles if cfg else None) or [] if getattr(p, "name", None) == group.timing_profile),
        None,
    )
    return getattr(profile, "cycle_time_ms", None)


def _warn_can_leg_timing(group, bus, cycle_time_ms) -> None:
    """
    One pass over the CAN ``bus``'s NM frames: check that a configured frame
    cyclic timing matches the group's profile, and that the frame's effective
    cadence (its own cyclic timing, or the group's ``cycle_time_ms`` when the
    frame states none) is physically achievable at the bus baud rate.
    """

    baud_rate = getattr(bus, "baud_rate", None)
    for frame in _nm_frames(bus, group.nm_pdu):
        frame_cycles_ms = _frame_cycles_ms(frame)
        _warn_cycle_mismatch(group, bus, frame, frame_cycles_ms, cycle_time_ms)
        _warn_infeasible_cadence(group, bus, frame, frame_cycles_ms, cycle_time_ms, baud_rate)


def _nm_frames(bus, nm_pdu):
    """Yield the ``bus``'s frames whose packed PDUs reference the NM PDU ``nm_pdu``."""

    for frame in bus.frames or []:
        if any(inst.pdu_ref == nm_pdu for inst in frame.packed_pdus or []):
            yield frame


def _frame_cycles_ms(frame):
    """Return the frame's configured cyclic timings in ms (``cyclic.cycle`` is in seconds)."""

    timing = getattr(frame, "timing", None)
    return [cyclic.cycle * 1000 for cyclic in getattr(timing, "cyclic_timings", None) or []]


def _warn_cycle_mismatch(group, bus, frame, frame_cycles_ms, cycle_time_ms) -> None:
    """Warn for every configured frame cyclic timing that contradicts the group's ``cycle_time_ms``."""

    for frame_cycle_ms in frame_cycles_ms:  # (b) consistency with the referenced timing profile
        if abs(frame_cycle_ms - cycle_time_ms) > 1e-6:
            warn(
                f"state management group '{group.name}': NM frame '{frame.name}' on bus '{bus.name}' is configured "
                f"with a cyclic timing of {frame_cycle_ms:g} ms, which does not match the group's cycle_time_ms "
                f"({cycle_time_ms})",
                category=Category.CONSISTENCY,
                error_number="201",
            )


def _warn_infeasible_cadence(group, bus, frame, frame_cycles_ms, cycle_time_ms, baud_rate) -> None:
    """Warn when the frame's effective cadence undercuts its transmission time at the bus ``baud_rate``."""

    if not baud_rate:
        return
    frame_tx_ms = _can_frame_bits(frame.length) / baud_rate * 1000  # (a) physical feasibility of the cadence
    cadence_ms = min(frame_cycles_ms) if frame_cycles_ms else cycle_time_ms
    source = "configured cyclic timing" if frame_cycles_ms else "cycle_time_ms"
    if cadence_ms < frame_tx_ms:
        warn(
            f"state management group '{group.name}': {source} ({cadence_ms:g}) is shorter than the "
            f"~{frame_tx_ms:.2f} ms needed to send NM frame '{frame.name}' on bus '{bus.name}' at {baud_rate} bit/s",
            category=Category.CONSISTENCY,
            error_number="202",
        )


def _can_frame_bits(payload_bytes: int) -> int:
    """CAN frame size in bits (floor): fixed overhead + 8 bits per payload byte."""

    return _CAN_FRAME_OVERHEAD_BITS + 8 * payload_bytes
