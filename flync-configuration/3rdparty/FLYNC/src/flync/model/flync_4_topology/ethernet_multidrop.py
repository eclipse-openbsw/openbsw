"""
The Ethernet multidrop branch of the system topology.
"""

from typing import Annotated, Dict, List, Literal, Optional, cast

from pydantic import Field, model_serializer, model_validator

from flync.core.annotations.reference import Reference
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major, warn
from flync.model.flync_4_ecu.phy import BASET1S
from flync.model.flync_4_ecu.port import ECUPort

#: PHYs that can sit on a shared medium. 10BASE-T1M joins this tuple.
MULTIDROP_PHYS = (BASET1S,)

_INTERFRAME_GAP_BIT_TIMES = 96


class PLCACycle(FLYNCBaseModel):
    """
    The PLCA parameters for a segment.

    Parameters
    ----------
    transmit_opportunity_count : int
        How many slots one cycle has, 1 to 255.  May exceed the slots handed out: a spare one costs ``to_timer`` bit times
        per cycle and leaves room for a node added later.
    to_timer : int, optional
        How long an unused slot stays open, in bit times of 100 ns.  Defaults to 32.
    """

    transmit_opportunity_count: Annotated[int, Field(ge=1, le=255)] = Field()
    to_timer: Annotated[int, Field(ge=1, le=255)] = Field(default=32)


class EthernetMultidropNode(FLYNCBaseModel):
    """
    One ECU port on a multidrop segment and the slot of the cycle it transmits in.

    Parameters
    ----------
    ecu_port_name : str
        Name of the ECU port on the segment (alias: ``ecu_port``).
    node_id : int, optional
        Its slot in the PLCA cycle, counted from 0.  Slot 0 makes it the coordinator.  Leave it off to opt the node out of PLCA:
        it then competes for the medium by CSMA/CD.
    burst_count : int, optional
        How many extra frames this node may send back-to-back inside its own slot.  Defaults to 0, one frame per slot.
    burst_timer : int, optional
        How long the node may keep the medium between those frames, in bit times.  Defaults to 128.  Has to stay above the 96 bit
        interframe gap, or the burst ends before the next frame starts.
    """

    ecu_port_name: Annotated[str, Reference(source="_ecu_port")] = Field(alias="ecu_port")
    node_id: Optional[Annotated[int, Field(ge=0, le=254)]] = Field(default=None)
    burst_count: Annotated[int, Field(ge=0, le=255)] = Field(default=0)
    burst_timer: Annotated[int, Field(ge=0, le=255)] = Field(default=128)

    _ecu_port: Optional[ECUPort] = None

    @property
    def ecu_port(self) -> Optional[ECUPort]:
        return self._ecu_port

    @property
    def ecu_name(self) -> Optional[str]:
        """Name of the ECU owning this port, available once the connection has been bound."""

        ecu = getattr(self._ecu_port, "_ecu", None) if self._ecu_port is not None else None
        return getattr(ecu, "name", None)

    @property
    def label(self) -> str:
        """How this node is named in a message: ``ECU/port`` once bound, the port name before that."""

        ecu = self.ecu_name
        return f"{ecu}/{self.ecu_port_name}" if ecu else self.ecu_port_name

    @property
    def participates(self) -> bool:
        """Whether this node holds a slot at all."""

        return self.node_id is not None

    @property
    def is_coordinator(self) -> bool:
        """Whether this node holds slot 0 and therefore opens each cycle."""

        return self.node_id == 0

    @property
    def role(self) -> Optional[str]:
        """This node's part in the cycle: ``"coordinator"`` for slot 0, ``"follower"`` otherwise, ``None`` with no slot."""

        if not self.participates:
            return None
        return "coordinator" if self.is_coordinator else "follower"

    @model_validator(mode="after")
    def check_burst_timer(self) -> "EthernetMultidropNode":
        """A burst timer at or below the interframe gap ends the burst before the next frame starts."""

        if self.burst_count > 0 and self.burst_timer <= _INTERFRAME_GAP_BIT_TIMES:
            raise err_major(
                "Node '{port}' bursts {count} extra frames but its burst_timer is {timer} bit times, at or below the {gap} bit "
                "interframe gap. The burst would end before the next frame starts.",
                port=self.ecu_port_name,
                count=self.burst_count,
                timer=self.burst_timer,
                gap=_INTERFRAME_GAP_BIT_TIMES,
                category=Category.VALUE_RANGE,
                error_number="336",
            )
        return self

    @model_validator(mode="after")
    def check_burst_needs_a_slot(self) -> "EthernetMultidropNode":
        """Bursting is a PLCA feature; with no slot the node competes by CSMA/CD and has nothing to burst inside."""

        if self.node_id is None and (self.burst_count != 0 or self.burst_timer != 128):
            raise err_major(
                "Node '{port}' sets burst_count/burst_timer but no node_id. Bursting needs a slot in the PLCA cycle; "
                "give the node a slot or drop the burst parameters.",
                port=self.ecu_port_name,
                category=Category.CONSISTENCY,
                error_number="335",
            )
        return self

    @model_serializer
    def serialize(self):
        payload = {"ecu_port": self.ecu_port_name}
        if self.node_id is not None:
            payload["node_id"] = self.node_id
        # Defaults stay out, so a file that never mentioned bursting does not grow them on a round trip.
        for field in ("burst_count", "burst_timer"):
            value = getattr(self, field)
            if value != type(self).model_fields[field].default:
                payload[field] = value
        return payload


