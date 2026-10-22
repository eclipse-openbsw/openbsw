"""Defines the internal connection types between ECU components such as ECU ports, switch ports, and controller interfaces."""

from typing import TYPE_CHECKING, Annotated, List, Literal, Optional

from pydantic import Field, PrivateAttr, RootModel

from flync.core.annotations.reference import Reference
from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.connection_compatibility import (
    validate_cbs_idleslopes_fit_portspeed,
    validate_compulsory_mii_config_compatibility,
    validate_gptp,
    validate_htb,
    validate_macsec,
    validate_optional_mii_config_compatibility,
)
from flync.model.flync_4_ecu.controller import Controller, EthernetInterface
from flync.model.flync_4_ecu.port import ECUPort
from flync.model.flync_4_ecu.switch import Switch, SwitchPort

if TYPE_CHECKING:
    from flync.model.flync_4_ecu.ecu import ECU


class InternalConnection(FLYNCBaseModel):
    """
    Represents a base internal connection between two ECU components.

    Parameters
    ----------
    type : str
        The type of the connection.

    id : str
        Unique identifier for the connection.

    Private Attributes
    ------------------
    _ecu : :class:`~flync.model.flync_4_ecu.ecu.ECU`
        The ECU object to which this connection belongs.
        Managed internally.
    """

    type: str = Field()
    id: str = Field()
    _ecu: Optional["ECU"] = PrivateAttr(default=None)

    @property
    def ecu(self) -> Optional["ECU"]:
        return self._ecu

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        """Resolve all component references and wire connected-component back-refs. Called by ECU after all components are loaded."""

    def validate_compatibility(self) -> None:
        """Run compatibility checks on the already-bound components. Called by ECU after bind()."""


class ECUPortToXConnection(InternalConnection):
    """
    Base class for connections from an ECU port to another ECU component.

    Parameters
    ----------
    ecu_port_name : str
        Name of the ECU port (alias: ``ecu_port``).

    Private Attributes
    ------------------
    _ecu_port : :class:`~flync.model.flync_4_ecu.port.ECUPort`
        Internal reference to the actual ECU port object.
        Managed privately.
    """

    ecu_port_name: Annotated[str, Reference(source="_ecu_port")] = Field(alias="ecu_port")
    _ecu_port: Optional["ECUPort"] = PrivateAttr(default=None)

    @property
    def ecu_port(self) -> "ECUPort":
        assert self._ecu_port is not None
        return self._ecu_port

    def resolve_ecu_port(self, ports: list) -> None:
        self._ecu_port = next((p for p in ports if p.name == self.ecu_port_name), None)
        if self._ecu_port is None:
            raise err_major(
                f"ECU port '{self.ecu_port_name}' referenced in connection '{self.id}' was not found or was not validated",
                category=Category.REFERENCE,
                error_number="072",
            )


class SwitchPortToXConnection(InternalConnection):
    """
    Base class for connections from an ECU switch port to another component.

    Parameters
    ----------
    switch_port_name : str
        Name of the ECU switch port (alias: ``switch_port``).

    switch_name : str, optional
        Name of the parent switch that owns ``switch_port`` (alias: ``switch``). Required only
        when the switch-port name alone is ambiguous within the ECU (i.e. more than one of the
        ECU's switches defines a port with the same name). When omitted, the resolver scans all
        switches in the ECU and raises if the reference is ambiguous.

    Private Attributes
    ------------------
    _switch_port : :class:`~flync.model.flync_4_ecu.switch.SwitchPort`
        Internal reference to the actual switch port object.
        Managed privately.

    _switch : :class:`~flync.model.flync_4_ecu.switch.Switch`
        Internal reference to the parent switch of ``_switch_port``.
        Managed privately.
    """

    switch_port_name: Annotated[str, Reference(source="_switch_port")] = Field(alias="switch_port")
    switch_name: Annotated[Optional[str], Reference(source="_switch")] = Field(default=None, alias="switch")
    _switch_port: Optional["SwitchPort"] = PrivateAttr(default=None)
    _switch: Optional["Switch"] = PrivateAttr(default=None)

    @property
    def switch_port(self) -> "SwitchPort":
        assert self._switch_port is not None
        return self._switch_port

    @property
    def switch(self) -> "Switch":
        assert self._switch is not None
        return self._switch

    @staticmethod
    def _find_switch_port(port_name: str, switch_name: Optional[str], switches: list, connection_id: str) -> "SwitchPort":
        """Locate a switch port by name within the ECU's switches, with optional scoping to a named switch.

        Raises:
            err_major: The named switch does not exist in the ECU.
            err_major: The port name is ambiguous across multiple switches and no switch was specified.
            err_major: No matching switch port was found.
        """
        if switch_name is not None:
            switch = next((s for s in switches if s.name == switch_name), None)
            if switch is None:
                raise err_major(
                    f"Switch '{switch_name}' referenced in connection '{connection_id}' was not found in the current ECU.",
                    category=Category.REFERENCE,
                    error_number="073",
                )
            port = next((p for p in switch.ports if p.name == port_name), None)
        else:
            matches = [p for s in switches for p in s.ports if p.name == port_name]
            if len(matches) > 1:
                owners = [p.get_switch().name for p in matches]
                raise err_major(
                    f"Switch port '{port_name}' referenced in connection '{connection_id}' is ambiguous — "
                    f"multiple switches ({', '.join(owners)}) in the same ECU define a port with this name. "
                    f"Add an explicit 'switch:' reference (e.g. switch: {owners[0]}) to the connection to disambiguate. "
                    f"Switch port names are only unique within a single switch.",
                    category=Category.UNIQUENESS,
                    error_number="074",
                )
            port = matches[0] if matches else None
        if port is None:
            raise err_major(
                f"Switch port '{port_name}' referenced in connection '{connection_id}' was not found or was not validated",
                category=Category.REFERENCE,
                error_number="075",
            )
        return port

    def resolve_switch_port(self, switches: list) -> None:
        self._switch_port = self._find_switch_port(self.switch_port_name, self.switch_name, switches, self.id)
        self._switch = self._switch_port.switch


