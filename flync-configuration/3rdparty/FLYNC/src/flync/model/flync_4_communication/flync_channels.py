"""Channel-level configuration for CAN, LIN, Ethernet, and PDU definitions."""

from typing import Annotated, Iterable, List, Mapping, Optional

from pydantic import Field, model_validator

from flync.core.annotations.external import (
    External,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.bit_ranges import (
    BitRange,
    check_bit_ranges_no_overlap,
    check_bit_ranges_within,
)
from flync.core.validators.generic import validate_list_items_unique
from flync.model.flync_4_bus.can_bus import CANBus
from flync.model.flync_4_bus.lin_bus import LINBus
from flync.model.flync_4_signal.frame import Frame
from flync.model.flync_4_signal.pdu import (
    PDU,
    ContainerPDU,
    MultiplexedPDU,
    PDUInstance,
    StandardPDU,
)


class FLYNCChannelConfig(FLYNCBaseModel):
    """
    Channel-level configuration grouping all buses and shared PDU definitions.

    Parameters
    ----------
    pdus : list of :class:`~flync.model.flync_4_signal.StandardPDU` | :class:`~flync.model.flync_4_signal.MultiplexedPDU`, optional
        Shared PDU definitions that may be referenced from any channel.
    can_buses : list of :class:`~flync.model.flync_4_bus.CANBus`, optional
        CAN and CAN FD bus configurations.
    lin_buses : list of :class:`~flync.model.flync_4_bus.LINBus`, optional
        LIN bus configurations.
    ethernet_pdu_containers : list of :class:`~flync.model.flync_4_signal.ContainerPDU`, optional
        Ethernet Container PDU definitions.
    """

    pdus: Annotated[
        Optional[
            List[
                Annotated[
                    StandardPDU | MultiplexedPDU,
                    Field(discriminator="type"),
                ]
            ]
        ],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="pdus",
        ),
    ] = Field(
        default_factory=list,
        description="Shared PDU definitions, one file per PDU.",
    )
    can_buses: Annotated[
        Optional[List[CANBus]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="can",
        ),
    ] = Field(
        default=None,
        description="CAN / CAN FD bus definitions, one file per bus.",
    )
    lin_buses: Annotated[
        Optional[List[LINBus]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="lin",
        ),
    ] = Field(
        default=None,
        description="LIN bus definitions, one file per bus.",
    )
    ethernet_pdu_containers: Annotated[
        Optional[List[ContainerPDU]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIXED_PATH,
            path="ethernet_pdu_containers",
        ),
    ] = Field(
        default=None,
        description="Ethernet Container PDU definitions.",
    )

    @model_validator(mode="after")
    def validate_pdus_name_unique(self):
        validate_list_items_unique([p.name for p in (self.pdus or [])], "PDUs")
        return self

    @model_validator(mode="after")
    def validate_canbus_name_unique(self):
        validate_list_items_unique([can.name for can in (self.can_buses or [])], "CANBus")
        return self

    @model_validator(mode="after")
    def validate_linbus_name_unique(self):
        validate_list_items_unique([lin.name for lin in (self.lin_buses or [])], "LINBus")
        return self

    def _pdu_registry(self) -> Mapping[str, PDU]:
        """Return a ``name -> PDU`` lookup over the shared PDU definitions."""
        return {p.name: p for p in (self.pdus or [])}

    @model_validator(mode="after")
    def validate_pdu_refs(self) -> "FLYNCChannelConfig":
        """Verify packed PDUs in CAN/LIN frames reference known PDUs and fit without overlap."""
        pdu_registry = self._pdu_registry()
        _validate_multiplexed_pdu_placements(self.pdus or [], pdu_registry)
        buses_by_kind = (
            ("CANBus", self.can_buses or []),
            ("LINBus", self.lin_buses or []),
        )
        for kind, buses in buses_by_kind:
            for bus in buses:
                unknown_refs = _collect_unknown_pdu_refs(bus.frames, pdu_registry)
                if unknown_refs:
                    raise err_major(
                        "{kind} '{name}' references unknown PDU(s): {unknown_refs}",
                        kind=kind,
                        name=bus.name,
                        unknown_refs=sorted(unknown_refs),
                        category=Category.REFERENCE,
                        error_number="057",
                    )
                _validate_frame_pdu_placements(kind, bus, pdu_registry)
        return self

    @model_validator(mode="after")
    def validate_ethernet_pdu_container_refs(self) -> "FLYNCChannelConfig":
        """Verify contained PDUs in ethernet_pdu_containers reference known PDUs."""
        pdu_registry = self._pdu_registry()
        for container in self.ethernet_pdu_containers or []:
            unknown_refs = _collect_unknown_contained_pdu_refs(container, pdu_registry)
            if unknown_refs:
                raise err_major(
                    "ContainerPDU '{name}' references unknown PDU(s): {unknown_refs}",
                    name=container.name,
                    unknown_refs=sorted(unknown_refs),
                    category=Category.REFERENCE,
                    error_number="236",
                )
        return self

    @model_validator(mode="after")
    def validate_multiplexed_pdu_refs(self) -> "FLYNCChannelConfig":
        """Verify MultiplexedPDU static/mux group PDU instances reference known PDUs."""
        pdu_registry = self._pdu_registry()
        for pdu in self.pdus or []:
            if not isinstance(pdu, MultiplexedPDU):
                continue
            unknown_refs = _collect_unknown_muxed_pdu_refs(pdu, pdu_registry)
            if unknown_refs:
                raise err_major(
                    "MultiplexedPDU '{name}' references unknown PDU(s): {unknown_refs}",
                    name=pdu.name,
                    unknown_refs=sorted(unknown_refs),
                    category=Category.REFERENCE,
                    error_number="237",
                )
        return self


