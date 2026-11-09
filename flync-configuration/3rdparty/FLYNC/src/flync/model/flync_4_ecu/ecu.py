"""Defines the ECU model for FLYNC."""

from typing import Annotated, Iterator, List, Optional, TypeVar

from pydantic import BeforeValidator, Field, model_validator

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.base_utils import find_all
from flync.core.utils.exceptions import Category, err_major, err_minor, warn
from flync.core.validators.generic import none_to_empty_list, validate_or_remove
from flync.model.flync_4_ecu.controller import (
    Controller,
    EthernetInterface,
)
from flync.model.flync_4_ecu.internal_topology import (
    InternalTopology,
    SwitchPortToControllerInterface,
    SwitchPortToXConnection,
)
from flync.model.flync_4_ecu.mac_multicast_endpoint import (
    MACMulticastEndpoints,
)
from flync.model.flync_4_ecu.multicast_groups import MulticastGroupMembership
from flync.model.flync_4_ecu.port import ECUPort
from flync.model.flync_4_ecu.socket_container import SocketContainer
from flync.model.flync_4_ecu.sockets import Socket
from flync.model.flync_4_ecu.switch import Switch, SwitchPort
from flync.model.flync_4_metadata import ECUMetadata
from flync.model.flync_4_nm import StateMembershipRef
from flync.model.flync_4_nm.state_management import EffectiveMember
from flync.model.flync_4_someip import (  # type: ignore  # noqa: F401
    SOMEIPServiceConsumer,
    SOMEIPServiceDeployment,
    SOMEIPServiceProvider,
)

_T_Service = TypeVar("_T_Service", bound=SOMEIPServiceDeployment)


