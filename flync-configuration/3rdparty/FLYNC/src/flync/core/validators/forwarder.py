"""Workspace-level validators for PDU forwarders and PDU sender/receiver socket deployments (forwarder-based or standalone)."""

from __future__ import annotations

from typing import TYPE_CHECKING, Dict, List, Optional, Tuple, Union

from pydantic_core import PydanticCustomError

from flync.core.utils.exceptions import Category, err_major, err_minor
from flync.model.flync_4_ecu.can_interface import CANInterface
from flync.model.flync_4_signal.forwarder import (
    CANFrameEgress,
    CANFrameForwarder,
    EthSocketEgress,
    PDUForwarder,
)
from flync.model.flync_4_signal.frame import (
    CANFDFrame,
    CANFrame,
)
from flync.model.flync_4_signal.pdu import (
    PDU,
    ContainerPDU,
)

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.controller import Controller
    from flync.model.flync_4_ecu.sockets import Socket
    from flync.model.flync_4_signal.pdu_deployment import PDUReceiver, PDUSender
    from flync.model.flync_model import FLYNCModel


CANAnyFrame = Union[CANFrame, CANFDFrame]


# ---------------------------------------------------------------------------
# Error source attribution
#
# Forwarder checks run as a FLYNCModel-level ``model_validator``, so their errors
# attach to the workspace-root document and the validate CLI shows an empty Source
# column. These passes own no single ``.flync.yaml`` (they reason over the assembled
# model), so we stamp each finding with a structural ``yaml_path`` locator
# (controller / carrier / forwarder) that the CLI renders in the Source column.
# ---------------------------------------------------------------------------


def _with_source(err: PydanticCustomError, locator: str) -> PydanticCustomError:
    """Return *err* re-created with ``yaml_path=locator`` added to its context.

    Leaves an already-stamped error untouched so an inner, more specific locator wins.
    """

    ctx = dict(err.context or {})
    if "yaml_path" in ctx:
        return err
    ctx["yaml_path"] = locator
    # err.type / err.message_template are typed as ``str`` but were originally constructed
    # from ``LiteralString``, so re-forwarding them through PydanticCustomError is safe.
    return PydanticCustomError(err.type, err.message_template, ctx)  # type: ignore[arg-type]


def _pdu_forwarder_locator(controller: "Controller", socket: "Socket", fwd: PDUForwarder) -> str:
    """Path-style Source locator for a ``PDUForwarder`` deployment.

    Bracket-free on purpose: the validate CLI renders this through Rich, which treats
    ``[...]`` as console markup and would silently drop the bracketed segment.
    """

    return f"controllers/{controller.name}/sockets/{socket.name}/pdu_forwarder/{fwd.pdu_ref}"


def _pdu_deployment_locator(controller: "Controller", socket: "Socket", dep: "Union[PDUSender, PDUReceiver]") -> str:
    """Path-style Source locator for a ``PDUSender`` / ``PDUReceiver`` deployment (bracket-free, see above)."""

    return f"controllers/{controller.name}/sockets/{socket.name}/{dep.deployment_type}/{dep.pdu_ref}"


def _can_forwarder_locator(controller: "Controller", iface: CANInterface, fwd: CANFrameForwarder) -> str:
    """Path-style Source locator for a ``CANFrameForwarder`` deployment (``iface.bus_ref`` names the interface)."""

    return f"controllers/{controller.name}/can_interfaces/{iface.bus_ref}/forwarder_frames/{fwd.frame_ref}"


# ---------------------------------------------------------------------------
# Tree walking
# ---------------------------------------------------------------------------


def _iter_sockets_on_controller(controller: "Controller"):
    """Yield every :class:`Socket` owned by *controller* (across all eth interfaces / VLAN containers)."""

    for eth_iface in controller.ethernet_interfaces or []:
        for socket_container in eth_iface.sockets or []:
            for socket in socket_container.sockets or []:
                yield socket


def _iter_pdu_forwarders_on_socket(socket: "Socket"):
    """Yield every :class:`PDUForwarder` deployment carried by *socket*."""

    for dep_root in socket.deployments or []:
        dep = dep_root.root
        if isinstance(dep, PDUForwarder):
            yield dep