class ControllerInterfaceToXConnection(InternalConnection):
    """
    Base class for connections from an controller interface to another component inside an ECU.

    Parameters
    ----------
    iface_name : str
        Name of the controller interface (alias: ``controller_interface``).

    controller_name : str, optional
        Name of the parent controller that owns ``controller_interface`` (alias: ``controller``).
        Required only when ``controller_interface`` alone is ambiguous within the ECU (i.e. more
        than one of the ECU's controllers defines an interface with the same name).

    Private Attributes
    ------------------
    _iface : :class:`~flync.model.flync_4_ecu.controller.ControllerInterface`
        Internal reference to the actual ControllerInterface object.
        Managed privately.

    _controller : :class:`~flync.model.flync_4_ecu.controller.Controller`
        Internal reference to the parent controller of ``_iface``.
        Managed privately.
    """

    iface_name: Annotated[str, Reference(source="_iface")] = Field(alias="controller_interface")
    controller_name: Annotated[Optional[str], Reference(source="_controller")] = Field(default=None, alias="controller")
    _iface: Optional["EthernetInterface"] = PrivateAttr(default=None)
    _controller: Optional["Controller"] = PrivateAttr(default=None)

    @property
    def iface(self) -> "EthernetInterface":
        assert self._iface is not None
        return self._iface

    @property
    def controller(self) -> "Controller":
        assert self._controller is not None
        return self._controller

    @staticmethod
    def _find_controller_interface(
        iface_name: str, controller_name: Optional[str], controllers: list[Controller], connection_id: str
    ) -> "tuple[EthernetInterface, Controller]":
        """Locate a controller interface by name within the ECU's controllers, with optional scoping to a named controller.

        Raises:
            err_major: The named controller does not exist in the ECU.
            err_major: The interface name is ambiguous across multiple controllers and no controller was specified.
            err_major: No matching controller interface was found.
        """
        if controller_name is not None:
            ctrl = next((c for c in controllers if c.name == controller_name), None)
            if ctrl is None:
                raise err_major(
                    f"Controller '{controller_name}' referenced in connection '{connection_id}' was not found in the current ECU.",
                    category=Category.REFERENCE,
                    error_number="076",
                )
            iface = next((i for i in ctrl.get_interfaces() if i.name == iface_name), None)
        else:
            matches = [(i, c) for c in controllers for i in c.get_interfaces() if i.name == iface_name]
            if len(matches) > 1:
                owners = [c.name for _, c in matches]
                raise err_major(
                    f"Controller interface '{iface_name}' referenced in connection '{connection_id}' is ambiguous — "
                    f"multiple controllers ({', '.join(owners)}) in the same ECU define an interface with this name. "
                    f"Add an explicit 'controller:' reference (e.g. controller: {owners[0]}) to the connection to disambiguate. "
                    f"Controller interface names are only unique within a single controller.",
                    category=Category.UNIQUENESS,
                    error_number="077",
                )
            iface, ctrl = matches[0] if matches else (None, None)
        if iface is None or ctrl is None:
            raise err_major(
                f"Controller interface '{iface_name}' referenced in connection '{connection_id}' was not found or was not validated",
                category=Category.REFERENCE,
                error_number="078",
            )
        return iface, ctrl

    def resolve_controller_interface(self, controllers: list) -> None:
        self._iface, self._controller = self._find_controller_interface(self.iface_name, self.controller_name, controllers, self.id)