def _collect_unknown_pdu_refs(frames: Iterable[Frame], pdu_registry: Mapping[str, PDU]) -> "set[str]":
    """Return pdu_ref names in ``frames`` not present in the PDU registry."""
    unknown: set[str] = set()
    for frame in frames:
        for pdu_inst in frame.packed_pdus:
            if pdu_inst.pdu_ref not in pdu_registry:
                unknown.add(pdu_inst.pdu_ref)
    return unknown


def _collect_unknown_contained_pdu_refs(container: ContainerPDU, pdu_registry: Mapping[str, PDU]) -> "set[str]":
    """Return pdu_ref names in ``container.contained_pdus`` not present in the PDU registry."""
    return {contained.pdu_ref for contained in container.contained_pdus if contained.pdu_ref not in pdu_registry}


def _collect_unknown_muxed_pdu_refs(pdu: MultiplexedPDU, pdu_registry: Mapping[str, PDU]) -> "set[str]":
    """Return pdu_ref names in ``pdu``'s static_group/mux_groups not present in the PDU registry."""
    unknown: set[str] = set()
    for static in pdu.static_group or []:
        if static.pdu_ref not in pdu_registry:
            unknown.add(static.pdu_ref)
    for group in pdu.mux_groups:
        if group.pdu.pdu_ref not in pdu_registry:
            unknown.add(group.pdu.pdu_ref)
    return unknown


def _resolve_signal_ranges(inst: PDUInstance, label: str, pdu_registry: Mapping[str, PDU]) -> List[BitRange]:
    """Return the actual per-signal bit ranges a :class:`PDUInstance` occupies.

    Resolves each placed signal inside the referenced PDU to its ``[bit_position + signal_offset,
    bit_position + signal_offset + signal_length)`` range instead of the whole PDU's bounding box. This
    lets multiplexed payloads sit in the "gap" between sparse static signals without a false overlap,
    while still flagging any signal that genuinely intersects another.
    """
    if inst.bit_position is None or (ref_pdu := pdu_registry.get(inst.pdu_ref)) is None:
        return []
    signals = getattr(ref_pdu, "signals", None)
    if not signals:
        # Nothing is known to be empty inside a signal-less PDU, so keep the
        # full footprint (mirrors the old bounding-box behaviour).
        return [(f"{label} '{inst.pdu_ref}'", inst.bit_position, inst.bit_position + ref_pdu.length * 8)]
    ranges: List[BitRange] = []
    for si in signals:
        if getattr(si, "bit_position", None) is None:
            continue
        start = inst.bit_position + si.bit_position
        ranges.append((f"{label} '{si.signal.name}'", start, start + si.signal.bit_length))
    return ranges


def _validate_multiplexed_pdu_placements(pdus: Iterable[PDU], pdu_registry: Mapping[str, PDU]) -> None:
    """Validate the PDU placements of every :class:`MultiplexedPDU` in ``pdus``.

    Every placement (selector signal, ``static_group``, ``mux_groups``) must fit within the
    multiplexed PDU's own length. Static placements must not overlap each other nor the selector.
    Each mux group is checked against the selector and the static placements only: mux groups are
    mutually exclusive alternatives selected at runtime, so two groups with different
    ``selector_value`` are expected to share the same bits.
    """
    for pdu in pdus:
        if not isinstance(pdu, MultiplexedPDU):
            continue
        context = f"MultiplexedPDU '{pdu.name}'"
        sel = pdu.selector_signal
        static_placements = list(pdu.static_group or [])
        sel_ranges: List[BitRange] = []
        if sel.bit_position is not None:
            sel_ranges.append((sel.signal.name, sel.bit_position, sel.bit_position + sel.signal.bit_length))
        # Overlap and within checks use the referenced PDUs' actual per-signal ranges
        # rather than their full (byte-aligned) bounding boxes, so sparse static
        # signals sitting in the "gap" between the selector and/or a muxed payload
        # don't produce false hits.
        static_signals = [r for inst in static_placements for r in _resolve_signal_ranges(inst, "static_group", pdu_registry)]
        group_signals = [
            r for group in pdu.mux_groups for r in _resolve_signal_ranges(group.pdu, f"mux_group(selector={group.selector_value})", pdu_registry)
        ]
        check_bit_ranges_within(context, [*sel_ranges, *static_signals, *group_signals], pdu.length * 8)
        check_bit_ranges_no_overlap(context, [*sel_ranges, *static_signals])
        for group in pdu.mux_groups:
            group_ranges = _resolve_signal_ranges(group.pdu, f"mux_group(selector={group.selector_value})", pdu_registry)
            check_bit_ranges_no_overlap(context, [*sel_ranges, *static_signals, *group_ranges])


def _validate_frame_pdu_placements(kind: str, bus, pdu_registry: Mapping[str, PDU]) -> None:
    """Validate that PDU instances placed in each frame on ``bus`` fit without overlap.

    The referenced PDU's length (resolved from ``pdu_registry``) is used to
    compute each placement's bit range so true overlap can be detected, not
    just bit_position collisions.
    """

    for frame in bus.frames:
        ranges: List[BitRange] = []
        for pdu_inst in frame.packed_pdus:
            if pdu_inst.bit_position is None:
                continue
            pdu = pdu_registry.get(pdu_inst.pdu_ref)
            if pdu is None:
                continue
            ranges.append(
                (
                    pdu_inst.pdu_ref,
                    pdu_inst.bit_position,
                    pdu_inst.bit_position + pdu.length * 8,
                )
            )
        context = f"{kind} '{bus.name}' frame '{frame.name}'"
        check_bit_ranges_within(context, ranges, frame.length * 8)
        check_bit_ranges_no_overlap(context, ranges)