def _collect_pdu_forwarders(model: "FLYNCModel") -> List[Tuple["Controller", "Socket", PDUForwarder]]:
    """Collect every PDUForwarder in the workspace alongside its parent socket and owning controller."""

    out: List[Tuple["Controller", "Socket", PDUForwarder]] = []
    for ecu in model.ecus:
        for controller in ecu.controllers:
            for socket in _iter_sockets_on_controller(controller):
                for fwd in _iter_pdu_forwarders_on_socket(socket):
                    out.append((controller, socket, fwd))
    return out


def _iter_pdu_deployments_on_socket(socket: "Socket"):
    """Yield every :class:`PDUSender` / :class:`PDUReceiver` deployment carried by *socket*."""

    from flync.model.flync_4_signal.pdu_deployment import PDUReceiver, PDUSender  # local import — avoid module cycle

    for dep_root in socket.deployments or []:
        dep = dep_root.root
        if isinstance(dep, (PDUSender, PDUReceiver)):
            yield dep


def _collect_pdu_deployments(model: "FLYNCModel") -> List[Tuple["Controller", "Socket", "Union[PDUSender, PDUReceiver]"]]:
    """Collect every PDUSender / PDUReceiver in the workspace alongside its parent socket and owning controller.

    Deployments are collected unconditionally — standalone senders/receivers (plain ECU-to-ECU
    PDU exchange with no forwarder involved) are included exactly like forwarder-adjacent ones.
    """

    out: List[Tuple["Controller", "Socket", "Union[PDUSender, PDUReceiver]"]] = []
    for ecu in model.ecus:
        for controller in ecu.controllers:
            for socket in _iter_sockets_on_controller(controller):
                for dep in _iter_pdu_deployments_on_socket(socket):
                    out.append((controller, socket, dep))
    return out


def _iter_can_interfaces(model: "FLYNCModel"):
    """Yield ``(controller, can_interface)`` for every CAN interface in the workspace."""

    for ecu in model.ecus:
        for controller in ecu.controllers:
            for can_iface in controller.can_interfaces or []:
                yield controller, can_iface


def _collect_can_frame_forwarders(model: "FLYNCModel"):
    """Collect every CANFrameForwarder in the workspace alongside its parent CAN interface and owning controller."""

    out = []
    for controller, can_iface in _iter_can_interfaces(model):
        for fwd in can_iface.forwarder_frames or []:
            out.append((controller, can_iface, fwd))
    return out


def _build_socket_indexes(model: "FLYNCModel"):
    """Build controller-scoped indexes for socket and PDUForwarder lookup (socket names may collide across controllers)."""

    socket_by_controller_name: Dict[Tuple[str, str], Tuple["Controller", "Socket"]] = {}
    pdu_forwarder_by_controller_socket_pdu: Dict[Tuple[str, str, str], PDUForwarder] = {}

    for ecu in model.ecus:
        for controller in ecu.controllers:
            for socket in _iter_sockets_on_controller(controller):
                socket_by_controller_name[(controller.name, socket.name)] = (controller, socket)
                for fwd in _iter_pdu_forwarders_on_socket(socket):
                    pdu_forwarder_by_controller_socket_pdu[(controller.name, socket.name, fwd.pdu_ref)] = fwd
    return socket_by_controller_name, pdu_forwarder_by_controller_socket_pdu


def _index_can_forwarders_by_bus_id(
    can_iface: CANInterface,
    can_frame_catalog: Dict[str, CANAnyFrame],
    out: Dict[Tuple[str, int], CANFrameForwarder],
) -> None:
    """Add every forwarder frame of *can_iface* to *out*, keyed by ``(bus_ref, can_id)``."""

    for fwd in can_iface.forwarder_frames or []:
        frame = can_frame_catalog.get(fwd.frame_ref)
        if frame is not None:
            out[(can_iface.bus_ref, frame.can_id)] = fwd