class ECUPortToSwitchPort(ECUPortToXConnection, SwitchPortToXConnection):
    """
    Represents a connection from an ECU port to a switch port.

    Parameters
    ----------
    type : Literal["ecu_port_to_switch_port"]
        Type of the connection. Defaults to ``"ecu_port_to_switch_port"``.
    """

    type: Literal["ecu_port_to_switch_port"] = Field("ecu_port_to_switch_port")

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_ecu_port(ports)
        self.resolve_switch_port(switches)
        if not any(c.name == self.switch_port.name for c in self.ecu_port._connected_components):
            self.ecu_port._connected_components.append(self.switch_port)
        self.switch_port._connected_component = self.ecu_port
        self.switch_port.copy_mdi_config_to_switch(self.ecu_port.mdi_config)

    def validate_compatibility(self) -> None:
        validate_optional_mii_config_compatibility(self.ecu_port, self.switch_port, self.id)
        if self.ecu_port.mdi_config and self.switch_port.traffic_classes:
            validate_cbs_idleslopes_fit_portspeed(
                self.switch_port.traffic_classes,
                self.ecu_port.mdi_config.speed,
            )

    def get_switch_port_refs(self) -> "List[SwitchPort]":
        return [self.switch_port]


class ECUPortToControllerInterface(ECUPortToXConnection, ControllerInterfaceToXConnection):
    """
    Represents a connection from an ECU port to a controller interface.

    Parameters
    ----------
    type : Literal["ecu_port_to_controller_interface"]
        Type of the connection.
        Defaults to ``"ecu_port_to_controller_interface"``.

    iface_name : str
        Name of the controller interface (alias: ``controller_interface``).

    controller_name : str, optional
        Name of the parent controller that owns ``controller_interface`` (alias: ``controller``).
        Required only when ``controller_interface`` alone is ambiguous within the ECU (i.e. more
        than one of the ECU's controllers defines an interface with the same name).

    Private Attributes
    ------------------
    _iface : :class:`~flync.model.flync_4_ecu.controller.ControllerInterface`
        Internal reference to the actual ControllerInterface object.
        Managed privately.

    _controller : :class:`~flync.model.flync_4_ecu.controller.Controller`
        Internal reference to the parent controller of ``_iface``.
        Managed privately.
    """

    type: Literal["ecu_port_to_controller_interface"] = Field("ecu_port_to_controller_interface")

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_ecu_port(ports)
        self.resolve_controller_interface(controllers)
        if not any(c.name == self.iface.name for c in self.ecu_port._connected_components):
            self.ecu_port._connected_components.append(self.iface)
        self.iface._connected_component.append(self.ecu_port)

    def validate_compatibility(self) -> None:
        validate_optional_mii_config_compatibility(self.ecu_port, self.iface.interface_config, self.id)
        validate_htb(self.iface.interface_config, self.ecu_port.mdi_config.speed if self.ecu_port.mdi_config else None)


class SwitchPortToControllerInterface(SwitchPortToXConnection, ControllerInterfaceToXConnection):
    """
    Represents a connection from a switch port to a controller interface.

    Parameters
    ----------
    type : Literal["switch_port_to_controller_interface"]
        Type of the connection.
        Defaults to ``"switch_port_to_controller_interface"``.
    """

    type: Literal["switch_port_to_controller_interface"] = Field("switch_port_to_controller_interface")

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_switch_port(switches)
        self.resolve_controller_interface(controllers)
        self.switch_port._connected_component = self.iface
        self.iface._connected_component.append(self.switch_port)

    def validate_compatibility(self) -> None:
        validate_compulsory_mii_config_compatibility(self.switch_port, self.iface.interface_config, self.id)
        assert self.iface.interface_config.mii_config is not None
        validate_htb(self.iface.interface_config, self.iface.interface_config.mii_config.speed)
        validate_macsec(self.switch_port, self.iface.interface_config, self.id)
        validate_gptp(self.switch_port, self.iface.interface_config, self.id)

    def get_switch_port_refs(self) -> "List[SwitchPort]":
        return [self.switch_port]


