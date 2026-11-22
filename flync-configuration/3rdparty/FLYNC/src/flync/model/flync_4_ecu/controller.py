"""
Defines the Controller, EthernetInterfaceConfig, and EthernetInterface models for FLYNC.

``Controller`` declares three fields whose types live in modules that import this one —
``ComputeNode``, ``Switch``, and ``ControllerTopology``. They are therefore forward references
here, resolved by the wiring at the bottom of :mod:`flync.model.flync_4_ecu`.
"""

from typing import TYPE_CHECKING, Annotated, Any, List, Literal, Optional

from pydantic import (
    AfterValidator,
    BeforeValidator,
    Field,
    PrivateAttr,
    field_serializer,
    field_validator,
    model_validator,
)
from pydantic.networks import IPvAnyAddress

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes.macaddress import FLYNCMacAddress
from flync.core.utils.exceptions import Category, err_fatal, err_major, err_minor
from flync.core.validators.address import validate_multicast_list, validate_vlan_id
from flync.core.validators.connection_compatibility import (
    validate_ingress_streams_fields,
    validate_vlan_ids_unique,
)
from flync.core.validators.generic import (
    none_to_empty_list,
    validate_list_items_and_remove,
    validate_list_items_unique,
    validate_or_remove,
)
from flync.core.validators.traffic_classes import validate_traffic_classes
from flync.core.version_migrators.legacy_controller_check import (
    reject_legacy_controller,
)
from flync.model.flync_4_app.app_bindings import AppBindings
from flync.model.flync_4_ecu.can_interface import CANInterface
from flync.model.flync_4_ecu.controller_interface import ControllerInterface
from flync.model.flync_4_ecu.lin_interface import AnyLINInterface
from flync.model.flync_4_ecu.phy import MII, RGMII, RMII, SGMII, XFI
from flync.model.flync_4_ecu.router import RouteEntry, gateway_in_subnet
from flync.model.flync_4_ecu.socket_container import SocketContainer
from flync.model.flync_4_ecu.sockets import (
    IPv4AddressEndpoint,
    IPv6AddressEndpoint,
)
from flync.model.flync_4_metadata.metadata import EmbeddedMetadata
from flync.model.flync_4_nm import StateMembershipRef
from flync.model.flync_4_security import Firewall, MACsecConfig
from flync.model.flync_4_tsn import (
    HTBInstance,
    PTPConfig,
    Stream,
    TrafficClass,
)

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.compute_node import ComputeNode
    from flync.model.flync_4_ecu.controller_topology import ControllerTopology
    from flync.model.flync_4_ecu.switch import Switch

_PTPConfigField = Annotated[
    Optional[PTPConfig],
    BeforeValidator(validate_or_remove("PTP config", PTPConfig)),
]
_MACsecConfigField = Annotated[
    Optional[MACsecConfig],
    BeforeValidator(validate_or_remove("MACsec config", MACsecConfig)),
]
_FirewallField = Annotated[
    Optional[Firewall],
    BeforeValidator(validate_or_remove("firewall", Firewall)),
]
_HTBField = Annotated[
    Optional[HTBInstance],
    BeforeValidator(validate_or_remove("HTB config", HTBInstance)),
]
_IngressStreamsField = Annotated[
    Optional[List[Stream]],
    BeforeValidator(validate_or_remove("ingress streams", List[Stream])),
    BeforeValidator(none_to_empty_list),
]
_TrafficClassesField = Annotated[
    Optional[List[TrafficClass]],
    AfterValidator(validate_traffic_classes),
    BeforeValidator(validate_or_remove("traffic classes", List[TrafficClass])),
    BeforeValidator(none_to_empty_list),
]


