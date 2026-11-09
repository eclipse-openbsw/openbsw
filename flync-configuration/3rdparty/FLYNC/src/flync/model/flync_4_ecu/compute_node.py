"""
Defines the ComputeNode model for FLYNC.

A :class:`ComputeNode` is a recursive container that mirrors the shape of a
:class:`~flync.model.flync_4_ecu.controller.Controller`: it owns Ethernet interfaces, may bind
applications, and may itself host further compute nodes and virtual switches. Nesting a compute node
inside another models a hypervisor hosting a guest that in turn hosts its own guests.

Unlike a Controller, a compute node has no ``controller_topology`` of its own — every link inside a
controller, at any nesting depth, is declared once in that controller's
:class:`~flync.model.flync_4_ecu.controller_topology.ControllerTopology`.
"""

from typing import Annotated, Iterator, List, Optional

from pydantic import Field, model_validator

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_app.app_bindings import AppBindings
from flync.model.flync_4_ecu.controller import EthernetInterface
from flync.model.flync_4_ecu.switch import Switch


class ComputeNode(FLYNCBaseModel):
    """
    A compute node hosted by a controller, or nested inside another compute node.

    A compute node may own virtual Ethernet interfaces (``EthernetInterface`` without a PHY), may declare
    its own virtual switches to bridge them, and may host further compute nodes. It is wired to the
    rest of the controller through the controller's ``controller_topology.flync.yaml`` — a compute node
    never connects to anything outside its own controller.

    Parameters
    ----------
    name : str
        Name of the compute node, implied from the folder name on disk. Must be unique across the
        whole controller subtree.

    ethernet_interfaces : list of :class:`~flync.model.flync_4_ecu.controller.EthernetInterface`, optional
        Virtual Ethernet interfaces exposed by this compute node. These carry no ``mii_config``: a
        virtual NIC has no PHY.

    app_bindings : :class:`~flync.model.flync_4_app.AppBindings`, optional
        Applications this compute node should bind to.

    compute_nodes : list of :class:`ComputeNode`, optional
        Compute nodes nested inside this one.

    virtual_switches : list of :class:`~flync.model.flync_4_ecu.switch.Switch`, optional
        Software MAC bridges defined inside this compute node.
    """

    name: Annotated[
        str,
        Implied(
            strategy=ImpliedStrategy.FOLDER_NAME,
        ),
    ] = Field()
    ethernet_interfaces: Annotated[
        Optional[List[EthernetInterface]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    app_bindings: Annotated[
        Optional[AppBindings],
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(default=None, description="Applications a compute node should bind to.")
    compute_nodes: Annotated[
        Optional[List["ComputeNode"]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)
    virtual_switches: Annotated[
        Optional[List[Switch]],
        External(
            output_structure=OutputStrategy.FOLDER,
            naming_strategy=NamingStrategy.FIELD_NAME,
        ),
    ] = Field(default_factory=list)

    @model_validator(mode="after")
    def reject_mii_config_on_virtual_interface(self):
        """
        Raise when an interface of this compute node declares a ``mii_config``.

        A virtual NIC is a software endpoint with no media-independent interface behind it; a PHY
        configuration on one signals that a physical controller interface was placed in the compute
        node by mistake.
        """

        for iface in self.ethernet_interfaces or []:
            if iface.interface_config is not None and iface.interface_config.mii_config is not None:
                raise err_major(
                    "Interface '{iface}' of compute node '{node}' declares a mii_config. "
                    "A virtual interface has no PHY — move the interface to the controller if it is a physical one.",
                    category=Category.CONSISTENCY,
                    error_number="339",
                    iface=iface.name,
                    node=self.name,
                )
        return self

    def get_interfaces(self) -> List["EthernetInterface"]:
        """Return this compute node's own Ethernet interfaces, excluding those of nested compute nodes."""
        return list(self.ethernet_interfaces or [])

    def get_all_deployments(self) -> Iterator:
        """Yield every SOME/IP deployment declared on this compute node's sockets."""

        for eth_iface in self.ethernet_interfaces or []:
            for sock_con in eth_iface.sockets or []:
                for socket in sock_con.sockets or []:
                    yield from socket.deployments or []

    def get_consumed_service_instances(self) -> set:
        """Return the ``(service, major_version, instance_id)`` triples this compute node deploys as a SOME/IP consumer."""

        return {
            (dep.root.service, dep.root.major_version, dep.root.instance_id)
            for dep in self.get_all_deployments()
            if dep.root.deployment_type == "someip_consumer"
        }

    def model_post_init(self, __context):
        for interface in self.ethernet_interfaces or []:
            if interface.interface_config is not None:
                interface._controller = self
                interface.interface_config._name = interface.name
        return super().model_post_init(__context)


def iter_subtree_compute_nodes(node) -> Iterator["ComputeNode"]:
    """
    Yield every :class:`ComputeNode` beneath ``node``, depth-first, at any nesting depth.

    ``node`` is a :class:`~flync.model.flync_4_ecu.controller.Controller` or a
    :class:`ComputeNode` — both expose a ``compute_nodes`` field, so the walk is uniform.
    """

    for child in getattr(node, "compute_nodes", None) or []:
        yield child
        yield from iter_subtree_compute_nodes(child)


def iter_subtree_switches(node) -> Iterator["Switch"]:
    """
    Yield every virtual switch beneath ``node``, including those inside nested compute nodes.

    ``node`` is a :class:`~flync.model.flync_4_ecu.controller.Controller`, whose own switches are
    under ``switches``, or a :class:`ComputeNode`, whose own switches are under ``virtual_switches``.
    Every nested compute node uses ``virtual_switches`` regardless of ``node``'s own type.
    """

    yield from getattr(node, "switches", None) or getattr(node, "virtual_switches", None) or []
    for child in iter_subtree_compute_nodes(node):
        yield from child.virtual_switches or []


def iter_subtree_interfaces(node) -> Iterator["EthernetInterface"]:
    """
    Yield every Ethernet interface beneath ``node``, including those of nested compute nodes.

    For a Controller this is its physical interfaces followed by every compute node interface in the
    subtree. Use :meth:`~flync.model.flync_4_ecu.controller.Controller.get_interfaces` instead wherever
    only the physical, ECU-visible interfaces are meant.
    """

    yield from getattr(node, "ethernet_interfaces", None) or []
    for child in iter_subtree_compute_nodes(node):
        yield from child.ethernet_interfaces or []