class SwitchPortToHostControllerInterface(SwitchPortToXConnection):
    """
    Represents the on-die connection between a switch's CPU port and an Ethernet interface of that same
    switch's own host controller.

    Unlike :class:`SwitchPortToControllerInterface`, this connection models an internal, on-die link — the
    switch and its host controller sit on the same die/package, so there is no PHY, MII, or cabling involved.
    No MII/MACsec/gPTP/HTB compatibility checks are performed for this connection type. The referenced
    interface must belong to the CPU port's own switch; it can never reference the host controller of a
    different switch.

    Parameters
    ----------
    type : Literal["switch_port_to_host_controller_interface"]
        Type of the connection. Defaults to ``"switch_port_to_host_controller_interface"``.

    host_controller_interface_name : str
        Name of the Ethernet interface on the switch's own host controller (alias: ``host_controller_interface``).

    Private Attributes
    ------------------
    _iface : :class:`~flync.model.flync_4_ecu.controller.EthernetInterface`
        Internal reference to the resolved host-controller interface.
        Managed privately.
    """

    type: Literal["switch_port_to_host_controller_interface"] = Field("switch_port_to_host_controller_interface")

    host_controller_interface_name: Annotated[str, Reference(source="_iface")] = Field(alias="host_controller_interface")
    _iface: Optional["EthernetInterface"] = PrivateAttr(default=None)

    @property
    def iface(self) -> "EthernetInterface":
        assert self._iface is not None
        return self._iface

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_switch_port(switches)
        host_controller = self.switch.host_controller
        if host_controller is None:
            raise err_major(
                f"Connection '{self.id}' references a host controller interface, but switch '{self.switch.name}' has no host controller.",
                category=Category.REFERENCE,
                error_number="239",
            )
        self._iface = next((i for i in host_controller.get_interfaces() if i.name == self.host_controller_interface_name), None)
        if self._iface is None:
            raise err_major(
                f"Host controller interface '{self.host_controller_interface_name}' referenced in connection '{self.id}' was not found "
                f"on the host controller of switch '{self.switch.name}'.",
                category=Category.REFERENCE,
                error_number="240",
            )
        self.switch_port._connected_component = self.iface
        self.iface._connected_component.append(self.switch_port)

    def validate_compatibility(self) -> None:
        """No compatibility checks: the CPU port and its host controller interface are linked on-die, not through a PHY."""
        return None

    def get_switch_port_refs(self) -> "List[SwitchPort]":
        return [self.switch_port]


class SwitchPortToSwitchPort(SwitchPortToXConnection):
    """
    Represents a connection between two switch ports on the same ECU.

    Parameters
    ----------
    type : Literal["switch_to_switch_same_ecu"]
        Type of the connection. Defaults to ``"switch_to_switch_same_ecu"``.

    switch2_port_name : str
        Name of the second switch port (alias: ``switch2_port``).

    switch2_name : str, optional
        Name of the parent switch that owns ``switch2_port`` (alias: ``switch2``). Required only
        when ``switch2_port`` alone is ambiguous within the ECU. Mirrors the role of ``switch`` for
        the first port; see :class:`SwitchPortToXConnection`.

    Private Attributes
    ------------------
    _switch2_port : :class:`~flync.model.flync_4_ecu.switch.SwitchPort`
        Internal reference to the second switch port object.
        Managed privately.

    _switch2 : :class:`~flync.model.flync_4_ecu.switch.Switch`
        Internal reference to the parent switch of ``_switch2_port``.
        Managed privately.
    """

    type: Literal["switch_to_switch_same_ecu"] = Field("switch_to_switch_same_ecu")

    _switch2_port: Optional["SwitchPort"] = PrivateAttr(default=None)
    _switch2: Optional["Switch"] = PrivateAttr(default=None)
    switch2_port_name: Annotated[str, Reference(source="_switch2_port")] = Field(alias="switch2_port")
    switch2_name: Annotated[Optional[str], Reference(source="_switch2")] = Field(default=None, alias="switch2")

    @property
    def switch2_port(self) -> "SwitchPort":
        assert self._switch2_port is not None
        return self._switch2_port

    @property
    def switch2(self) -> "Switch":
        assert self._switch2 is not None
        return self._switch2

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_switch_port(switches)
        self._switch2_port = self._find_switch_port(self.switch2_port_name, self.switch2_name, switches, self.id)
        self._switch2 = self._switch2_port.switch
        self.switch_port._connected_component = self.switch2_port
        self.switch2_port._connected_component = self.switch_port

    def validate_compatibility(self) -> None:
        validate_compulsory_mii_config_compatibility(self.switch_port, self.switch2_port, self.id)
        validate_macsec(self.switch_port, self.switch2_port, self.id)
        validate_gptp(self.switch_port, self.switch2_port, self.id)

    def get_switch_port_refs(self) -> "List[SwitchPort]":
        return [self.switch_port, self.switch2_port]