class VirtualControllerInterface(FLYNCBaseModel):
    """
    A VLAN-tagged virtual interface stacked on top of a physical controller interface or a compute node.

    Each virtual interface represents one logical network endpoint, identified by a VLAN ID and assigned one or more IP addresses.
    Multiple virtual interfaces can be defined on the same physical interface or compute node to separate traffic across different VLANs.

    Parameters
    ----------
    name : str
        Name of the virtual interface.

    vlanid : int, optional
        VLAN identifier. Values 0-4094 are accepted; 4095 is reserved by IEEE 802.1Q and emits a warning when used. ``None`` denotes an
        untagged virtual interface.

    addresses : list of \
    :class:`~flync.model.flync_4_ecu.sockets.IPv4AddressEndpoint` or \
    :class:`~flync.model.flync_4_ecu.sockets.IPv6AddressEndpoint`
        Assigned IPv4 and IPv6 address endpoints.

    multicast : list of :class:`~pydantic.networks.IPvAnyAddress` or :class:`~MacAddress`, optional
        Allowed multicast addresses.
    """

    name: str = Field()
    vlanid: Annotated[
        Optional[int],
        AfterValidator(validate_vlan_id),
    ] = Field(default=None)
    addresses: List[IPv6AddressEndpoint | IPv4AddressEndpoint] = Field()
    multicast: Annotated[
        Optional[List[IPvAnyAddress | FLYNCMacAddress]],
        AfterValidator(validate_multicast_list),
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])

    @field_serializer("addresses", "multicast")
    def serialize_addresses(self, value):
        if value is not None:
            return [(v.model_dump() if isinstance(v, FLYNCBaseModel) else str(v).upper()) for v in value]