def _slot(node: EthernetMultidropNode) -> int:
    """The node id a participating node holds; guarded by :meth:`~EthernetMultidropNode.participates`."""

    return cast(int, node.node_id)


class EthernetMultidropConnection(FLYNCBaseModel):
    """
    Connects N ECU ports on one shared medium.

    Parameters
    ----------
    type : Literal["ethernet_multidrop"]
        The type of the connection.  Defaults to ``"ethernet_multidrop"``.
    id : str
        Unique identifier of the connection, and the name the segment is known by.
    plca : :class:`PLCACycle`, optional
        The cycle shared by the nodes below.  Leaving it off makes the segment a plain CSMA/CD medium, and no node may then claim a slot.
    nodes : list of :class:`EthernetMultidropNode`
        The ports sharing this medium.
    """

    type: Literal["ethernet_multidrop"] = Field(default="ethernet_multidrop")
    id: str = Field()
    plca: Optional[PLCACycle] = Field(default=None)
    nodes: List[EthernetMultidropNode] = Field()

    @property
    def coordinator(self) -> Optional[EthernetMultidropNode]:
        """The node holding slot 0, or ``None`` when no node does."""

        return next((node for node in self.nodes if node.is_coordinator), None)

    @property
    def followers(self) -> List[EthernetMultidropNode]:
        """Every node taking part in PLCA without holding slot 0."""

        return [node for node in self.nodes if node.participates and not node.is_coordinator]

    @property
    def participants(self) -> List[EthernetMultidropNode]:
        """Every node holding a slot, in cycle order."""

        return sorted((node for node in self.nodes if node.participates), key=_slot)

    @model_validator(mode="after")
    def check_slots_need_a_cycle(self) -> "EthernetMultidropConnection":
        """A slot number without a cycle to sit in describes nothing."""

        if self.plca is None:
            claimed = sorted(node.ecu_port_name for node in self.nodes if node.participates)
            if claimed:
                raise err_major(
                    "Multidrop connection '{connection}' declares no 'plca' cycle, but node(s) {nodes} claim a node_id. "
                    "Add the cycle, or drop the slots and let the segment arbitrate by CSMA/CD.",
                    connection=self.id,
                    nodes=claimed,
                    category=Category.CONSISTENCY,
                    error_number="334",
                )
        return self

    @model_serializer
    def serialize(self):
        payload = {"type": self.type, "id": self.id}
        if self.plca is not None:
            # Dump the model, not its known fields, so a cycle widened later survives a save→reload round trip.
            payload["plca"] = self.plca.model_dump()
        payload["nodes"] = [node.serialize() for node in self.nodes]
        return payload

    def bind(self, ports_by_name: dict) -> None:
        """Resolve every node's ``ecu_port`` against the model's ports."""

        for node in self.nodes:
            port = ports_by_name.get(node.ecu_port_name)
            if port is None:
                raise err_major(
                    "ECU port '{port}' in multidrop connection '{connection}' of the system topology does not exist",
                    port=node.ecu_port_name,
                    connection=self.id,
                    category=Category.REFERENCE,
                    error_number="329",
                )
            node._ecu_port = port
            # Reflect the node's slot/burst onto the PHY so code generation can read a port's PLCA config from the PHY alone.
            if isinstance(port.mdi_config, BASET1S):
                port.mdi_config._multidrop_node = node


