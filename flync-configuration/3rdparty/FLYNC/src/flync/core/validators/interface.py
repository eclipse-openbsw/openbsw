"""Workspace-level validators for bus interface frame references (CAN / CAN FD and LIN).

Complements :mod:`flync.core.validators.forwarder`, which resolves *forwarder* references only.
The passes here cover the plain ``sender_frames`` / ``receiver_frames`` declarations of the bus
interfaces, plus the interface's own ``bus_ref``, so that a dangling id or a CAN interface pointing at
a LIN bus (or vice versa) is reported instead of silently ignored.
"""

from __future__ import annotations

from typing import TYPE_CHECKING, Dict, Iterator, Tuple, Union

from pydantic_core import PydanticCustomError

from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_ecu.can_interface import CANFrameRef, CANInterface
from flync.model.flync_4_ecu.lin_interface import LINFrameRef, LINMasterInterface, LINSlaveInterface
from flync.model.flync_4_signal.frame import CANFDFrame, CANFrame, LINFrame

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.controller import Controller
    from flync.model.flync_model import FLYNCModel


CANAnyFrame = Union[CANFrame, CANFDFrame]
AnyFrame = Union[CANFrame, CANFDFrame, LINFrame]
AnyFrameRef = Union[CANFrameRef, LINFrameRef]
AnyBusInterface = Union[CANInterface, LINMasterInterface, LINSlaveInterface]


# ---------------------------------------------------------------------------
# Error source attribution
#
# These checks run as a FLYNCModel-level ``model_validator``, so their errors attach to the
# workspace-root document and the validate CLI would show an empty Source column. We stamp each
# finding with a structural ``yaml_path`` locator, exactly as the forwarder passes do.
# ---------------------------------------------------------------------------


def _with_source(err: PydanticCustomError, locator: str) -> PydanticCustomError:
    """Return *err* re-created with ``yaml_path=locator`` added to its context."""

    ctx = dict(err.context or {})
    if "yaml_path" in ctx:
        return err
    ctx["yaml_path"] = locator
    # err.type / err.message_template are typed as ``str`` but were originally constructed
    # from ``LiteralString``, so re-forwarding them through PydanticCustomError is safe.
    return PydanticCustomError(err.type, err.message_template, ctx)  # type: ignore[arg-type]


def _interface_locator(controller: "Controller", iface: AnyBusInterface, kind: str) -> str:
    """Path-style Source locator for a bus interface (bracket-free: the CLI renders this through Rich)."""

    return f"controllers/{controller.name}/{kind}_interfaces/{iface.name}"


# ---------------------------------------------------------------------------
# Catalogs
# ---------------------------------------------------------------------------


def _channels(model: "FLYNCModel"):
    """Return ``communication.channels`` or ``None`` when the model declares no communication."""

    return getattr(model.communication, "channels", None) if model.communication else None


def _build_can_frames_by_bus(model: "FLYNCModel") -> Dict[str, Dict[int, CANAnyFrame]]:
    """Return ``{bus_name: {can_id: frame}}`` for every CAN / CAN FD bus under ``communication.channels``."""

    out: Dict[str, Dict[int, CANAnyFrame]] = {}
    channels = _channels(model)
    if channels is None or channels.can_buses is None:
        return out
    for bus in channels.can_buses:
        out[bus.name] = {frame.can_id: frame for frame in bus.frames or []}
    return out


def _build_lin_frames_by_bus(model: "FLYNCModel") -> Dict[str, Dict[int, LINFrame]]:
    """Return ``{bus_name: {lin_id: frame}}`` for every LIN bus under ``communication.channels``."""

    out: Dict[str, Dict[int, LINFrame]] = {}
    channels = _channels(model)
    if channels is None or channels.lin_buses is None:
        return out
    for bus in channels.lin_buses:
        out[bus.name] = {frame.lin_id: frame for frame in bus.frames or []}
    return out


# ---------------------------------------------------------------------------
# Tree walking
# ---------------------------------------------------------------------------


def _iter_bus_interfaces(model: "FLYNCModel") -> Iterator[Tuple["Controller", AnyBusInterface, str]]:
    """Yield ``(controller, interface, kind)`` for every CAN and LIN interface in the model."""

    for ecu in model.ecus or []:
        for controller in ecu.controllers or []:
            for can_iface in controller.can_interfaces or []:
                yield controller, can_iface, "can"
            for lin_iface in controller.lin_interfaces or []:
                yield controller, lin_iface, "lin"