class EthernetInterfaceConfig(FLYNCBaseModel):
    """
    Configuration for an Ethernet interface.

    The same model backs two kinds of endpoint:

    * a **physical** interface on a :class:`Controller`, which carries a ``mii_config`` describing the
      media-independent interface behind it, and

    * a **virtual** interface on a :class:`~flync.model.flync_4_ecu.compute_node.ComputeNode`,
      which has no PHY and therefore must not declare a ``mii_config``.

    In both cases, virtual interfaces (VLANs) are stacked on the interface and carry its IP addresses.
    Bridging between interfaces — physical to virtual, or virtual to virtual — is expressed in the
    owning controller's ``controller_topology.flync.yaml``, not on the interface itself.

    Parameters
    ----------
    mac_address : :class:`MacAddress`, optional
        MAC address of the interface in standard notation.

    mii_config : :class:`~flync.model.flync_4_ecu.phy.MII` or :class:`~flync.model.flync_4_ecu.phy.RMII` or \
    :class:`~flync.model.flync_4_ecu.phy.SGMII` or :class:`~flync.model.flync_4_ecu.phy.RGMII`, optional
        Media-independent interface configuration. Physical interfaces only.

    virtual_interfaces : list of :class:`VirtualControllerInterface`, optional
        VLAN-tagged virtual interfaces stacked on this interface.

    ptp_config : :class:`~flync.model.flync_4_tsn.PTPConfig`, optional
        Precision Time Protocol configuration.

    macsec_config : :class:`~flync.model.flync_4_security.MACsecConfig`, optional
        MACsec configuration.

    firewall : :class:`~flync.model.flync_4_security.Firewall`, optional
        Firewall configuration for the interface.

    htb : :class:`~flync.model.flync_4_tsn.HTBInstance`, optional
        Hierarchical Token Bucket (HTB) egress shaping configuration.

    ingress_streams : list of :class:`~flync.model.flync_4_tsn.Stream`, optional
        IEEE 802.1Qci ingress stream policing configuration.

    traffic_classes : list of :class:`~flync.model.flync_4_tsn.TrafficClass`, optional
        Traffic class definitions and egress queue shaping configuration.

    routing_table : list of :class:`~flync.model.flync_4_ecu.router.RouteEntry`, optional
        Static routing table for forwarding between subnets.
        When provided, this interface acts as an IP router.
        Each entry maps a destination network to a ``default_gateway`` IP and an ``egress_interface`` (VCI name).

    """

    mac_address: Optional[FLYNCMacAddress] = Field(default=None)
    mii_config: Optional[MII | RMII | SGMII | RGMII | XFI] = Field(default=None, discriminator="type")
    virtual_interfaces: Annotated[
        Optional[List[VirtualControllerInterface]],
        BeforeValidator(
            validate_list_items_and_remove(
                "virtual interface",
                VirtualControllerInterface,
                severity="minor",
            )
        ),
    ] = Field(default_factory=list)
    ptp_config: _PTPConfigField = Field(default=None)
    macsec_config: _MACsecConfigField = Field(default=None)
    firewall: _FirewallField = Field(default=None)
    htb: _HTBField = Field(default=None)
    ingress_streams: _IngressStreamsField = Field(default=[])
    traffic_classes: _TrafficClassesField = Field(default_factory=list)
    routing_table: Annotated[
        Optional[List[RouteEntry]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    _name: Optional[str] = None

    @property
    def name(self):
        """Interface name, propagated from the parent :class:`EthernetInterface` (implied from the folder name)."""
        return self._name

    @field_validator("ingress_streams", mode="after")
    def validate_ingress_streams(cls, value):
        """Ensure no ingress stream carries an ipv or ats value."""
        return validate_ingress_streams_fields(value, "controller interface")

    @model_validator(mode="after")
    def validate_vlans(self):
        """Raise if any VLAN ID is repeated across virtual interfaces."""
        validate_vlan_ids_unique(self.virtual_interfaces, self.name)
        return self

    @model_validator(mode="after")
    def validate_routing_table_egress_interface(self):
        """
        Validate that every ``egress_interface`` in the routing table exists as a VCI on this interface.

        Raises:
            err_minor: An ``egress_interface`` is not a VCI of this interface.
        """
        if self.routing_table:
            vci_names = [vci.name for vci in self.virtual_interfaces or []]
            for route in self.routing_table:
                if route.egress_interface not in vci_names:
                    raise err_minor(
                        f"RouteEntry egress_interface {route.egress_interface} is not a virtual interface of the controller interface.",
                        category=Category.REFERENCE,
                        error_number="063",
                    )
        return self

    @model_validator(mode="after")
    def validate_routing_table_default_gateway(self):
        """
        Validate that ``default_gateway`` of each route falls within the subnet of its ``egress_interface`` VCI.

        Raises:
            err_minor: ``default_gateway`` is not within the subnet of the ``egress_interface`` VCI.
        """
        if self.routing_table:
            vci_map = {vci.name: vci for vci in self.virtual_interfaces or []}
            for route in self.routing_table:
                vci = vci_map.get(route.egress_interface)
                if vci is None:
                    continue
                if not gateway_in_subnet(route, vci):
                    raise err_minor(
                        f"RouteEntry default_gateway {route.default_gateway} is not within the subnet of egress_interface {route.egress_interface}.",
                        category=Category.CONSISTENCY,
                        error_number="064",
                    )
        return self

    def is_part_of_vlan(self, vlan):
        for vint in self.virtual_interfaces:
            if vint.vlanid == vlan:
                return True

        return False

    def get_all_ips(self):
        ips = []
        for viface in self.virtual_interfaces or []:
            for address in viface.addresses:
                ips.append(str(address.address))

        return ips

    def get_all_macs(self):
        macs = []
        if self.mac_address is not None:
            macs.append(self.mac_address)

        return macs


class EthernetInterface(ControllerInterface):
    """
    An Ethernet Interface of a Controller.

    Parameters
    ----------

    name : str
        Name of the ethernet interface, implied from the folder name on disk.

    interface_config : :class:`~EthernetInterfaceConfig`
        Configuration of the Ethernet interface.

    sockets : list of \
        :class:`~flync.model.flync_4_ecu.socket_container.SocketContainer`, optional
        Socket containers for this Ethernet interface.

    Private Attributes
    ------------------
    _connected_component :
        The switch port, controller interface, or ECU port connected to this interface. Managed internally; not part of the public API.
    _type :
        Fixed to ``"controller_interface"``.
    _controller :
        The :class:`Controller` that owns this interface. Managed internally.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FOLDER_NAME)] = Field()
    interface_config: Annotated[
        EthernetInterfaceConfig,
        External(
            output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field()
    sockets: Annotated[
        Optional[List[SocketContainer]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    _connected_component: List = []
    _type: Literal["controller_interface"] = PrivateAttr(default="controller_interface")
    # A Controller for a physical interface, a ComputeNode for a virtual one. Both expose
    # ``name`` and ``ethernet_interfaces``, which is all the accessors below need.
    _controller: Optional[Any] = PrivateAttr(default=None)

    @property
    def type(self):
        return self._type

    @property
    def connected_component(self):
        return self._connected_component

    @property
    def macsec_config(self):
        return self.interface_config.macsec_config

    @property
    def ptp_config(self):
        return self.interface_config.ptp_config

    def get_controller(self):
        """Return the owner of this interface — a :class:`Controller`, or a ``ComputeNode`` for a virtual interface."""
        if not self._controller:
            raise err_fatal("Fatal Error: The interface is not a part of any controller", category=Category.STRUCTURAL, error_number="065")
        return self._controller

    def get_connected_components(self):
        """Return the component connected to this interface."""
        return self._connected_component

    def is_part_of_vlan(self, vlan):
        return self.interface_config.is_part_of_vlan(vlan)

    def get_other_interfaces(self):
        """Return all EthernetInterfaces of the same owner — the controller, or the compute node, that declares this one."""
        return list(self.get_controller().ethernet_interfaces or [])


class Controller(FLYNCBaseModel):
    """
    Represents a controller device that contains multiple interfaces.

    Parameters
    ----------
    name : str
        Name of the controller.

    controller_metadata : :class:`~flync.model.flync_4_metadata.metadata.EmbeddedMetadata`
        Metadata describing the embedded controller.

    ethernet_interfaces : list of :class:`~EthernetInterface`, optional
        Ethernet interfaces of the controller.

    can_interfaces : list of :class:`~flync.model.flync_4_ecu.can_interface.CANInterface`, optional
        CAN bus interfaces of the controller.

    lin_interfaces : list of :class:`~flync.model.flync_4_ecu.lin_interface.AnyLINInterface`, optional
        LIN bus interfaces of the controller.

    compute_nodes : list of :class:`~flync.model.flync_4_ecu.compute_node.ComputeNode`, optional
        Compute nodes hosted by this controller. Compute nodes nest arbitrarily deep; names must be
        unique across the whole subtree. Stored in the ``compute_nodes/`` folder.

    switches : list of :class:`~flync.model.flync_4_ecu.switch.Switch`, optional
        Virtual switches hosted by this controller, modelled with the same class as a hardware
        switch. Stored in the ``switches/`` folder.

    controller_topology : :class:`~flync.model.flync_4_ecu.controller_topology.ControllerTopology`, optional
        Connectivity between the controller's virtual switches and the Ethernet interfaces of the
        controller and of its compute nodes. Stored in ``controller_topology.flync.yaml``.

    app_bindings : :class:`~flync.model.flync_4_app.AppBindings`, optional
        Applications a controller should bind to.

    state_memberships : list of :class:`~flync.model.flync_4_nm.StateMembershipRef`, optional
        Assignments of this controller to state management groups.
        Stored in ``state_memberships.flync.yaml`` inside the controller folder.

    Private Attributes
    ------------------
    _type : Literal["controller"]
        The type of the object generated. Defaults to ``"controller"``.
    """

    name: Annotated[
        str,
        Implied(
            strategy=ImpliedStrategy.FOLDER_NAME,
        ),
    ] = Field()
    controller_metadata: Annotated[
        EmbeddedMetadata,
        External(
            output_structure=OutputStrategy.SINGLE_FILE,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field()

    ethernet_interfaces: Annotated[
        Optional[List[EthernetInterface]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    can_interfaces: Annotated[
        Optional[List[CANInterface]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    lin_interfaces: Annotated[
        Optional[List[AnyLINInterface]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    compute_nodes: Annotated[
        Optional[List["ComputeNode"]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    switches: Annotated[
        Optional[List["Switch"]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    controller_topology: Annotated[
        Optional["ControllerTopology"],
        External(
            output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default=None)
    app_bindings: Annotated[Optional[AppBindings], External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT)] = Field(
        default=None, description="Applications a controller should bind to."
    )

    state_memberships: Annotated[
        Optional[List[StateMembershipRef]],
        External(output_structure=OutputStrategy.SINGLE_FILE),
        BeforeValidator(validate_or_remove("state memberships", List[StateMembershipRef])),
        BeforeValidator(none_to_empty_list),
    ] = Field(
        default=[],
        description="Assignments of this controller to state management groups.",
    )
    _type: Literal["controller"] = PrivateAttr(default="controller")

    @model_validator(mode="before")
    @classmethod
    def reject_legacy_controller_layout(cls, data):
        reject_legacy_controller(data)
        return data

    def _interfaces_by_kind(self):
        return (
            ("ethernet", self.ethernet_interfaces or []),
            ("CAN", self.can_interfaces or []),
            ("LIN", self.lin_interfaces or []),
        )

    @model_validator(mode="after")
    def require_at_least_one_interface(self):
        if not any(interfaces for _kind, interfaces in self._interfaces_by_kind()):
            raise err_major(
                "Controller must declare at least one interface (Ethernet, CAN, or LIN).",
                category=Category.REQUIRED,
                error_number="066",
            )
        return self

    @model_validator(mode="after")
    def validate_unique_interface_names(self):
        """Validate that this controller's own Ethernet interface names are unique."""
        validate_list_items_unique(
            [eth.name for eth in self.ethernet_interfaces or [] if eth.interface_config],
            "Controller Interfaces (name)",
        )
        return self

    @model_validator(mode="after")
    def validate_unique_compute_node_names(self):
        """Validate that compute node names are unique across the whole controller subtree."""
        validate_list_items_unique(
            [node.name for node in self.iter_subtree_compute_nodes()],
            "Compute Nodes (name)",
        )
        return self

    @model_validator(mode="after")
    def validate_unique_virtual_switch_names(self):
        """
        Validate that virtual switch names are unique across the whole controller subtree.

        Virtual switch names need not differ from the ECU's hardware switch names — the two
        namespaces never meet, since no controller topology connection can reach a hardware switch.
        """
        validate_list_items_unique(
            [switch.name for switch in self.iter_subtree_switches()],
            "Virtual Switches (name)",
        )
        return self

    @model_validator(mode="after")
    def validate_unique_interface_names_across_types(self):
        """
        Validate that interface names are unique across all interface types.

        A name that is already unique within a single type is left to :meth:`validate_unique_interface_names`;
        this check only flags names reused by more than one interface type, which would create ambiguous references.
        """
        name_types: dict[str, set] = {}
        for label, interfaces in self._interfaces_by_kind():
            for iface in interfaces:
                name_types.setdefault(iface.name, set()).add(label)

        for name, types in name_types.items():
            if len(types) > 1:
                raise err_major(
                    "Interface name '{name}' is used by more than one interface type: {types}",
                    category=Category.UNIQUENESS,
                    error_number="248",
                    name=name,
                    types=", ".join(sorted(types)),
                )
        return self

    @model_validator(mode="after")
    def validate_unique_ethernet_mac_addresses(self):
        """Validate that Ethernet interfaces across this controller's whole subtree use distinct MAC addresses."""
        seen: set = set()
        for eth in self.iter_subtree_interfaces():
            mac = eth.interface_config.mac_address if eth.interface_config is not None else None
            if mac is None:
                continue
            if mac in seen:
                raise err_major(
                    "MAC address '{mac}' is used by more than one Ethernet interface on controller '{controller}'",
                    category=Category.UNIQUENESS,
                    error_number="249",
                    mac=mac,
                    controller=self.name,
                )
            seen.add(mac)
        return self

    @model_validator(mode="after")
    def resolve_controller_topology_connections(self):
        """
        Bind and compatibility-check the controller's internal connections.
        """

        if self.controller_topology is None:
            return self

        connections = [conn_union.root for conn_union in self.controller_topology.connections]
        switches = list(self.iter_subtree_switches())
        owners: list = [self, *self.iter_subtree_compute_nodes()]

        for conn in connections:
            conn.bind(switches, owners, [])
        for conn in connections:
            conn.validate_compatibility()
        return self

    def iter_subtree_compute_nodes(self):
        """Yield every compute node beneath this controller, at any nesting depth."""
        from flync.model.flync_4_ecu.compute_node import iter_subtree_compute_nodes

        return iter_subtree_compute_nodes(self)

    def iter_subtree_switches(self):
        """Yield every virtual switch beneath this controller, including those inside nested compute nodes."""
        from flync.model.flync_4_ecu.compute_node import iter_subtree_switches

        return iter_subtree_switches(self)

    def iter_subtree_interfaces(self):
        """
        Yield every Ethernet interface beneath this controller, physical and virtual alike.

        Use :meth:`get_interfaces` instead wherever only the physical, ECU-visible interfaces are
        meant — the ECU's internal topology must never reach a compute node's interface.
        """
        from flync.model.flync_4_ecu.compute_node import iter_subtree_interfaces

        return iter_subtree_interfaces(self)

    def get_all_ips(self):
        """
        Helper function.
        Return all the IPs in the Controller, including those of its compute nodes.
        """

        all_ips = []
        for eth_iface in self.iter_subtree_interfaces():
            all_ips.extend(eth_iface.interface_config.get_all_ips())
        return all_ips

    def get_all_macs(self):
        """
        Helper function.
        Return all the MAC addresses in the Controller, including those of its compute nodes.
        """

        all_macs = []
        for eth_iface in self.iter_subtree_interfaces():
            all_macs.extend(eth_iface.interface_config.get_all_macs())
        return all_macs

    def get_consumed_service_instances(self) -> set:
        """
        Helper function.
        Return the ``(service, major_version, instance_id)`` triples this Controller deploys as a SOME/IP consumer.

        Covers the whole subtree: a service consumed by a compute node is consumed by the
        controller hosting it.
        """

        return {
            (dep.root.service, dep.root.major_version, dep.root.instance_id)
            for eth_iface in self.iter_subtree_interfaces()
            for sock_con in eth_iface.sockets or []
            for socket in sock_con.sockets or []
            for dep in socket.deployments or []
            if dep.root.deployment_type == "someip_consumer"
        }

    def get_interfaces(self) -> list["EthernetInterface"]:
        """
        Return this controller's own physical Ethernet interfaces.

        Deliberately excludes compute node interfaces: this is what the ECU's internal topology
        resolves against, and only a physical interface may cross the controller boundary. Use
        :meth:`iter_subtree_interfaces` for the recursive view.
        """
        return list(self.ethernet_interfaces or [])

    def find_controller_interface(self, interface_name: str) -> EthernetInterfaceConfig:
        return next(i.interface_config for i in (self.ethernet_interfaces or []) if i.name == interface_name)

    def model_post_init(self, __context):
        for interface in self.ethernet_interfaces or []:
            if interface.interface_config is not None:
                interface._controller = self
                interface.interface_config._name = interface.name
        return super().model_post_init(__context)
