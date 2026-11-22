"""
Runtime-derived system-wide CAN and LIN bus topology.

Unlike :class:`~flync.model.flync_4_topology.ethernet_topology.EthernetTopology`, CAN and LIN buses have no
authored topology file: connectivity is declared implicitly via the ``bus_ref`` on each ECU controller's CAN/LIN
interfaces. This module derives a system-wide view of that connectivity (which ECUs/controllers/interfaces attach
to which bus) and validates it. The result is never authored in YAML; it is (re)computed by
:meth:`~flync.model.flync_model.FLYNCModel.build_and_validate_bus_topologies` on every model load.
"""

from typing import TYPE_CHECKING, Dict, List, Literal, Optional, Tuple

from pydantic import Field, PrivateAttr

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major, warn
from flync.model.flync_4_bus.can_bus import CANBus
from flync.model.flync_4_bus.lin_bus import LINBus
from flync.model.flync_4_ecu.lin_interface import LINMasterInterface

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.can_interface import CANInterface
    from flync.model.flync_4_ecu.ecu import ECU
    from flync.model.flync_4_ecu.lin_interface import LINSlaveInterface
    from flync.model.flync_model import FLYNCModel


class BusAttachmentPoint(FLYNCBaseModel):
    """
    A single ECU controller interface attached to a CAN or LIN bus.

    Parameters
    ----------
    ecu_name : str
        Name of the ECU that owns the attached interface.
    controller_name : str
        Name of the controller that owns the attached interface.
    interface_name : str
        Name of the CAN, LIN or Ethernet multidrop interface attached to the bus.
    role : Literal["can_node", "lin_master", "lin_slave"]
        Role the interface plays on the bus.
    """

    ecu_name: str = Field()
    controller_name: str = Field()
    interface_name: str = Field()
    role: Literal["can_node", "lin_master", "lin_slave"] = Field()
    _interface: "Optional[CANInterface | LINMasterInterface | LINSlaveInterface]" = PrivateAttr(default=None)
    _ecu: "Optional[ECU]" = PrivateAttr(default=None)


class BusTopology(FLYNCBaseModel):
    """
    Runtime-derived attachment topology of a single CAN or LIN bus.

    Parameters
    ----------
    bus_name : str
        Name of the bus or segment, matching ``bus_ref`` on the attached interfaces.
    bus_type : Literal["can", "lin"]
        Kind of bus.
    attachments : list of :class:`BusAttachmentPoint`
        ECU interfaces attached to this bus.
    """

    bus_name: str = Field()
    bus_type: Literal["can", "lin"] = Field()
    attachments: List[BusAttachmentPoint] = Field(default_factory=list)


class CANBusTopology(BusTopology):
    """Runtime-derived attachment topology of a single CAN bus."""

    bus_type: Literal["can"] = Field(default="can")
    _bus: Optional[CANBus] = None


class LINBusTopology(BusTopology):
    """Runtime-derived attachment topology of a single LIN bus."""

    bus_type: Literal["lin"] = Field(default="lin")
    _bus: Optional[LINBus] = None

    @property
    def master(self) -> Optional[BusAttachmentPoint]:
        """The single LIN master attachment on this bus, if any."""
        return next((a for a in self.attachments if a.role == "lin_master"), None)

    @property
    def slaves(self) -> List[BusAttachmentPoint]:
        """All LIN slave attachments on this bus."""
        return [a for a in self.attachments if a.role == "lin_slave"]


def build_bus_topologies(
    flync_model: "FLYNCModel",
) -> Tuple[List[CANBusTopology], List[LINBusTopology], Optional[dict], Optional[dict]]:
    """
    Derive the system-wide CAN and LIN bus topology from bus definitions and ECU controller interfaces.

    Groups every CAN and LIN interface across all ECUs by ``bus_ref``, and seeds a zero-attachment entry for every
    bus declared under ``flync_model.communication.channels`` so unused buses can still be reported.

    Returns
    -------
    tuple
        ``(can_topologies, lin_topologies, can_defs, lin_defs)``. ``can_defs``/``lin_defs`` are ``{bus_name: bus}``
        registries, or ``None`` when no bus definitions could be determined (e.g. no ``communication.channels``);
        pass them straight through to :func:`validate_bus_topologies`.
    """

    can_defs = _bus_registry(flync_model, "can_buses")
    lin_defs = _bus_registry(flync_model, "lin_buses")

    can_by_name: Dict[str, CANBusTopology] = {}
    lin_by_name: Dict[str, LINBusTopology] = {}

    for ecu in flync_model.ecus:
        _collect_ecu_bus_attachments(ecu, can_by_name, lin_by_name)

    _seed_defined_buses(can_by_name, can_defs, CANBusTopology)
    _seed_defined_buses(lin_by_name, lin_defs, LINBusTopology)
    _link_bus_definitions(can_by_name, can_defs)
    _link_bus_definitions(lin_by_name, lin_defs)

    return list(can_by_name.values()), list(lin_by_name.values()), can_defs, lin_defs


