"""Decode DBC databases into a FLYNC model (DBC to FLYNC direction)."""

import importlib.metadata
import logging
from collections import defaultdict
from typing import Dict, List, Literal, Optional, Tuple

from cantools.database.can.message import Message
from cantools.database.can.signal import Signal

from flync.model import FLYNCModel  # type: ignore[import-untyped]
from flync.model.flync_4_bus.can_bus import CANBus
from flync.model.flync_4_communication.flync_channels import FLYNCChannelConfig
from flync.model.flync_4_communication.flync_communication import FLYNCCommunicationConfig
from flync.model.flync_4_ecu.can_interface import CANFrameRef, CANInterface
from flync.model.flync_4_ecu.controller import Controller
from flync.model.flync_4_ecu.ecu import ECU
from flync.model.flync_4_metadata.metadata import BaseVersion, ECUMetadata, EmbeddedMetadata, SystemMetadata
from flync.model.flync_4_signal import MultiplexedPDU, SignalInstance, StandardPDU
from flync.model.flync_4_signal.frame import CANFDFrame, CANFrame
from flync.model.flync_4_signal.pdu import MuxGroup, PDUInstance
from flync.model.flync_4_signal.signal import Signal as FLYNCSignal
from flync.model.flync_4_signal.signal import SignalDataType
from flync.model.flync_4_signal.value_encoding import TextEntry, TextTable

from .dbc_config import DbcConverterConfig
from .loading import _fd_baud_rate, _nominal_baud_rate

logger = logging.getLogger(__name__)


def _comment_text(comment) -> Optional[str]:
    """Normalize a cantools comment (``str`` or language-keyed ``dict``) to text."""
    if isinstance(comment, dict):
        return comment.get("EN") or (next(iter(comment.values())) if comment else None)
    return comment


def _bit_width_index(bit_length: int) -> int:
    """Return the bucket index (0..3) for a 8/16/32/64-bit signal width."""
    index = 3
    match bit_length:
        case n if n <= 8:
            index = 0
        case n if n <= 16:
            index = 1
        case n if n <= 32:
            index = 2
    return index


_UG_INT_TYPES = (SignalDataType.UINT8, SignalDataType.UINT16, SignalDataType.UINT32, SignalDataType.UINT64)
_SG_INT_TYPES = (SignalDataType.INT8, SignalDataType.INT16, SignalDataType.INT32, SignalDataType.INT64)


def map_data_type(bit_length: int, is_signed: bool, is_float: bool) -> SignalDataType:
    """Map DBC signal length/sign/floatness to the smallest fitting FLYNC data type.

    Integer signals wider than 64 bits (diagnostic/crypto blobs) cannot be
    represented as a fixed-width integer, so they map to ``bytearray`` (which
    accepts any multiple-of-8 bit length).
    """
    if is_float:
        return SignalDataType.FLOAT32 if bit_length <= 32 else SignalDataType.FLOAT64
    if bit_length > 64:
        return SignalDataType.BYTEARRAY
    types = _SG_INT_TYPES if is_signed else _UG_INT_TYPES
    return types[_bit_width_index(bit_length)]


def _coerce_initial_value(raw_initial, data_type: SignalDataType):
    """Return a FLYNC-compatible ``initial_value`` or ``None`` when it cannot be represented."""
    result = None
    if data_type.is_float() and raw_initial is not None:
        result = raw_initial
    elif data_type == SignalDataType.BYTEARRAY and isinstance(raw_initial, bytes):
        result = raw_initial
    elif isinstance(raw_initial, int) and not isinstance(raw_initial, bool):
        result = raw_initial
    elif isinstance(raw_initial, float) and raw_initial.is_integer():
        result = int(raw_initial)
    return result