def validate_multidrop_connections(connections: List[EthernetMultidropConnection]) -> None:
    """Run the system-wide checks over every multidrop connection."""

    _validate_port_claimed_once(connections)
    for conn in connections:
        _validate_port_phys(conn)
        _warn_nodes_outside_plca(conn)
        _validate_coordinator_cardinality(conn)
        _validate_unique_opportunities(conn)
        _validate_opportunities_within_cycle(conn)
        _warn_unused_opportunities(conn)
        _warn_coordinator_missing_from_follower_group(conn)
        _warn_shaper_on_multidrop_port(conn)
        _validate_gptp_on_segment(conn)


def _validate_port_phys(conn: EthernetMultidropConnection) -> None:
    """Every node's port has to carry a PHY that can sit on a shared medium."""

    for node in conn.nodes:
        if node.ecu_port is None:
            continue

        mdi_config = node.ecu_port.mdi_config
        if not isinstance(mdi_config, MULTIDROP_PHYS):
            raise err_major(
                "Multidrop connection '{connection}' puts port '{port}' on the segment, but that port's MDI config is '{mode}'. "
                "A shared medium needs a multidrop-capable PHY.",
                connection=conn.id,
                port=node.label,
                mode=mdi_config.mode,
                category=Category.CONSISTENCY,
                error_number="331",
            )
        if mdi_config.topology != "multidrop":
            raise err_major(
                "Multidrop connection '{connection}' puts port '{port}' on the segment, but that port declares topology '{topology}'. "
                "Set it to 'multidrop', or connect the port point to point instead.",
                connection=conn.id,
                port=node.label,
                topology=mdi_config.topology,
                category=Category.CONSISTENCY,
                error_number="332",
            )


def _validate_port_claimed_once(connections: List[EthernetMultidropConnection]) -> None:
    """One port carries one PHY, so it sits on one segment and appears there once."""

    seen: Dict[str, str] = {}
    for conn in connections:
        for node in conn.nodes:
            if node.ecu_port_name in seen:
                raise err_major(
                    "Port '{port}' is claimed by two multidrop connections, '{first}' and '{second}'. One port carries one PHY and "
                    "therefore sits on one segment.",
                    port=node.ecu_port_name,
                    first=seen[node.ecu_port_name],
                    second=conn.id,
                    category=Category.UNIQUENESS,
                    error_number="333",
                )
            seen[node.ecu_port_name] = conn.id


def _warn_nodes_outside_plca(conn: EthernetMultidropConnection) -> None:
    """Warn about nodes competing for a medium the others arbitrate."""

    if conn.plca is None:
        return
    outside = sorted(node.label for node in conn.nodes if not node.participates)
    if outside:
        warn(
            f"Multidrop connection '{conn.id}': node(s) {outside} hold no transmit opportunity while the rest of the segment runs PLCA. "
            f"They transmit outside the cycle, which costs every node on the segment part of the bounded access time, not just them.",
            category=Category.CONSISTENCY,
            error_number="328",
        )