def _build_can_indexes(model: "FLYNCModel", can_frame_catalog: Optional[Dict[str, CANAnyFrame]] = None):
    """Build indexes for CAN interface lookup ``(controller, bus)`` and CAN forwarder lookup ``(bus, can_id)``."""

    can_iface_by_controller_bus: Dict[Tuple[str, str], CANInterface] = {}
    can_forwarder_by_bus_id: Dict[Tuple[str, int], CANFrameForwarder] = {}

    for controller, can_iface in _iter_can_interfaces(model):
        can_iface_by_controller_bus[(controller.name, can_iface.bus_ref)] = can_iface
        if can_frame_catalog is not None:
            _index_can_forwarders_by_bus_id(can_iface, can_frame_catalog, can_forwarder_by_bus_id)
    return can_iface_by_controller_bus, can_forwarder_by_bus_id


# ---------------------------------------------------------------------------
# PDU and frame catalogs (built from FLYNCModel.communication.channels)
# ---------------------------------------------------------------------------


def _build_pdu_catalog(model: "FLYNCModel") -> Dict[str, PDU]:
    """Return a name-keyed dict of every Standard / Multiplexed / Container PDU declared under ``communication.channels``."""

    out: Dict[str, PDU] = {}
    channels = getattr(model.communication, "channels", None) if model.communication else None
    if channels is None:
        return out
    for pdu in channels.pdus or []:
        out[pdu.name] = pdu
    for container in channels.ethernet_pdu_containers or []:
        out[container.name] = container
    return out


def _build_can_frame_catalog(model: "FLYNCModel") -> Dict[str, CANAnyFrame]:
    """Return a name-keyed dict of every CAN / CAN FD frame declared under ``communication.channels.can_buses``."""

    out: Dict[str, CANAnyFrame] = {}
    channels = getattr(model.communication, "channels", None) if model.communication else None
    if channels is None or channels.can_buses is None:
        return out
    for bus in channels.can_buses:
        for frame in bus.frames or []:
            out[frame.name] = frame
    return out


def _build_can_frame_catalog_by_bus_id(model: "FLYNCModel") -> Dict[Tuple[str, int], CANAnyFrame]:
    """Return a ``(bus_name, can_id)``-keyed dict of every CAN / CAN FD frame, for egress lookups by CAN ID."""

    out: Dict[Tuple[str, int], CANAnyFrame] = {}
    channels = getattr(model.communication, "channels", None) if model.communication else None
    if channels is None or channels.can_buses is None:
        return out
    for bus in channels.can_buses:
        for frame in bus.frames or []:
            out[(bus.name, frame.can_id)] = frame
    return out


# ---------------------------------------------------------------------------
# Per-forwarder reference resolution + payload fit
# ---------------------------------------------------------------------------


def _resolve_egress_pdu(
    ingress_pdu: Optional[PDU],
    extract_pdu_ref: Optional[str],
    pdu_catalog: Dict[str, PDU],
    egress_label: str,
    owner: str,
) -> Optional[PDU]:
    """Return the PDU an egress actually emits, applying optional container extraction (raises on invalid ``extract_pdu_ref``)."""

    if extract_pdu_ref is None:
        return ingress_pdu
    if ingress_pdu is None:
        return None
    if not isinstance(ingress_pdu, ContainerPDU):
        raise err_minor(
            "{owner}: {egress}: extract_pdu_ref='{ref}' is only valid when ingress is a ContainerPDU; ingress '{ingress}' is a {kind}.",
            owner=owner,
            egress=egress_label,
            ref=extract_pdu_ref,
            ingress=ingress_pdu.name,
            kind=type(ingress_pdu).__name__,
            category=Category.CONSISTENCY,
            error_number="035",
        )
    inner_refs = [c.pdu_ref for c in ingress_pdu.contained_pdus]
    if extract_pdu_ref not in inner_refs:
        raise err_major(
            "{owner}: {egress}: extract_pdu_ref '{ref}' is not contained in ContainerPDU '{ingress}' (contained_pdus = {inner}).",
            owner=owner,
            egress=egress_label,
            ref=extract_pdu_ref,
            ingress=ingress_pdu.name,
            inner=sorted(set(inner_refs)),
            category=Category.REFERENCE,
            error_number="036",
        )
    if inner_refs.count(extract_pdu_ref) > 1:
        raise err_major(
            "{owner}: {egress}: extract_pdu_ref '{ref}' is ambiguous; ContainerPDU '{ingress}' contains it more than once.",
            owner=owner,
            egress=egress_label,
            ref=extract_pdu_ref,
            ingress=ingress_pdu.name,
            category=Category.UNIQUENESS,
            error_number="037",
        )
    return pdu_catalog.get(extract_pdu_ref)