def _to_flync_signal(s: Signal) -> FLYNCSignal:
    """Convert a cantools Signal into a FLYNC Signal."""
    data_type = map_data_type(s.length, s.is_signed, s.is_float)
    kwargs: dict = {
        "name": s.name,
        "description": _comment_text(s.comment),
        "bit_length": s.length,
        "data_type": data_type,
        "factor": s.scale,
        "offset": s.offset,
        "unit": s.unit or None,
        "initial_value": _coerce_initial_value(s.raw_initial, data_type),
    }
    if s.minimum is not None:
        kwargs["lower_limit"] = s.minimum
    if s.maximum is not None:
        kwargs["upper_limit"] = s.maximum

    choices = _in_range_choices(s, data_type)
    if choices:
        kwargs["value_encoding"] = TextTable(
            type="text_table",
            entries=[TextEntry(value=int(value), label=getattr(label, "name", str(label))) for value, label in choices.items()],
        )
    return FLYNCSignal(**kwargs)


def _in_range_choices(s: Signal, data_type: SignalDataType) -> Optional[Dict[int, object]]:
    """Return ``s.choices`` filtered to entries representable by the FLYNC signal.

    Complex types (``bytearray``) have no text-table encoding, and raw values
    that fall outside the signal's bit range (common in hand-edited DBCs, e.g.
    a ``7-bit`` signal with a ``VAL_`` entry at ``127``) are dropped since FLYNC
    cannot represent them. Drops are surfaced as warnings.
    """
    if not s.choices:
        return None
    if data_type.is_complex_datattype():
        logger.warning("Value table dropped for bytearray signal '%s'", s.name)
        return None
    if s.is_signed:
        lo, hi = -(1 << (s.length - 1)), (1 << (s.length - 1)) - 1
    else:
        lo, hi = 0, (1 << s.length) - 1
    dropped = [int(v) for v in s.choices if not (lo <= int(v) <= hi)]
    if dropped:
        logger.warning(
            "Value table entries for signal '%s' outside bit range [%d, %d] dropped: %s",
            s.name,
            lo,
            hi,
            sorted(dropped),
        )
    return {value: label for value, label in s.choices.items() if lo <= int(value) <= hi}


def _to_flync_signal_instance(s: Signal) -> SignalInstance:
    """Convert a cantools Signal into a FLYNC SignalInstance, keeping its absolute bit start."""
    return SignalInstance(
        signal=_to_flync_signal(s),
        bit_position=s.start,
        endianness="BE" if s.byte_order == "big_endian" else "LE",
    )