class ControllerInterfaceToControllerInterface(ControllerInterfaceToXConnection):
    """
    Represents a direct connection between two controller interfaces in an ECU.

    Parameters
    ----------
    type : Literal["controller_interface_to_controller_interface"]
        Type of the connection. Defaults to \
        ``"controller_interface_to_controller_interface"``.

    iface_name : str
        Name of the first controller
        interface (alias: ``controller_interface``).

    iface2_name : str
        Name of the second controller
        interface (alias: ``controller_interface2``).

    controller2_name : str, optional
        Name of the parent controller that owns the second interface (alias: ``controller2``).

    Private Attributes
    ------------------
    _iface : \
    :class:`~flync.model.flync_4_ecu.controller.ControllerInterface`
        Internal reference to the first controller interface.
        Managed privately.

    _iface2 : \
    :class:`~flync.model.flync_4_ecu.controller.ControllerInterface`
        Internal reference to the second controller interface.
        Managed privately.

    _controller2 : :class:`~flync.model.flync_4_ecu.controller.Controller`
        Internal reference to the controller that owns the second interface.
        Managed privately.
    """

    type: Literal["controller_interface_to_controller_interface"] = Field("controller_interface_to_controller_interface")

    iface2_name: Annotated[str, Reference(source="_iface2")] = Field(alias="controller_interface2")
    controller2_name: Annotated[Optional[str], Reference(source="_controller2")] = Field(default=None, alias="controller2")
    _iface2: Optional["EthernetInterface"] = PrivateAttr(default=None)
    _controller2: Optional["Controller"] = PrivateAttr(default=None)

    @property
    def iface2(self) -> "EthernetInterface":
        assert self._iface2 is not None
        return self._iface2

    @property
    def controller2(self) -> "Controller":
        assert self._controller2 is not None
        return self._controller2

    def bind(self, switches: list, controllers: list, ports: list) -> None:
        self.resolve_controller_interface(controllers)
        self._iface2, self._controller2 = self._find_controller_interface(self.iface2_name, self.controller2_name, controllers, self.id)
        self.iface._connected_component.append(self.iface2)
        self.iface2._connected_component.append(self.iface)

    def validate_compatibility(self) -> None:
        validate_compulsory_mii_config_compatibility(self.iface.interface_config, self.iface2.interface_config, self.id)
        assert self.iface.interface_config.mii_config is not None
        assert self.iface2.interface_config.mii_config is not None
        validate_htb(self.iface.interface_config, self.iface.interface_config.mii_config.speed)
        validate_htb(self.iface2.interface_config, self.iface2.interface_config.mii_config.speed)
        validate_macsec(self.iface.interface_config, self.iface2.interface_config, self.id)
        validate_gptp(self.iface.interface_config, self.iface2.interface_config, self.id)


class InternalConnectionUnion(RootModel):
    """
    Union type representing a connection between two internal ECU components.

    This model wraps a union of different internal connection types and uses the ``type`` field as a discriminator to
    determine which specific connection type is present.

    Possible types
    --------------
    :class:`~flync.model.flync_4_ecu.internal_topology.ECUPortToSwitchPort`
        Connection from an ECU port to a switch port.

    :class:`~flync.model.flync_4_ecu.internal_topology.ECUPortToControllerInterface`
        Connection from an ECU port to a controller interface.

    :class:`~flync.model.flync_4_ecu.internal_topology.SwitchPortToControllerInterface`
        Connection from a switch port to a controller interface.

    :class:`~flync.model.flync_4_ecu.internal_topology.SwitchPortToHostControllerInterface`
        On-die connection from a switch's CPU port to an interface of that switch's own host controller.

    :class:`~flync.model.flync_4_ecu.internal_topology.SwitchPortToSwitchPort`
        Connection between two switch ports on the same ECU.

    :class:`~flync.model.flync_4_ecu.internal_topology.ControllerInterfaceToControllerInterface`
        Connection between two controller interfaces.
    """

    root: (
        ECUPortToSwitchPort
        | ECUPortToControllerInterface
        | SwitchPortToControllerInterface
        | SwitchPortToHostControllerInterface
        | SwitchPortToSwitchPort
        | ControllerInterfaceToControllerInterface
    ) = Field(discriminator="type")


class InternalTopology(FLYNCBaseModel):
    """
    Parameters
    ----------
    connections : list of :class:`InternalConnectionUnion`
        List of internal connections between ECU components.
        Defaults to an empty list.
    """

    connections: List[InternalConnectionUnion] = Field(default_factory=list)