def _collect_ecu_bus_attachments(
    ecu: "ECU",
    can_by_name: Dict[str, CANBusTopology],
    lin_by_name: Dict[str, LINBusTopology],
) -> None:
    """Scan a single ECU's controllers and attach their CAN/LIN interfaces to the topology maps."""
    for controller in ecu.controllers:
        for can_iface in controller.can_interfaces or []:
            can_topo = can_by_name.setdefault(can_iface.bus_ref, CANBusTopology(bus_name=can_iface.bus_ref))
            _attach(can_topo, ecu, controller.name, can_iface, "can_node")
        for lin_iface in controller.lin_interfaces or []:
            role: Literal["lin_master", "lin_slave"] = "lin_master" if isinstance(lin_iface, LINMasterInterface) else "lin_slave"
            lin_topo = lin_by_name.setdefault(lin_iface.bus_ref, LINBusTopology(bus_name=lin_iface.bus_ref))
            _attach(lin_topo, ecu, controller.name, lin_iface, role)


def _seed_defined_buses(by_name: dict, defs: Optional[dict], topo_cls: type) -> None:
    """Ensure every declared bus has an entry in *by_name*, even if no ECU attaches to it."""
    if defs is not None:
        for name in defs:
            by_name.setdefault(name, topo_cls(bus_name=name))


def _link_bus_definitions(by_name: dict, defs: Optional[dict]) -> None:
    """Set each topology entry's ``_bus`` back-reference to the matching bus definition."""
    for name, entry in by_name.items():
        entry._bus = defs.get(name) if defs is not None else None


def _bus_registry(flync_model: "FLYNCModel", attr: str) -> Optional[dict]:
    """Return ``{bus_name: bus}`` for ``attr`` (a bus list on ``communication.channels``), or ``None`` if it cannot be determined."""

    channels = flync_model.communication.channels if flync_model.communication else None
    buses = getattr(channels, attr, None) if channels else None
    return {b.name: b for b in buses} if buses is not None else None


def _attach(topo: BusTopology, ecu: "ECU", controller_name: str, iface, role: "Literal['can_node', 'lin_master', 'lin_slave']") -> None:
    """
    Create a :class:`BusAttachmentPoint` for a single ECU controller interface and append it to *topo*.

    Parameters
    ----------
    topo : BusTopology
        The bus topology entry to which the attachment is added.
    ecu : ECU
        The ECU that owns the interface being attached.
    controller_name : str
        Name of the controller within *ecu* that owns the interface.
    iface : CANInterface | LINMasterInterface | LINSlaveInterface
        The CAN or LIN interface being attached to the bus.
    role : Literal["can_node", "lin_master", "lin_slave"]
        The role the interface plays on the bus.
    """
    attachment = BusAttachmentPoint(
        ecu_name=ecu.name,
        controller_name=controller_name,
        interface_name=iface.name,
        role=role,
    )
    attachment._interface = iface
    attachment._ecu = ecu
    topo.attachments.append(attachment)


def validate_bus_topologies(
    can_topos: List[CANBusTopology],
    lin_topos: List[LINBusTopology],
    can_defs: Optional[dict],
    lin_defs: Optional[dict],
) -> None:
    """Run the system-wide CAN/LIN bus consistency checks: unknown ``bus_ref``, LIN master cardinality, LIN schedule table
    presence, unused/singly-attached buses."""

    for can_topo in can_topos:
        _validate_bus_ref_known(can_topo, can_defs)
        _validate_attachment_count(can_topo, "CAN", can_defs)
    for lin_topo in lin_topos:
        _validate_bus_ref_known(lin_topo, lin_defs)
        _validate_lin_masters(lin_topo)
        _validate_lin_schedule_tables(lin_topo)
        _validate_attachment_count(lin_topo, "LIN", lin_defs)