class ECU(FLYNCBaseModel):
    """
    Represents an Electronic Control Unit (ECU) in the network.

    Parameters
    ----------
    name : str
        Name of the ECU.

    ports : list of :class:`~flync.model.flync_4_ecu.port.ECUPort`, optional
        List of physical Ethernet ECU ports. May be empty for a CAN/LIN-only ECU, but at least one port is required
        as soon as the ECU declares any Ethernet interface or switch.

    controllers : list of :class:`~flync.model.flync_4_ecu.controller.Controller`
        Controllers associated with this ECU.

    switches : list of :class:`~flync.model.flync_4_ecu.switch.Switch`, optional
        Switches integrated within the ECU. If not provided, the ECU contains no internal switches.

    topology : :class:`~flync.model.flync_4_ecu.internal_topology.InternalTopology`, optional
        Internal topology defining the connectivity between ECU components. May be omitted for a CAN/LIN-only ECU,
        but is required as soon as the ECU declares any Ethernet interface or switch.

    multicast_groups : list of :class:`~flync.model.flync_4_ecu.multicast_groups.MulticastGroupMembership`, optional
        Multicast group memberships of the ECU. This field is populated automatically internally.

    mac_multicast_endpoints : :class:`~flync.model.flync_4_ecu.mac_multicast_endpoint.MACMulticastEndpoints`, optional
        MAC multicast endpoints configuration for the ECU.

    ecu_metadata : :class:`~flync.model.flync_4_metadata.metadata.ECUMetadata`
        Metadata information describing the ECU.

    state_memberships : list of \
    :class:`~flync.model.flync_4_nm.StateMembershipRef`, optional
        Assignments of this ECU to state management groups (whole-ECU granularity).
        Stored in ``state_memberships.flync.yaml`` inside the ECU folder.
    """

    name: Annotated[
        str,
        Implied(
            strategy=ImpliedStrategy.FOLDER_NAME,
        ),
    ] = Field()
    ports: Annotated[
        Optional[List["ECUPort"]],
        External(
            output_structure=OutputStrategy.SINGLE_FILE,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    controllers: Annotated[
        List["Controller"],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field()
    switches: Annotated[
        Optional[List["Switch"]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    topology: Annotated[
        Optional["InternalTopology"],
        External(
            output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
        BeforeValidator(validate_or_remove("internal topology", InternalTopology, severity="major")),
    ] = Field(default=None)
    ecu_metadata: Annotated[
        "ECUMetadata",
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field()
    mac_multicast_endpoints: Annotated[
        Optional["MACMulticastEndpoints"],
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(exclude=True, default=None)
    multicast_groups: Optional[List[MulticastGroupMembership]] = Field(default_factory=list, exclude=True)
    state_memberships: Annotated[
        Optional[List[StateMembershipRef]],
        External(output_structure=OutputStrategy.SINGLE_FILE),
        BeforeValidator(validate_or_remove("state memberships", List[StateMembershipRef])),
        BeforeValidator(none_to_empty_list),
    ] = Field(
        default=[],
        description="Assignments of this ECU to state management groups (whole-ECU granularity).",
    )
    _state_effective_members: List[EffectiveMember] = []

    _connectivity_check_done: bool = False

    def model_post_init(self, context):
        """
        Perform post-initialization processing after the model is created.

        Following steps are performed:
        1. Reference the ECU in child components to allow access to ECU-level information.

        2. Bind sockets (defined per ethernet interface) to their corresponding IP addresses in the virtual interfaces, ensuring each socket is
           associated with the correct ECU IP.

        3. Populate multicast group memberships based on socket configurations and virtual interface settings.
        """

        self.__reference_ecu_in_children()
        self.__bind_sockets_to_ip()
        self.__populate_multicast_tx_groups_from_socket()
        self.__populate_multicast_rx_groups_from_interfaces()
        self._populate_multicast_tx_groups_from_mac_multicast_endpoints()
        self.__populate_state_memberships()

    @model_validator(mode="before")
    @classmethod
    def skip_on_broken_controllers(cls, data):
        """
        Skip ECU validation when controllers or switches failed to load.

        When a controller or switch cannot be parsed, the workspace places None in the respective list.
        Attempting to validate the ECU in that state produces a cascade of unhelpful errors,
        instead report a single major error so that per-component errors remain the focus.
        """

        if isinstance(data, dict):
            controllers = data.get("controllers") or []
            switches = data.get("switches") or []
            broken_controllers = isinstance(controllers, list) and any(c is None for c in controllers)
            broken_switches = isinstance(switches, list) and any(s is None for s in switches)
            if broken_controllers or broken_switches:
                raise err_major(
                    "ECU has invalid components. Check controller and switch errors for details.", category=Category.STRUCTURAL, error_number="068"
                )
        return data

    @model_validator(mode="after")
    def validate_ethernet_hw_requires_ports_and_topology(self):
        """Ethernet interfaces and switches only make sense wired to physical ports through an internal topology."""

        has_ethernet_hw = bool(self.switches) or any(c.ethernet_interfaces for c in self.controllers)
        if has_ethernet_hw and not self.ports:
            raise err_major(
                f"ECU '{self.name}' declares Ethernet interfaces or switches but has no ECU ports defined (ports.flync.yaml).",
                category=Category.REQUIRED,
                error_number="227",
            )
        if has_ethernet_hw and self.topology is None:
            raise err_major(
                f"ECU '{self.name}' declares Ethernet interfaces or switches but has no internal topology (topology.flync.yaml).",
                category=Category.REQUIRED,
                error_number="228",
            )
        return self

    @model_validator(mode="after")
    def resolve_topology_connections(self):
        connections = [conn_union.root for conn_union in self.topology.connections] if self.topology else []

        for conn in connections:
            conn.bind(self.switches or [], self.controllers, self.ports or [])

        self.__validate_switch_port_connections(connections)
        self.__validate_single_physical_connection_per_interface(connections)

        for conn in connections:
            conn.validate_compatibility()
        return self

    def __validate_switch_port_connections(self, connections):
        """Validate that switch ports are not self-connected and not connected to more than one component."""
        seen_switch_ports: set[int] = set()
        for conn in connections:
            if not isinstance(conn, SwitchPortToXConnection):
                continue
            switch_ports = conn.get_switch_port_refs()
            if len(switch_ports) == 2 and switch_ports[0] is switch_ports[1]:
                sp = switch_ports[0]
                raise err_major(
                    f"switch port '{sp.name}' on switch '{sp.get_switch().name}' is connected to itself. "
                    f"A component cannot be connected to itself.",
                    category=Category.COMPATIBILITY,
                    error_number="209",
                )
            for switch_port in switch_ports:
                if id(switch_port) in seen_switch_ports:
                    raise err_major(
                        f"switch port '{switch_port.name}' on switch '{switch_port.get_switch().name}' is connected to more than one component. "
                        f"Each switch port in the internal topology may only be connected to a single other component.",
                        category=Category.COMPATIBILITY,
                        error_number="210",
                    )
                seen_switch_ports.add(id(switch_port))

    def __validate_single_physical_connection_per_interface(self, connections):
        """Raise if the same physical controller interface is connected to more than one switch port."""
        seen: dict[tuple, str] = {}
        for conn in connections:
            if not isinstance(conn, SwitchPortToControllerInterface):
                continue
            key = (conn.controller_name, conn.iface_name)
            if key in seen:
                raise err_major(
                    "Physical controller interface '{iface}' is connected more than once in the internal topology",
                    category=Category.UNIQUENESS,
                    error_number="250",
                    iface=conn.iface_name,
                )
            seen[key] = conn.id

    @model_validator(mode="after")
    def validate_no_unconnected_components(self):
        if self._connectivity_check_done:
            return self
        self._connectivity_check_done = True

        self.__warn_unconnected_ports()
        self.__warn_unconnected_switch_ports()
        self.__warn_unconnected_controller_interfaces()
        self.__warn_unconnected_host_controller_interfaces()

        return self

    def __warn_unconnected_ports(self):
        """Warn about ECU ports that are not connected in the internal topology."""
        for port in self.ports or []:
            if not port._connected_components:
                warn(
                    f"ECU port '{port.name}' is not connected in the internal topology.",
                    category=Category.STRUCTURAL,
                    error_number="211",
                )

    def __warn_unconnected_switch_ports(self):
        """Warn about switch ports that are not connected in the internal topology."""
        for switch_port in self.get_all_switch_ports():
            if switch_port._connected_component is None:
                warn(
                    f"Switch port '{switch_port.name}' (switch: '{switch_port._switch.name}') is not connected in the internal topology.",
                    category=Category.STRUCTURAL,
                    error_number="212",
                )

    def __warn_unconnected_controller_interfaces(self):
        """Warn about controller interfaces that are not connected in the internal topology."""
        for iface in self.get_all_interfaces():
            if not iface._connected_component:
                warn(
                    f"Controller interface '{iface.name}' (controller: '{iface._controller.name}') is not connected in the internal topology.",
                    category=Category.STRUCTURAL,
                    error_number="213",
                )

    def __warn_unconnected_host_controller_interfaces(self):
        """Warn about switch host-controller interfaces that are not connected to their switch's CPU port."""
        for switch in self.switches or []:
            if switch.host_controller is None:
                continue
            for iface in switch.host_controller.get_interfaces():
                if not iface._connected_component:
                    warn(
                        f"Host controller interface '{iface.name}' (switch: '{switch.name}') is not connected to its switch's CPU port "
                        "in the internal topology.",
                        category=Category.STRUCTURAL,
                        error_number="238",
                    )

    @model_validator(mode="after")
    def validate_vlans_in_sockets(self):
        """
        Validate that the VLAN IDs specified in the socket containers of each ethernet interface are configured in a virtual interface of that same
        ethernet interface."""

        for controller in self.controllers:
            for eth_iface in controller.iter_subtree_interfaces():
                iface_config = eth_iface.interface_config
                vlan_ids_in_sockets = {sc.vlan_id for sc in (eth_iface.sockets or [])}
                if not vlan_ids_in_sockets:
                    continue
                vlan_ids_in_interface = {vi.vlanid for vi in iface_config.virtual_interfaces or []}
                missing_vlans = vlan_ids_in_sockets - vlan_ids_in_interface
                if missing_vlans:
                    raise err_minor(
                        "Error in socket configuration:\n"
                        "The following VLAN IDs are specified in the socket containers "
                        "but not configured in any virtual interface of ethernet interface "
                        f"{iface_config.name}: {missing_vlans}.",
                        category=Category.REFERENCE,
                        error_number="069",
                    )
        return self

    def __bind_sockets_to_ip(self):
        """
        Associate each socket with the matching IP address on the same ethernet interface where the socket is defined.

        Sockets are scoped to the ethernet interface that owns them, so a socket is only bound to addresses belonging to that interface — not
        to identically-addressed interfaces elsewhere in the ECU.

        Raises:
            err_minor: If a socket's endpoint address does not belong to any virtual interface of the ethernet interface that defines the socket.
        """

        for eth_iface in self.__get_all_ethernet_interfaces():
            self.__bind_iface_sockets_to_ip(eth_iface)
        return self

    def __get_all_ethernet_interfaces(self):
        """Return all ethernet interfaces of all controllers of the ECU, including those of their compute nodes."""
        return [eth_iface for controller in self.controllers for eth_iface in controller.iter_subtree_interfaces()]

    def __bind_iface_sockets_to_ip(self, eth_iface):
        """Bind every socket defined on ``eth_iface`` to its matching IP address on that same interface."""
        iface_config = eth_iface.interface_config
        iface_ips = set(iface_config.get_all_ips())
        for socket_container in eth_iface.sockets or []:
            for socket in socket_container.sockets or []:
                if socket.endpoint_type != "multicast":
                    self.__bind_socket_to_ip(socket, iface_config, iface_ips)

    def __bind_socket_to_ip(self, socket, iface_config, iface_ips):
        """Bind a single unicast socket to the virtual-interface IP address matching its endpoint address."""
        endpoint_ip = str(socket.endpoint_address)
        if endpoint_ip not in iface_ips:
            raise err_minor(
                f"Error in socket {socket.name}:\n"
                f"The IP {endpoint_ip} is not configured in any virtual interface of ethernet "
                f"interface {iface_config.name} in ECU {self.name}.",
                category=Category.REFERENCE,
                error_number="070",
            )
        for vi in iface_config.virtual_interfaces:
            for ip in vi.addresses:
                if ip.address == socket.endpoint_address:
                    ip.sockets.append(socket)

    def __reference_ecu_in_children(self):
        """
        allows the children attributes to access ._ecu
        """

        [setattr(p, "_ecu", self) for p in self.ports or []]  # noqa
        [setattr(c, "_ecu", self) for c in (self.topology.connections if self.topology else [])]  # noqa
        return self

    def __populate_multicast_tx_groups_from_socket(self):
        """
        Add Multicast TX entries from sockets (defined per ethernet interface) to multicast group memberships.
        """

        for eth_iface in self.__get_all_ethernet_interfaces():
            for socket_container in eth_iface.sockets or []:
                for socket in socket_container.sockets:
                    if socket.endpoint_type == "multicast":
                        self.__add_multicast_tx_groups_for_socket(socket, socket_container)
        return self

    def __add_multicast_tx_groups_for_socket(self, socket, socket_container):
        """Create and register one multicast TX group membership per multicast address of ``socket``."""
        interface = self.get_interface_for_ip(str(socket.endpoint_address))
        for multicast_addr in socket.multicast_tx:
            group = MulticastGroupMembership(
                group=multicast_addr,
                description=socket.name,
                mode="tx",
                vlan=socket_container.vlan_id,
                src_ip=socket.endpoint_address,
            )
            group._interface = interface
            self.multicast_groups.append(group)

    def __populate_multicast_rx_groups_from_interfaces(self):
        """
        Add Multicast RX entries from virtual interfaces to multicast group memberships.
        """

        for interface in find_all(self.controllers, EthernetInterface):
            for viface in interface.interface_config.virtual_interfaces:
                for multicast_addr in viface.multicast:
                    group = MulticastGroupMembership(
                        group=multicast_addr,
                        description="",
                        mode="rx",
                        vlan=viface.vlanid,
                        src_ip=None,
                    )
                    group._interface = interface
                    self.multicast_groups.append(group)
        return self

    def _populate_multicast_tx_groups_from_mac_multicast_endpoints(self):
        """
        Add Multicast TX entries from MAC multicast endpoints to multicast group memberships.
        """

        if self.mac_multicast_endpoints is not None:
            for endpoint_union in self.mac_multicast_endpoints.endpoints:
                endpoint = endpoint_union.root
                for multicast_addr in endpoint.multicast_tx:
                    group = MulticastGroupMembership(
                        group=multicast_addr,
                        description=endpoint.name,
                        mode="tx",
                        vlan=endpoint.vlan_id or None,
                        src_ip=None,
                    )
                    interface = self.get_interface_for_mac(str(endpoint.mac_address))
                    if not interface:
                        raise err_minor(
                            f"Error in MAC multicast:\n"
                            f"endpoints. Could not find an interface for address {endpoint.mac_address}. ECU {self.name}",
                            category=Category.REFERENCE,
                            error_number="071",
                        )
                    group._interface = interface
                    self.multicast_groups.append(group)
        return self

    def __populate_state_memberships(self):
        """Resolve this ECU's and its controllers' state_memberships into EffectiveMember entries."""
        for ref in self.state_memberships or []:
            self.__add_effective_state_member(ref, "ecu", self.name, self.name)
        for ctrl in self.controllers:
            for ref in ctrl.state_memberships or []:
                self.__add_effective_state_member(ref, "controller", f"{self.name}/{ctrl.name}", self.name)
        return self

    def __add_effective_state_member(self, ref, kind, path, ecu_name):
        """Append one or more EffectiveMember entries for a single state_membership ref."""
        if ref.role == "observer":
            self._state_effective_members.append(EffectiveMember(ref.group, kind, path, ecu_name, ref.role, None))
            return
        bits = ref.relevance_bits or [path.rsplit("/", 1)[-1]]
        for bit in bits:
            self._state_effective_members.append(EffectiveMember(ref.group, kind, path, ecu_name, ref.role, bit))

    def get_all_controllers(self):
        """Return a list of all controllers of the ECU."""
        return self.controllers

    def iter_controllers_and_switch_hosts(self):
        """Yield the ECU's own controllers, then the host controller inside each switch."""

        yield from self.controllers
        for switch in self.switches or []:
            if switch.host_controller is not None:
                yield switch.host_controller

    def get_all_ports(self):
        """Return a list of all ports of the ECU."""
        return self.ports or []

    def get_all_switches(self):
        """Return a list of all switches of the ECU."""
        return self.switches

    def get_internal_topology(self):
        """Return the ECU's internal topology, or ``None`` if none is defined."""
        return self.topology

    def get_all_interfaces(self):
        """Return a list of all Ethernet interfaces of the ECU's controllers, compute node interfaces included."""

        return [i for c in self.controllers for i in c.iter_subtree_interfaces()]

    def get_all_switch_ports(self) -> List["SwitchPort"]:
        """Return a list of all ports of the ECU switch."""
        ports = []
        if self.switches is not None:
            for switch in self.switches:
                for port in switch.ports:
                    if port:
                        ports.append(port)
        return ports

    def get_switch_by_name(self, switch_name: str):
        """Retrieve a Switch of the ECU by name."""
        if self.switches is not None:
            for switch in self.switches:
                if switch.name == switch_name:
                    return switch
        return None  # Return None if not found

    def get_all_ips(self):
        """
        Get all IPs in a ECU
        """

        return [ip for ctrl in self.iter_controllers_and_switch_hosts() for ip in ctrl.get_all_ips()]

    def get_all_macs(self):
        """
        Get all MAC addresses in a ECU
        """

        return [mac for ctrl in self.iter_controllers_and_switch_hosts() for mac in ctrl.get_all_macs()]

    def __iter_socket_containers(self) -> Iterator[SocketContainer]:
        """
        Yield every socket container across all ethernet interfaces of the ECU.

        Yields
        ------
        SocketContainer
            Each socket container of each ethernet interface of each controller of the ECU.
        """

        for controller in self.controllers:
            for eth_iface in controller.iter_subtree_interfaces():
                yield from eth_iface.sockets or []

    def __iter_sockets(self) -> Iterator[Socket]:
        """
        Yield every socket across all socket containers of the ECU.

        Yields
        ------
        Socket
            Each socket of each socket container of the ECU.
        """

        for socket_container in self.__iter_socket_containers():
            yield from socket_container.sockets or []

    def get_all_sockets(self) -> dict[int | None, List[Socket]]:
        """
        Get all sockets across all ethernet interfaces of the ECU, grouped by VLAN ID.
        """

        all_socket_containers = list(self.__iter_socket_containers())
        return {
            vlan_id: [
                socket
                for socket_container in all_socket_containers
                if socket_container.vlan_id == vlan_id
                for socket in socket_container.sockets or []
            ]
            for vlan_id in {socket_container.vlan_id for socket_container in all_socket_containers}
        }

    def get_interface_for_ip(self, ip):
        for iface in self.get_all_interfaces():
            if ip in iface.interface_config.get_all_ips():
                return iface

    def get_interface_for_mac(self, mac):
        for iface in self.get_all_interfaces():
            macs = [iface.interface_config.mac_address]
            if mac in macs:
                return iface

    def __get_services_of_type(self, service_type: type[_T_Service]) -> list[_T_Service]:
        """
        Return all SOME/IP service deployments of a given type.

        Iterates over all ethernet interfaces, their socket containers and sockets, collecting deployments whose root matches ``service_type``.

        Parameters
        ----------
        service_type : type[SOMEIPServiceDeployment]
            The concrete deployment type to filter by (e.g. :class:`~flync.model.flync_4_someip.SOMEIPServiceConsumer` or
            :class:`~flync.model.flync_4_someip.SOMEIPServiceProvider`).

        Returns
        -------
        list[SOMEIPServiceDeployment]
            All matching service deployment instances found across the ECU's socket containers.
        """

        service_instances: list[_T_Service] = []
        for socket in self.__iter_sockets():
            for deployment in socket.deployments or []:
                if isinstance(deployment.root, service_type):
                    service_instances.append(deployment.root)
        return service_instances

    def get_consumed_services(self) -> list[SOMEIPServiceConsumer]:
        """
        Return all SOME/IP service consumer deployments of the ECU.

        Returns
        -------
        list[SOMEIPServiceConsumer]
            All :class:`~flync.model.flync_4_someip.SOMEIPServiceConsumer` instances found across the ECU's socket containers.
        """

        return self.__get_services_of_type(SOMEIPServiceConsumer)

    def get_provided_services(self) -> list[SOMEIPServiceProvider]:
        """
        Return all SOME/IP service provider deployments of the ECU.

        Returns
        -------
        list[SOMEIPServiceProvider]
            All :class:`~flync.model.flync_4_someip.SOMEIPServiceProvider` instances found across the ECU's socket containers.
        """

        return self.__get_services_of_type(SOMEIPServiceProvider)

    @model_validator(mode="after")
    def validate_unique_someip_service_instances(self) -> "ECU":
        """
        Flag repeated ``(protocol, endpoint_type, service, major_version, instance_id)`` deployments across the
        ECU's sockets.

        Protocol and endpoint type are part of the identity: offering the same instance over both UDP and TCP is
        a normal dual-transport setup, and a UDP consumer commonly splits across a unicast socket (regular
        eventgroups) and a separate multicast socket (multicast-only eventgroups) - neither is a conflict.

        Repeating the same instance on the very same transport/endpoint combination is warned about for both
        roles: server and client. Providing the same Service Instance twice may become an error in the future.
        """

        seen_consumers: set = set()
        seen_providers: set = set()
        for deployment, key in self.iter_someip_deployment_identities():
            if isinstance(deployment, SOMEIPServiceProvider):
                if key in seen_providers:
                    # Only a warning while FLYNC has no variant handling. This becomes an
                    # error once variants can express that two of those never coexist.
                    warn(
                        self.__duplicate_deployment_message("provider", deployment),
                        category=Category.UNIQUENESS,
                        error_number="243",
                    )
                seen_providers.add(key)
            elif key in seen_consumers:
                warn(
                    self.__duplicate_deployment_message("consumer", deployment),
                    category=Category.UNIQUENESS,
                    error_number="241",
                )
            else:
                seen_consumers.add(key)
        return self

    def iter_someip_deployment_identities(self) -> Iterator[tuple]:
        """
        Yield ``(deployment, identity)`` for every SOME/IP provider or consumer deployment across the ECU's sockets.

        The identity is the ``(protocol, endpoint_type, service, major_version, instance_id)`` tuple that decides
        whether two deployments describe the very same service instance on the very same transport.
        """

        for socket in self.__iter_sockets():
            for dep_root in socket.deployments or []:
                deployment = dep_root.root
                if isinstance(deployment, (SOMEIPServiceConsumer, SOMEIPServiceProvider)):
                    identity = (
                        socket.protocol,  # type: ignore[attr-defined]
                        socket.endpoint_type,
                        deployment.service,
                        deployment.major_version,
                        deployment.instance_id,
                    )
                    yield deployment, identity

    def __duplicate_deployment_message(self, role: str, deployment: SOMEIPServiceDeployment) -> str:
        """Return the message naming the service instance *deployment* repeats in the given *role*."""

        return (
            f"ECU '{self.name}': duplicate {role} deployment for service instance "
            f"(service={deployment.service:#06x}, major_version={deployment.major_version}, "
            f"instance_id={deployment.instance_id})"
        )