def _validate_pdu_forwarder_refs_for(
    fwd: PDUForwarder,
    pdu_catalog: Dict[str, PDU],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Resolve and payload-check one ``PDUForwarder`` against the workspace PDU / CAN-frame catalogs."""

    owner = f"PDUForwarder(pdu_ref={fwd.pdu_ref})"
    ingress_pdu = pdu_catalog.get(fwd.pdu_ref)
    if ingress_pdu is None:
        raise err_major(
            "{owner}: pdu_ref '{ref}' does not name any PDU declared under communication.channels.",
            owner=owner,
            ref=fwd.pdu_ref,
            category=Category.REFERENCE,
            error_number="038",
        )
    _resolve_sinks_for(fwd.egresses, ingress_pdu, pdu_catalog, can_frame_by_bus_id, owner)


def _validate_can_frame_forwarder_refs_for(
    fwd: CANFrameForwarder,
    pdu_catalog: Dict[str, PDU],
    can_frame_catalog: Dict[str, CANAnyFrame],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Resolve and payload-check one ``CANFrameForwarder`` (ingress PDU is the single packed PDU of its ingress frame)."""

    owner = f"CANFrameForwarder(frame_ref={fwd.frame_ref})"
    ingress_frame = can_frame_catalog.get(fwd.frame_ref)
    if ingress_frame is None:
        raise err_major(
            "{owner}: frame_ref '{ref}' does not name any CAN or CAN FD frame declared under communication.channels.can_buses.",
            owner=owner,
            ref=fwd.frame_ref,
            category=Category.REFERENCE,
            error_number="039",
        )
    ingress_pdu: Optional[PDU] = None
    if ingress_frame.packed_pdus and len(ingress_frame.packed_pdus) == 1:
        ingress_pdu = pdu_catalog.get(ingress_frame.packed_pdus[0].pdu_ref)
    _resolve_sinks_for(fwd.egresses, ingress_pdu, pdu_catalog, can_frame_by_bus_id, owner)


def _resolve_sinks_for(
    egresses,
    ingress_pdu: Optional[PDU],
    pdu_catalog: Dict[str, PDU],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
    owner: str,
) -> None:
    """Per-egress resolution + payload-fit (CAN egresses only)."""

    for idx, egress_root in enumerate(egresses):
        egress = egress_root.root
        egress_label = f"egresses[{idx}]"
        egress_pdu = _resolve_egress_pdu(ingress_pdu, egress.extract_pdu_ref, pdu_catalog, egress_label, owner)
        if isinstance(egress, CANFrameEgress):
            egress_frame = can_frame_by_bus_id.get((egress.bus_ref, egress.frame_ref))
            if egress_frame is None:
                raise err_major(
                    "{owner}: {egress}: frame_ref id={ref} on bus '{bus}' does not name any CAN or CAN FD frame.",
                    owner=owner,
                    egress=egress_label,
                    ref=egress.frame_ref,
                    bus=egress.bus_ref,
                    category=Category.REFERENCE,
                    error_number="040",
                )
            if egress_pdu is not None and egress_pdu.length > egress_frame.length:
                raise err_minor(
                    "{owner}: {egress}: PDU '{pdu}' ({pdu_len} B) does not fit egress frame '{frame}' ({frame_len} B).",
                    owner=owner,
                    egress=egress_label,
                    pdu=egress_pdu.name,
                    pdu_len=egress_pdu.length,
                    frame=egress_frame.name,
                    frame_len=egress_frame.length,
                    category=Category.CONSISTENCY,
                    error_number="041",
                )


def validate_forwarder_refs(model: "FLYNCModel") -> None:
    """Workspace pass: resolve every forwarder's PDU / frame / extract refs and assert payload-fit on CAN egresses."""

    pdu_catalog = _build_pdu_catalog(model)
    can_frame_catalog = _build_can_frame_catalog(model)
    can_frame_by_bus_id = _build_can_frame_catalog_by_bus_id(model)

    for ctrl, socket, fwd in _collect_pdu_forwarders(model):
        try:
            _validate_pdu_forwarder_refs_for(fwd, pdu_catalog, can_frame_by_bus_id)
        except PydanticCustomError as err:
            raise _with_source(err, _pdu_forwarder_locator(ctrl, socket, fwd)) from None

    for ctrl, iface, fwd in _collect_can_frame_forwarders(model):
        try:
            _validate_can_frame_forwarder_refs_for(fwd, pdu_catalog, can_frame_catalog, can_frame_by_bus_id)
        except PydanticCustomError as err:
            raise _with_source(err, _can_forwarder_locator(ctrl, iface, fwd)) from None


def validate_pdu_deployment_refs(model: "FLYNCModel") -> None:
    """Workspace pass: every pdu_sender / pdu_receiver ``pdu_ref`` must name a PDU declared under ``communication.channels``.

    Any PDU kind is a valid target (Standard, Multiplexed, or Container). The pass covers all
    deployments, whether they take part in a forwarder chain or are standalone senders/receivers.
    """

    pdu_catalog = _build_pdu_catalog(model)
    for controller, socket, dep in _collect_pdu_deployments(model):
        if dep.pdu_ref not in pdu_catalog:
            err = err_major(
                "{owner}: pdu_ref '{ref}' does not name any PDU declared under communication.channels.",
                owner=f"{type(dep).__name__}(socket={socket.name}, pdu_ref={dep.pdu_ref})",
                ref=dep.pdu_ref,
                category=Category.REFERENCE,
                error_number="175",
            )
            raise _with_source(err, _pdu_deployment_locator(controller, socket, dep)) from None


# ---------------------------------------------------------------------------
# Locality + direction safety
# ---------------------------------------------------------------------------


def _post_extract_pdu_ref(ingress_pdu_ref: str, extract_pdu_ref: Optional[str]) -> str:
    """Return the PDU name actually emitted by an egress, after optional extraction."""

    return extract_pdu_ref if extract_pdu_ref is not None else ingress_pdu_ref


def _check_eth_socket_egress(
    egress: EthSocketEgress,
    owner: str,
    egress_pdu_ref: str,
    forwarder_controller: "Controller",
    socket_by_controller_name: Dict[Tuple[str, str], Tuple["Controller", "Socket"]],
) -> None:
    """Assert a ``eth_socket`` egress targets a same-controller socket carrying a matching PDUSender."""

    from flync.model.flync_4_signal.pdu_deployment import PDUSender  # local import — avoid module cycle

    entry = socket_by_controller_name.get((forwarder_controller.name, egress.socket_ref))
    if entry is None:
        raise err_major(
            "{owner}: eth_socket egress targets socket '{ref}' which does not exist on controller '{ctrl}'.",
            owner=owner,
            ref=egress.socket_ref,
            ctrl=forwarder_controller.name,
            category=Category.REFERENCE,
            error_number="042",
        )
    _, target_socket = entry
    has_matching_sender = any(isinstance(d.root, PDUSender) and d.root.pdu_ref == egress_pdu_ref for d in (target_socket.deployments or []))
    if not has_matching_sender:
        raise err_major(
            "{owner}: eth_socket egress target '{ref}' has no pdu_sender deployment for PDU '{pdu}'; cannot publish forwarded payload.",
            owner=owner,
            ref=egress.socket_ref,
            pdu=egress_pdu_ref,
            category=Category.CONSISTENCY,
            error_number="043",
        )


def _check_can_frame_egress(
    egress: CANFrameEgress,
    owner: str,
    forwarder_controller: "Controller",
    can_iface_by_controller_bus: Dict[Tuple[str, str], CANInterface],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Assert a ``can_frame`` egress targets a CAN interface on the forwarder's controller that is set up to send the egress frame."""

    iface = can_iface_by_controller_bus.get((forwarder_controller.name, egress.bus_ref))
    if iface is None:
        raise err_major(
            "{owner}: can_frame egress targets bus '{bus}' which has no CAN interface on controller '{ctrl}'.",
            owner=owner,
            bus=egress.bus_ref,
            ctrl=forwarder_controller.name,
            category=Category.REFERENCE,
            error_number="044",
        )
    egress_frame = can_frame_by_bus_id.get((egress.bus_ref, egress.frame_ref))
    if egress_frame is None or not any(s.bus_ref == egress.bus_ref and s.frame_ref == egress.frame_ref for s in iface.sender_frames):
        raise err_major(
            "{owner}: can_frame egress targets frame id={frame} on bus '{bus}', "
            "but controller '{ctrl}' does not list it in sender_frames of that interface.",
            owner=owner,
            frame=egress.frame_ref,
            bus=egress.bus_ref,
            ctrl=forwarder_controller.name,
            category=Category.CONSISTENCY,
            error_number="045",
        )


def _check_forwarder_egress_locality(
    egress,
    egress_owner: str,
    egress_pdu_ref: Optional[str],
    controller: "Controller",
    socket_by_controller_name: Dict[Tuple[str, str], Tuple["Controller", "Socket"]],
    can_iface_by_controller_bus: Dict[Tuple[str, str], CANInterface],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Dispatch a single egress to its locality check; raise on an unresolvable eth_socket egress PDU."""

    if isinstance(egress, EthSocketEgress):
        if egress_pdu_ref is None:
            raise err_major(
                "{owner}: eth_socket egress cannot resolve egress PDU; ingress frame has no single packed PDU.",
                owner=egress_owner,
                category=Category.REFERENCE,
                error_number="046",
            )
        _check_eth_socket_egress(egress, egress_owner, egress_pdu_ref, controller, socket_by_controller_name)
    else:
        _check_can_frame_egress(egress, egress_owner, controller, can_iface_by_controller_bus, can_frame_by_bus_id)


def _validate_pdu_forwarder_locality(
    controller: "Controller",
    socket: "Socket",
    fwd: PDUForwarder,
    socket_by_controller_name: Dict[Tuple[str, str], Tuple["Controller", "Socket"]],
    can_iface_by_controller_bus: Dict[Tuple[str, str], CANInterface],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Locality + direction safety for every egress of one ``PDUForwarder``."""

    owner = f"PDUForwarder(socket={socket.name}, pdu_ref={fwd.pdu_ref})"
    for idx, egress_root in enumerate(fwd.egresses):
        egress = egress_root.root
        egress_pdu_ref = _post_extract_pdu_ref(fwd.pdu_ref, egress.extract_pdu_ref)
        _check_forwarder_egress_locality(
            egress,
            f"{owner} egresses[{idx}]",
            egress_pdu_ref,
            controller,
            socket_by_controller_name,
            can_iface_by_controller_bus,
            can_frame_by_bus_id,
        )


def _validate_can_frame_forwarder_locality(
    controller: "Controller",
    parent_iface: CANInterface,
    fwd: CANFrameForwarder,
    socket_by_controller_name: Dict[Tuple[str, str], Tuple["Controller", "Socket"]],
    can_iface_by_controller_bus: Dict[Tuple[str, str], CANInterface],
    can_frame_catalog: Dict[str, CANAnyFrame],
    can_frame_by_bus_id: Dict[Tuple[str, int], CANAnyFrame],
) -> None:
    """Locality + direction safety for every egress of one ``CANFrameForwarder``."""

    owner = f"CANFrameForwarder(bus={parent_iface.bus_ref}, frame_ref={fwd.frame_ref})"
    ingress_egress_pdu_ref: Optional[str] = _egress_pdu_for_can_forwarder(fwd, can_frame_catalog)
    for idx, egress_root in enumerate(fwd.egresses):
        egress = egress_root.root
        per_sink_egress = _post_extract_pdu_ref(ingress_egress_pdu_ref, egress.extract_pdu_ref) if ingress_egress_pdu_ref else None
        _check_forwarder_egress_locality(
            egress,
            f"{owner} egresses[{idx}]",
            per_sink_egress,
            controller,
            socket_by_controller_name,
            can_iface_by_controller_bus,
            can_frame_by_bus_id,
        )


def validate_forwarder_locality(model: "FLYNCModel") -> None:
    """Workspace pass: assert every egress is same-controller and the target carries the matching ``pdu_sender`` / ``sender_frames``."""

    socket_by_controller_name, _ = _build_socket_indexes(model)
    can_iface_by_controller_bus, _ = _build_can_indexes(model)
    can_frame_catalog = _build_can_frame_catalog(model)
    can_frame_by_bus_id = _build_can_frame_catalog_by_bus_id(model)

    for controller, socket, fwd in _collect_pdu_forwarders(model):
        try:
            _validate_pdu_forwarder_locality(controller, socket, fwd, socket_by_controller_name, can_iface_by_controller_bus, can_frame_by_bus_id)
        except PydanticCustomError as err:
            raise _with_source(err, _pdu_forwarder_locator(controller, socket, fwd)) from None

    for controller, parent_iface, fwd in _collect_can_frame_forwarders(model):
        try:
            _validate_can_frame_forwarder_locality(
                controller,
                parent_iface,
                fwd,
                socket_by_controller_name,
                can_iface_by_controller_bus,
                can_frame_catalog,
                can_frame_by_bus_id,
            )
        except PydanticCustomError as err:
            raise _with_source(err, _can_forwarder_locator(controller, parent_iface, fwd)) from None


def _egress_pdu_for_can_forwarder(
    fwd: CANFrameForwarder,
    can_frame_catalog: Dict[str, CANAnyFrame],
) -> Optional[str]:
    """Return the PDU name carried by the CAN forwarder's ingress frame, or ``None`` when the frame packs zero or multiple PDUs."""

    frame = can_frame_catalog.get(fwd.frame_ref)
    if frame is None:
        return None
    if frame.packed_pdus and len(frame.packed_pdus) == 1:
        return frame.packed_pdus[0].pdu_ref
    return None


# ---------------------------------------------------------------------------
# Cycle detection (three-colour DFS)
# ---------------------------------------------------------------------------


_WHITE, _GRAY, _BLACK = 0, 1, 2


class _ForwarderCycleDetector(object):
    """Three-colour DFS over the workspace forwarder graph."""

    def __init__(self, model: "FLYNCModel") -> None:
        _, self._pdu_forwarder_idx = _build_socket_indexes(model)
        self._can_frame_catalog = _build_can_frame_catalog(model)
        _, self._can_forwarder_idx = _build_can_indexes(model, self._can_frame_catalog)

        pdu_fwds = _collect_pdu_forwarders(model)
        can_fwds = _collect_can_frame_forwarders(model)

        self._nodes: List[object] = [fwd for _, _, fwd in pdu_fwds]
        self._nodes.extend(fwd for _, _, fwd in can_fwds)

        self._controller_of: Dict[int, "Controller"] = {id(fwd): ctrl for ctrl, _, fwd in pdu_fwds}
        self._controller_of.update({id(fwd): ctrl for ctrl, _, fwd in can_fwds})

        # Full path-style Source locator per node, so a detected cycle can name the
        # interface (and socket) of every forwarder on the loop, not just the controller.
        self._locator_of: Dict[int, str] = {id(fwd): _pdu_forwarder_locator(ctrl, socket, fwd) for ctrl, socket, fwd in pdu_fwds}
        self._locator_of.update({id(fwd): _can_forwarder_locator(ctrl, iface, fwd) for ctrl, iface, fwd in can_fwds})

        self._state: Dict[int, int] = {id(n): _WHITE for n in self._nodes}
        self._parent: Dict[int, object] = {}

    def _ingress_pdu_ref(self, fwd) -> Optional[str]:
        """Return the PDU name received by ``fwd``."""
        if isinstance(fwd, PDUForwarder):
            return fwd.pdu_ref
        return _egress_pdu_for_can_forwarder(fwd, self._can_frame_catalog)

    def _resolve_eth_neighbour(self, fwd, egress: EthSocketEgress) -> Optional[object]:
        """Resolve the PDU-forwarder reached via an ``eth_socket`` egress, or ``None`` if unresolvable."""
        egress_pdu_ref = egress.extract_pdu_ref if egress.extract_pdu_ref is not None else self._ingress_pdu_ref(fwd)
        src_controller = self._controller_of.get(id(fwd))
        if egress_pdu_ref is None or src_controller is None:
            return None
        return self._pdu_forwarder_idx.get((src_controller.name, egress.socket_ref, egress_pdu_ref))

    def _resolve_neighbour(self, fwd, egress) -> Optional[object]:
        """Resolve the forwarder reached via one egress, or ``None`` if it does not target one."""
        if isinstance(egress, EthSocketEgress):
            return self._resolve_eth_neighbour(fwd, egress)
        return self._can_forwarder_idx.get((egress.bus_ref, egress.frame_ref))

    def _neighbours(self, fwd) -> List[object]:
        """Return forwarders reachable via ``fwd``'s egresses (graph adjacency for the DFS)."""
        out: List[object] = []
        for egress_root in fwd.egresses:
            nxt = self._resolve_neighbour(fwd, egress_root.root)
            if nxt is not None:
                out.append(nxt)
        return out

    @staticmethod
    def _fmt(fwd) -> str:
        """Format ``fwd`` as a short label for cycle-path error messages."""
        if isinstance(fwd, PDUForwarder):
            return f"PDUForwarder(pdu_ref={fwd.pdu_ref})"
        return f"CANFrameForwarder(frame_ref={fwd.frame_ref})"

    def _cycle_nodes(self, start, end) -> List[object]:
        """Ordered forwarder nodes forming the detected cycle (``end ... start ... end``), via the DFS parent map."""
        nodes: List[object] = [end]
        cur = start
        while cur is not None and cur is not end:
            nodes.append(cur)
            cur = self._parent.get(id(cur))
        nodes.append(end)
        return list(reversed(nodes))

    def _cycle_path(self, start, end) -> List[str]:
        """Short labels for each forwarder on the cycle (for the error message)."""
        return [self._fmt(node) for node in self._cycle_nodes(start, end)]

    def _cycle_source(self, start, end) -> str:
        """Source locator spanning every distinct interface/socket the cycle passes through."""
        seen: List[str] = []
        for node in self._cycle_nodes(start, end):
            locator = self._locator_of.get(id(node))
            if locator and locator not in seen:
                seen.append(locator)
        return " ; ".join(seen)

    def _visit_neighbour(self, u, v) -> None:
        """Apply DFS edge rules for ``u -> v``; raise on a GRAY back-edge."""
        v_state = self._state.get(id(v), _WHITE)
        if v_state == _GRAY:
            raise _with_source(
                err_major(
                    "Forwarder cycle detected: {path}",
                    path=" -> ".join(self._cycle_path(u, v)),
                    category=Category.STRUCTURAL,
                    error_number="047",
                ),
                self._cycle_source(u, v),
            )
        if v_state == _WHITE:
            self._parent[id(v)] = u
            self._dfs(v)

    def _dfs(self, u) -> None:
        """Recursive three-colour DFS body."""
        self._state[id(u)] = _GRAY
        for v in self._neighbours(u):
            self._visit_neighbour(u, v)
        self._state[id(u)] = _BLACK

    def detect(self) -> None:
        """Run DFS from every still-WHITE node."""
        for n in self._nodes:
            if self._state[id(n)] == _WHITE:
                self._dfs(n)


def detect_forwarder_cycles(model: "FLYNCModel") -> None:
    """Workspace pass: three-colour DFS over the forwarder graph; raises ``err_major`` with the cycle path on a back-edge."""

    _ForwarderCycleDetector(model).detect()