def _validate_bus_ref_known(topo: BusTopology, defs: Optional[dict], kind: str = "CAN/LIN") -> None:
    """
    Validate that a bus topology's ``bus_ref`` corresponds to a known bus definition.

    Emits a warning when no bus definitions are loaded (so the reference cannot be verified),
    or raises ``err_major`` when definitions are available but the referenced bus name is not among them.
    Silently returns when the topology has no attachments (nothing to validate).

    Parameters
    ----------
    topo : BusTopology
        The bus topology entry whose ``bus_name`` is checked.
    defs : dict or None
        ``{bus_name: bus}`` registry of declared buses, or ``None`` if unavailable.
    kind : str
        Human-readable label naming the kind of bus in the message, e.g. ``"CAN/LIN"`` or ``"10BASE-T1S"``.

    Raises
    ------
    err_major
        If *defs* is available and ``topo.bus_name`` is not found in it (error 221).
    """
    if not topo.attachments:
        return
    if defs is None:
        warn(
            f"bus_ref '{topo.bus_name}' referenced by ECU interface(s) cannot be verified: no {kind} bus definitions are loaded.",
            category=Category.REFERENCE,
            error_number="222",
        )
    elif topo.bus_name not in defs:
        raise err_major(
            f"{kind} interface(s) reference unknown bus '{topo.bus_name}'. Defined buses: {sorted(defs)}",
            category=Category.REFERENCE,
            error_number="221",
        )


def _validate_lin_masters(topo: LINBusTopology) -> None:
    """
    Validate LIN master cardinality on a single LIN bus.

    Raises ``err_major`` if more than one master interface is attached, and emits a warning
    if slave interfaces exist but no master is present.

    Parameters
    ----------
    topo : LINBusTopology
        The LIN bus topology entry to validate.

    Raises
    ------
    err_major
        If the bus has more than one master interface (error 223).
    """
    masters = [a for a in topo.attachments if a.role == "lin_master"]
    if len(masters) > 1:
        names = [f"{m.ecu_name}/{m.controller_name}/{m.interface_name}" for m in masters]
        raise err_major(
            f"LIN bus '{topo.bus_name}' has {len(masters)} master interfaces ({names}); exactly one is required.",
            category=Category.CONSISTENCY,
            error_number="223",
        )
    if not masters and topo.slaves:
        warn(
            f"LIN bus '{topo.bus_name}' has slave interface(s) but no master interface.",
            category=Category.CONSISTENCY,
            error_number="224",
        )


def _validate_lin_schedule_tables(topo: LINBusTopology) -> None:
    """
    Validate that a LIN bus declares a schedule table if and only if it has a master interface.

    Scheduling is the master's responsibility: a master needs at least one schedule table to schedule frames on,
    and a bus with slave(s) but no master has nothing to run a schedule table against. Skipped when the bus
    definition could not be resolved (unknown/unverifiable ``bus_ref``, already reported by
    :func:`_validate_bus_ref_known`), and when the bus has no attachments at all -- an authored-but-unattached bus
    is merely unused (already reported by :func:`_validate_attachment_count`), not a master/slave violation.

    Parameters
    ----------
    topo : LINBusTopology
        The LIN bus topology entry to validate.

    Raises
    ------
    err_major
        If the bus has a master but no schedule table (error 315), or a schedule table but slave(s) and no master
        (error 316).
    """
    if topo._bus is None or not topo.attachments:
        return
    has_master = topo.master is not None
    has_schedule_tables = bool(topo._bus.schedule_tables)
    if has_master and not has_schedule_tables:
        raise err_major(
            f"LIN bus '{topo.bus_name}' has a master interface but declares no schedule table; the master needs "
            "at least one schedule table to schedule frames on.",
            category=Category.CONSISTENCY,
            error_number="315",
        )
    if has_schedule_tables and not has_master and topo.slaves:
        raise err_major(
            f"LIN bus '{topo.bus_name}' declares schedule table(s) but has no master interface attached; "
            "scheduling is handled by the LIN Master only.",
            category=Category.CONSISTENCY,
            error_number="316",
        )


def _validate_attachment_count(topo: BusTopology, kind: str, defs: Optional[dict]) -> None:
    """
    Warn when a declared bus has zero or only a single attached ECU interface.

    Skipped entirely when *defs* is ``None`` or the bus is not among the declared definitions
    (unknown-ref errors are handled by :func:`_validate_bus_ref_known`).

    Parameters
    ----------
    topo : BusTopology
        The bus topology entry to check.
    kind : str
        Human-readable bus kind label used in warning messages (e.g. ``"CAN"`` or ``"LIN"``).
    defs : dict or None
        ``{bus_name: bus}`` registry of declared buses, or ``None`` if unavailable.
    """
    if defs is None or topo.bus_name not in defs:
        return
    if not topo.attachments:
        warn(
            f"{kind} bus '{topo.bus_name}' is defined but no ECU interface attaches to it.",
            category=Category.CONSISTENCY,
            error_number="226",
        )
    elif len(topo.attachments) == 1:
        name = topo.attachments[0].interface_name
        warn(
            f"{kind} bus '{topo.bus_name}' has only a single attached node ({name}).",
            category=Category.CONSISTENCY,
            error_number="230",
        )