def _validate_coordinator_cardinality(conn: EthernetMultidropConnection) -> None:
    """Validate that not more than 1 Coordinator is present on Multidrop Ethernet."""

    coordinators = sorted(node.label for node in conn.nodes if node.is_coordinator)
    if len(coordinators) > 1:
        raise err_major(
            "Multidrop connection '{connection}' gives transmit opportunity 0 to {count} nodes ({nodes}); exactly one coordinator is "
            "required. Two nodes emitting BEACONs collide.",
            connection=conn.id,
            count=len(coordinators),
            nodes=coordinators,
            category=Category.CONSISTENCY,
            error_number="325",
        )
    if not coordinators and conn.plca is not None and conn.nodes:
        warn(
            f"Multidrop connection '{conn.id}' has no node holding transmit opportunity 0, so nothing emits the BEACON and PLCA never "
            f"starts. The segment stays operational at CSMA/CD performance. Give one node opportunity 0 unless that is deliberate.",
            category=Category.CONSISTENCY,
            error_number="326",
        )


def _validate_unique_opportunities(conn: EthernetMultidropConnection) -> None:
    """Two nodes on one slot contend for it, which is what the cycle exists to prevent."""

    seen: Dict[int, str] = {}
    for node in conn.participants:
        slot = _slot(node)
        if slot in seen:
            raise err_major(
                "Multidrop connection '{connection}': nodes '{first}' and '{second}' both claim transmit opportunity {slot}. "
                "A slot belongs to one node.",
                connection=conn.id,
                first=seen[slot],
                second=node.label,
                slot=slot,
                category=Category.UNIQUENESS,
                error_number="327",
            )
        seen[slot] = node.label


def _validate_opportunities_within_cycle(conn: EthernetMultidropConnection) -> None:
    """A slot above the last one in the cycle never comes round, so that node can never transmit."""

    if conn.plca is None:
        return
    declared = conn.plca.transmit_opportunity_count
    outside = sorted(f"{node.label} (slot {_slot(node)})" for node in conn.participants if _slot(node) >= declared)
    if outside:
        raise err_major(
            "Multidrop connection '{connection}' has node(s) claiming a slot outside the cycle: {nodes}. The cycle holds {declared} "
            "slots numbered 0 to {highest}.",
            connection=conn.id,
            nodes=outside,
            declared=declared,
            highest=declared - 1,
            category=Category.CONSISTENCY,
            error_number="320",
        )


def _warn_unused_opportunities(conn: EthernetMultidropConnection) -> None:
    """A slot nobody uses costs its to_timer every cycle, which is a fair price for reserving room and worth saying out loud."""

    if conn.plca is None or not conn.nodes:
        return
    spare = conn.plca.transmit_opportunity_count - len(conn.participants)
    if spare > 0:
        warn(
            f"Multidrop connection '{conn.id}' declares {conn.plca.transmit_opportunity_count} transmit opportunities but hands out "
            f"{len(conn.participants)}. Every cycle then carries {spare} nobody uses, costing {spare * conn.plca.to_timer} bit times of "
            f"bandwidth - a legitimate price for reserving room for a node that joins later.",
            category=Category.CONSISTENCY,
            error_number="330",
        )


def _iter_internal_components(conn: EthernetMultidropConnection):
    """Yield ``(node, component)`` for what sits behind each port inside its own ECU: a switch port or a controller interface."""

    for node in conn.nodes:
        if node.ecu_port is None:
            continue
        component = node.ecu_port.get_internal_connected_component(None)
        if component is not None:
            yield node, component


def _warn_shaper_on_multidrop_port(conn: EthernetMultidropConnection) -> None:
    """Warn when an egress shaper leads onto a shared medium."""

    for _, component in _iter_internal_components(conn):
        if component.type != "switch_port":
            continue
        for traffic_class in component.traffic_classes or []:
            shaper = traffic_class.selection_mechanisms
            if shaper is None:
                continue
            warn(
                f"Multidrop connection '{conn.id}': switch port '{component.name}' carries traffic class '{traffic_class.name}' with a "
                f"{shaper.type.upper()} shaper, but leads onto a shared medium. The port transmits only inside its own transmit "
                f"opportunity, so the cycle, not the shaper, sets the achievable precision.",
                category=Category.CONSISTENCY,
                error_number="318",
            )