def _build_sub_pdu(name: str, sig_list: List[Signal]) -> Tuple[Optional[StandardPDU], Optional[int]]:
    """Build a StandardPDU from signals, re-basing them relative to their lowest start bit.

    FLYNC's :class:`MultiplexedPDU` placement model expects a static/mux-group PDU to
    occupy ``[bit_position, bit_position + length * 8)`` starting at the placement's
    ``bit_position``.  DBC signals carry absolute bit offsets, so this helper shifts
    them to be relative to the group's lowest start and places the PDU at that offset
    with a length covering exactly the group footprint.

    Returns:
        ``(pdu, base_bit_position)`` or ``(None, None)`` when ``sig_list`` is empty.
    """
    if not sig_list:
        return None, None
    base = min(s.start for s in sig_list)
    instances = [_to_flync_signal_instance(s) for s in sig_list]
    for si in instances:
        si.bit_position = (si.bit_position or 0) - base
    footprint = max((si.bit_position or 0) + si.signal.bit_length for si in instances)
    length = max(1, -(-footprint // 8))
    pdu = StandardPDU(name=name, type="standard", length=length, signals=instances)
    return pdu, base


def _collect_mux_groups(signals: List[Signal]) -> Dict[int, List[Signal]]:
    """Group multiplexed signals by their multiplexer id."""
    groups: Dict[int, List[Signal]] = {}
    for s in signals:
        if s.multiplexer_signal:
            for mid in s.multiplexer_ids or []:
                groups.setdefault(mid, []).append(s)
    return groups


def _build_static_group(
    extra: List[StandardPDU | MultiplexedPDU],
    main_name: str,
    static_signals: List[Signal],
) -> Optional[List[PDUInstance]]:
    """Build the static-group placement and register its PDU in ``extra``."""
    if not static_signals:
        return None
    static_name = f"{main_name}_static"
    static_pdu, static_base = _build_sub_pdu(static_name, static_signals)
    assert static_pdu is not None and static_base is not None
    extra.append(static_pdu)
    return [PDUInstance(pdu_ref=static_name, bit_position=static_base)]


def _build_mux_placements(
    extra: List[StandardPDU | MultiplexedPDU],
    main_name: str,
    mux_groups: Dict[int, List[Signal]],
) -> List[MuxGroup]:
    """Build the per-id mux-group placements and register their PDUs in ``extra``."""
    placements = []
    for mid in sorted(mux_groups.keys()):
        group_name = f"{main_name}_mux{mid}"
        group_pdu, group_base = _build_sub_pdu(group_name, mux_groups[mid])
        assert group_pdu is not None and group_base is not None
        extra.append(group_pdu)
        placements.append(MuxGroup(selector_value=mid, pdu=PDUInstance(pdu_ref=group_name, bit_position=group_base)))
    return placements


def _build_pdu_for_message(message: Message, prefix: str) -> Tuple[StandardPDU | MultiplexedPDU, List[StandardPDU | MultiplexedPDU]]:
    """Build the top-level PDU for a message plus any auxiliary PDUs it needs.

    A message without a multiplexer signal maps to a single :class:`StandardPDU`.
    A message with a multiplexer (``M``) signal maps to a :class:`MultiplexedPDU`
    whose static and mux-group signal sets are extracted into referenced
    :class:`StandardPDU` instances (returned in the second tuple element so the
    caller can register them).
    """
    signals = list(message.signals or [])
    selector = next((s for s in signals if s.is_multiplexer), None)

    main_name = f"{prefix}_{message.name}"
    pdu: StandardPDU | MultiplexedPDU
    if selector is None:
        pdu = StandardPDU(
            name=main_name,
            type="standard",
            length=message.length,
            signals=[_to_flync_signal_instance(s) for s in signals],
        )
        return pdu, []

    extra: List[StandardPDU | MultiplexedPDU] = []
    mux_groups = _collect_mux_groups(signals)
    static_signals = [s for s in signals if not s.is_multiplexer and not s.multiplexer_signal]
    static_group = _build_static_group(extra, main_name, static_signals)
    mux_placements = _build_mux_placements(extra, main_name, mux_groups)

    pdu = MultiplexedPDU(
        name=main_name,
        type="multiplexed",
        length=message.length,
        selector_signal=_to_flync_signal_instance(selector),
        static_group=static_group,
        mux_groups=mux_placements,
    )
    return pdu, extra


def _to_flync_frame(message: Message, pdu_name: str):
    """Convert a cantools Message into a FLYNC CANFrame/CANFDFrame referencing ``pdu_name``."""
    packed = [PDUInstance(pdu_ref=pdu_name, bit_position=0)]
    id_format: Literal["standard_11bit", "extended_29bit"] = "extended_29bit" if message.is_extended_frame else "standard_11bit"
    description = _comment_text(message.comment)
    if message.is_fd:
        return CANFDFrame(
            name=message.name,
            length=message.length,
            can_id=message.frame_id,
            id_format=id_format,
            description=description,
            packed_pdus=packed,
            type="can_fd",
        )
    return CANFrame(
        name=message.name,
        length=message.length,
        can_id=message.frame_id,
        id_format=id_format,
        description=description,
        packed_pdus=packed,
        type="can",
    )


def _flync_version() -> str:
    """Return a valid semver for generated metadata (installed flync version when parseable)."""
    try:
        version = importlib.metadata.version("flync")
        # ensure it parses as semver; fall back otherwise
        BaseVersion(version=version)
        return version
    except Exception:
        return "0.0.0"


def _register_frame_participants(
    message: Message,
    bus_name: str,
    frame_senders: Dict[Tuple[str, int], set],
    frame_receivers: Dict[Tuple[str, int], set],
) -> None:
    """Record which nodes send/receive a message's frame."""
    for sender in message.senders or []:
        frame_senders[(bus_name, message.frame_id)].add(sender)
    for sig in message.signals or []:
        for receiver in sig.receivers or []:
            frame_receivers[(bus_name, message.frame_id)].add(receiver)


def decode_dbc_files(dbc_files, config: Optional[DbcConverterConfig] = None) -> FLYNCModel:
    """Build and validate a FLYNCModel from loaded ``(Database, Path)`` tuples.

    Args:
        dbc_files: Output of :func:`load_dbc_files`.
        config: Optional DBC converter configuration (baud defaults).

    Returns:
        The validated FLYNCModel with one ``CANBus`` per DBC file and one
        synthesized ``ECU`` per node declared in ``BU_:``.
    """
    config = config or DbcConverterConfig(config_path="")

    pdus: List[StandardPDU | MultiplexedPDU] = []
    buses = []
    frame_senders: Dict[Tuple[str, int], set] = defaultdict(set)
    frame_receivers: Dict[Tuple[str, int], set] = defaultdict(set)
    declared_nodes: set = set()

    for db, path in dbc_files:
        bus_name = path.stem
        baud_rate = _nominal_baud_rate(db, config)
        fd_enabled = any(m.is_fd for m in db.messages)

        declared_nodes.update(node.name for node in db.nodes)

        frames = []
        for message in db.messages:
            pdu, extra = _build_pdu_for_message(message, bus_name)
            pdus.append(pdu)
            pdus.extend(extra)
            frames.append(_to_flync_frame(message, pdu.name))
            _register_frame_participants(message, bus_name, frame_senders, frame_receivers)

        buses.append(
            CANBus(
                name=bus_name,
                baud_rate=baud_rate,
                fd_enabled=fd_enabled,
                fd_baud_rate=_fd_baud_rate(db, config) if fd_enabled else None,
                frames=frames,
            )
        )

    ecus = _to_flync_ecus(frame_senders, frame_receivers, declared_nodes)
    version = _flync_version()

    return FLYNCModel(
        metadata=SystemMetadata(
            type="system",
            author="dbc-converter",
            compatible_flync_version=BaseVersion(version=version),
            release=BaseVersion(version=version),
        ),
        communication=FLYNCCommunicationConfig(
            channels=FLYNCChannelConfig(
                pdus=pdus,
                can_buses=buses,
            )
        ),
        ecus=ecus,
    )


def _to_flync_ecus(
    frame_senders: Dict[Tuple[str, int], set],
    frame_receivers: Dict[Tuple[str, int], set],
    declared_nodes: Optional[set] = None,
) -> List[ECU]:
    """Synthesize one ECU per DBC node with a single CAN controller and per-bus interfaces.

    The node set is the union of every node declared in the ``BU_:`` line and
    every participant observed on a frame, so a node that is declared but never
    participates in a message is still retained and not silently dropped.
    """
    membership: Dict[str, Dict[str, Dict[str, set]]] = defaultdict(lambda: defaultdict(lambda: defaultdict(set)))
    for (bus, fid), nodes in frame_senders.items():
        for node in nodes:
            membership[node][bus]["send"].add(fid)
    for (bus, fid), nodes in frame_receivers.items():
        for node in nodes:
            membership[node][bus]["recv"].add(fid)

    node_names = set(membership)
    if declared_nodes:
        node_names |= declared_nodes

    version = _flync_version()
    ecus = []
    for node in sorted(node_names):
        can_interfaces = []
        for bus in sorted(membership[node].keys()):
            roles = membership[node][bus]
            can_interfaces.append(
                CANInterface(
                    name=bus,
                    bus_ref=bus,
                    sender_frames=[CANFrameRef(bus_ref=bus, frame_ref=fid) for fid in sorted(roles.get("send", set()))],
                    receiver_frames=[CANFrameRef(bus_ref=bus, frame_ref=fid) for fid in sorted(roles.get("recv", set()))],
                )
            )
        if not can_interfaces:
            logger.warning(
                "Node '%s' is declared in BU_: but participates in no message/interface; "
                "cannot represent it as a FLYNC ECU and it will be omitted.",
                node,
            )
            continue
        controller = Controller(
            name="CONTROLLER",
            controller_metadata=EmbeddedMetadata(
                type="embedded",
                author="dbc-converter",
                compatible_flync_version=BaseVersion(version=version),
                target_system="DBC",
            ),
            can_interfaces=can_interfaces,
        )
        ecus.append(
            ECU(
                name=node,
                controllers=[controller],
                ecu_metadata=ECUMetadata(
                    type="ecu",
                    author="dbc-converter",
                    compatible_flync_version=BaseVersion(version=version),
                ),
            )
        )
    return ecus