def _iter_frame_refs(iface: AnyBusInterface) -> Iterator[Tuple[str, AnyFrameRef]]:
    """Yield ``(field_name, frame_ref)`` for the sender / receiver declarations present on *iface*.

    The LIN interfaces are asymmetric on purpose: a master only sends, a slave only receives.
    """

    for field_name in ("sender_frames", "receiver_frames"):
        for ref in getattr(iface, field_name, None) or []:
            yield field_name, ref


# ---------------------------------------------------------------------------
# Validation pass
# ---------------------------------------------------------------------------


def _bus_kind_label(kind: str) -> str:
    """Return the catalog wording used in the error messages for *kind*."""

    return "communication.channels.can_buses" if kind == "can" else "communication.channels.lin_buses"


def _validate_interface(iface: AnyBusInterface, kind: str, frames_by_bus: Dict[str, Dict[int, AnyFrame]]) -> None:
    """Resolve one interface's own ``bus_ref`` plus every sender / receiver frame reference it declares.

    *frames_by_bus* is the catalog of the interface's own bus kind, so an unresolvable name is reported against
    that kind - see :func:`validate_interface_frame_refs` for why the catalog is picked that way.
    """

    owner = f"{type(iface).__name__}(name={iface.name})"
    catalog = _bus_kind_label(kind)

    if iface.bus_ref not in frames_by_bus:
        raise err_major(
            "{owner}: bus_ref '{bus}' does not name any bus declared under {catalog}.",
            owner=owner,
            bus=iface.bus_ref,
            catalog=catalog,
            category=Category.REFERENCE,
            error_number="215",
        )

    for field_name, ref in _iter_frame_refs(iface):
        if ref.bus_ref not in frames_by_bus:
            raise err_major(
                "{owner}: {field}: bus_ref '{bus}' does not name any bus declared under {catalog}.",
                owner=owner,
                field=field_name,
                bus=ref.bus_ref,
                catalog=catalog,
                category=Category.REFERENCE,
                error_number="216",
            )
        if ref.frame_ref not in frames_by_bus[ref.bus_ref]:
            raise err_major(
                "{owner}: {field}: frame_ref id={ref} does not name any frame declared on bus '{bus}' under {catalog}.",
                owner=owner,
                field=field_name,
                ref=ref.frame_ref,
                bus=ref.bus_ref,
                catalog=catalog,
                category=Category.REFERENCE,
                error_number="217",
            )


def validate_interface_frame_refs(model: "FLYNCModel") -> None:
    """Workspace pass: every CAN / LIN interface must name a declared bus of its own kind and resolve its frame refs.

    The check is a plain lookup - is this frame id declared on this bus - performed against the catalog of the
    interface's own bus kind. Both aspects are load-bearing:

    *Scoped by bus*, because a CAN id is only unique within a bus: id ``0x100`` on ``CAN1`` and on ``CAN2`` are
    two different frames, which is why :class:`~flync.model.flync_4_ecu.can_interface.CANFrameRef` carries a
    ``bus_ref`` next to the ``frame_ref``. Asking only whether an id exists *somewhere* would accept an interface
    on ``CAN1`` declaring a frame that only exists on ``CAN2``.

    *Scoped by bus kind*, because ``bus_ref`` is a plain string and ``frame_ref`` a plain int - the type system
    stops a ``LINFrameRef`` from landing in a ``CANInterface``, but nothing stops a CAN interface from naming a
    LIN bus. Resolving a CAN interface against ``can_buses`` only is what catches that, so no separate cross-kind
    rule is needed: the LIN bus name simply is not in the CAN catalog. A single catalog merged over both
    kinds would instead accept LIN ids on a CAN interface, and would be ambiguous anyway - bus names are unique
    per kind but not across kinds, so a workspace may hold a CAN bus and a LIN bus of the same name.

    A bus kind whose catalog is empty is skipped: FLYNC supports partial models (an ECU or controller may be
    modelled without the bus catalog it will later be wired into), and there is nothing to resolve against.
    Once a workspace declares buses of a kind, every interface of that kind must resolve.
    """

    can_frames_by_bus = _build_can_frames_by_bus(model)
    lin_frames_by_bus = _build_lin_frames_by_bus(model)

    for controller, iface, kind in _iter_bus_interfaces(model):
        frames_by_bus: Dict[str, Dict[int, AnyFrame]] = can_frames_by_bus if kind == "can" else lin_frames_by_bus  # type: ignore[assignment]
        if not frames_by_bus:
            continue
        try:
            _validate_interface(iface, kind, frames_by_bus)
        except PydanticCustomError as err:
            raise _with_source(err, _interface_locator(controller, iface, kind)) from None