def _collect_gptp_transmitters(conn: EthernetMultidropConnection, transmitters_by_domain: Dict[int, List[str]]) -> None:
    """Collect each component's time transmitters by domain, raising on what a half duplex medium forbids."""

    for node, component in _iter_internal_components(conn):
        ptp_config = getattr(component, "ptp_config", None)
        if ptp_config is None:
            continue

        if ptp_config.cmlds_linkport_enabled:
            raise err_major(
                "Multidrop connection '{connection}': PTP config on '{source}' enables cmlds_linkport_enabled, which is not qualified "
                "for a half duplex link. Disable it on a port facing a shared medium.",
                connection=conn.id,
                source=component.name,
                category=Category.COMPATIBILITY,
                error_number="321",
            )

        for ptp_port in ptp_config.ptp_ports or []:
            sync_config = ptp_port.sync_config
            if sync_config.type != "time_transmitter":
                continue
            if sync_config.two_step is False:
                raise err_major(
                    "Multidrop connection '{connection}': time transmitter on '{source}' sets two_step false, but one step transport is "
                    "not qualified for a half duplex MAC.",
                    connection=conn.id,
                    source=component.name,
                    category=Category.COMPATIBILITY,
                    error_number="322",
                )
            transmitters_by_domain.setdefault(ptp_port.domain_id, []).append(f"{node.label} via {component.name}")


def _warn_shared_gptp_domains(conn: EthernetMultidropConnection, transmitters_by_domain: Dict[int, List[str]]) -> None:
    """Several time transmitters on one medium have to sit in different domains so their cycle masters do not collide."""

    for domain_id, transmitters in sorted(transmitters_by_domain.items()):
        if len(transmitters) > 1:
            warn(
                f"Multidrop connection '{conn.id}': {len(transmitters)} time transmitter ports share gPTP domain {domain_id} "
                f"({sorted(transmitters)}). Several time transmitters on one medium have to sit in different domains.",
                category=Category.CONSISTENCY,
                error_number="323",
            )


def _validate_gptp_on_segment(conn: EthernetMultidropConnection) -> None:
    """Enforce the gPTP constraints a half duplex shared medium imposes that no field default can express."""

    transmitters_by_domain: Dict[int, List[str]] = {}
    _collect_gptp_transmitters(conn, transmitters_by_domain)
    _warn_shared_gptp_domains(conn, transmitters_by_domain)


def _groups_of(node: EthernetMultidropNode) -> set:
    """State management groups the node's ECU belongs to, read off the ECU's resolved node-level memberships."""

    ecu = getattr(node.ecu_port, "_ecu", None) if node.ecu_port is not None else None
    members = getattr(ecu, "_state_effective_members", None) or []
    return {member.group for member in members}


def _warn_coordinator_missing_from_follower_group(conn: EthernetMultidropConnection) -> None:
    """Warn when a follower is in a state management group its coordinator is not."""

    coordinator = conn.coordinator
    if coordinator is None:
        return

    coordinator_groups = _groups_of(coordinator)
    for follower in conn.followers:
        for group_name in sorted(_groups_of(follower) - coordinator_groups):
            warn(
                f"Multidrop connection '{conn.id}': follower '{follower.label}' is in state management group '{group_name}' but its "
                f"coordinator '{coordinator.label}' is not, so the coordinator can be released while followers stay awake. The segment "
                f"then runs without BEACONs. Add the coordinator to the group unless that is deliberate.",
                category=Category.CONSISTENCY,
                error_number="337",
            )


def wire_multidrop_connections(connections: List[EthernetMultidropConnection]) -> None:
    """Join the ports on each segment to one another."""

    for conn in connections:
        ports = [node.ecu_port for node in conn.nodes if node.ecu_port is not None]
        for port in ports:
            # Identity, not equality: two ports can share a name across ECUs.
            already = {id(component) for component in port._connected_components}
            port._connected_components.extend(peer for peer in ports if peer is not port and id(peer) not in already)
