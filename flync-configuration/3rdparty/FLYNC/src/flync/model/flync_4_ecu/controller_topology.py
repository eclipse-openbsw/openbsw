"""
Defines the internal connection topology of a single Controller.

Where :mod:`~flync.model.flync_4_ecu.internal_topology` wires the components of an ECU (ports,
hardware switches, controller interfaces), this module wires the components *inside* one controller:
its virtual switches and the Ethernet interfaces of the controller itself and of every compute
node nested beneath it.

The two scopes are disjoint and never overlap. **The controller's own physical Ethernet interface is
the only thing that crosses the controller boundary**: a compute node, a virtual interface or a
virtual switch is never referenced from an ECU topology, and an ECU port or hardware switch is never
referenced from a controller topology. A software-to-hardware uplink is therefore two hops in two
files — virtual switch port to physical interface here, physical interface to hardware switch port in
the ECU's ``topology.flync.yaml``.

Reference resolution is inherited unchanged from
:mod:`~flync.model.flync_4_ecu.internal_topology`; only the compatibility checks differ. Every link
in here is a software link between endpoints on the same SoC, so there is no PHY, MII, cabling,
MACsec or gPTP link state to reconcile — exactly like
:class:`~flync.model.flync_4_ecu.internal_topology.SwitchPortToHostControllerInterface` and its
on-die connection. The subclasses below therefore keep the resolution and drop the media checks,
while keeping the ``type`` discriminator values of their bases so the YAML vocabulary is shared with
the ECU topology.
"""

from typing import List

from pydantic import Field, RootModel

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.model.flync_4_ecu.internal_topology import (
    ControllerInterfaceToControllerInterface,
    SwitchPortToControllerInterface,
    SwitchPortToSwitchPort,
)


class VirtualSwitchPortToInterface(SwitchPortToControllerInterface):
    """
    A virtual switch port connected to an Ethernet interface of the controller or of one of its compute nodes.

    This is also how a virtual switch uplinks onto a *physical* controller interface: the physical
    interface's ``mii_config`` describes the media on its other side, facing the ECU, and has nothing
    to do with this software-side attachment.
    """

    def validate_compatibility(self) -> None:
        """No media checks: both endpoints are software endpoints on the same SoC, with no PHY between them."""
        return None


class InterfaceToInterfaceLink(ControllerInterfaceToControllerInterface):
    """A direct point-to-point link between two Ethernet interfaces inside a controller, with no virtual switch in between."""

    def validate_compatibility(self) -> None:
        """No media checks: both endpoints are software endpoints on the same SoC, with no PHY between them."""
        return None


class VirtualSwitchPortToVirtualSwitchPort(SwitchPortToSwitchPort):
    """A link between two virtual switches inside the same controller."""

    def validate_compatibility(self) -> None:
        """No media checks: both endpoints are software endpoints on the same SoC, with no PHY between them."""
        return None


class ControllerConnectionUnion(RootModel):
    """
    Union type representing a connection between two components inside a controller.

    The ``type`` field discriminates which connection is present.

    Possible types
    --------------
    :class:`VirtualSwitchPortToInterface`
        ``switch_port_to_controller_interface`` — virtual switch port to an Ethernet interface of the
        controller or of any compute node beneath it.

    :class:`InterfaceToInterfaceLink`
        ``controller_interface_to_controller_interface`` — direct point-to-point link between two
        Ethernet interfaces.

    :class:`VirtualSwitchPortToVirtualSwitchPort`
        ``switch_to_switch_same_ecu`` — link between two virtual switches.
    """

    root: VirtualSwitchPortToInterface | InterfaceToInterfaceLink | VirtualSwitchPortToVirtualSwitchPort = Field(discriminator="type")


class ControllerTopology(FLYNCBaseModel):
    """
    Internal connectivity of one controller, stored in ``controller_topology.flync.yaml``.

    Parameters
    ----------
    connections : list of :class:`ControllerConnectionUnion`
        Connections between the controller's virtual switches and the Ethernet interfaces of the
        controller and of its compute nodes. Defaults to an empty list.
    """

    connections: List[ControllerConnectionUnion] = Field(default_factory=list)
