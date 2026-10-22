"""LIN interface configuration for ECU controllers."""

from typing import Annotated, List, Literal, Optional, Union

from pydantic import Field

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.model.flync_4_ecu.controller_interface import ControllerInterface

_LINProtocol = Literal["1.3", "2.0", "2.1", "2.2A"]


class LINFrameRef(FLYNCBaseModel):
    """
    Reference to a LIN frame by bus and LIN ID.

    Parameters
    ----------
    bus_ref : str
        Name of the :class:`~flync.model.flync_4_bus.lin_bus.LINBus` that owns the frame.
    frame_ref : int
        LIN ID of the :class:`~flync.model.flync_4_signal.frame.LINFrame` on the referenced bus.
    """

    bus_ref: str = Field()
    frame_ref: int = Field()


class LINMasterInterface(ControllerInterface):
    """
    LIN interface for a controller acting as LIN master on a bus.

    The master controls the schedule and initiates all frame transmissions.

    Parameters
    ----------
    name : str
        Name of the LIN interface, implied from the file name on disk.

    node_type : Literal["master"]
        Discriminator field.  Always ``"master"``.
    bus_ref : str
        Name of the :class:`~flync.model.flync_4_bus.lin_bus.LINBus` this interface connects to.
    lin_protocol : Literal["1.3", "2.0", "2.1", "2.2A"]
        LIN protocol version supported by this node.
    p2_min : float
        Minimum time (ms) between the end of a slave response and the start of the next frame header.
        Maps to ``P2_min`` in the LDF ``Nodes.Master`` block.
    st_min : float
        Minimum separation time (ms) between consecutive frame headers transmitted by the master.
        Maps to ``ST_min`` in the LDF ``Nodes.Master`` block.
    sender_frames : list of :class:`LINFrameRef`
        Frames transmitted (scheduled) by this master controller.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    node_type: Literal["master"] = Field(default="master")
    bus_ref: str = Field()
    lin_protocol: _LINProtocol = Field()
    p2_min: float = Field()
    st_min: float = Field()
    sender_frames: List[LINFrameRef] = Field(default_factory=list)


class LINSlaveInterface(ControllerInterface):
    """
    LIN interface for a controller acting as LIN slave on a bus.

    The slave responds to frame headers issued by the master.

    Parameters
    ----------
    name : str
        Name of the LIN interface, implied from the file name on disk.

    node_type : Literal["slave"]
        Discriminator field.  Always ``"slave"``.
    bus_ref : str
        Name of the :class:`~flync.model.flync_4_bus.lin_bus.LINBus` this interface connects to.
    lin_protocol : Literal["1.3", "2.0", "2.1", "2.2A"]
        LIN protocol version supported by this node.
    configured_nad : int
        Configured Node Address (0x00 – 0xFF).
        Written as ``configured_NAD`` in the LDF ``Node_attributes`` block.
    initial_nad : int
        Initial Node Address (0x00 – 0xFF).
        Written as ``initial_NAD`` in the LDF ``Node_attributes`` block.
    product_id : str, optional
        Product identification string.
        Written as ``product_id`` in the LDF ``Node_attributes`` block.
    response_error : str, optional
        Name of the signal used to report response errors.
        Written as ``response_error`` in the LDF ``Node_attributes`` block.
    receiver_frames : list of :class:`LINFrameRef`
        Frames received by this slave controller from the bus.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    node_type: Literal["slave"] = Field(default="slave")
    bus_ref: str = Field()
    lin_protocol: _LINProtocol = Field()
    configured_nad: Annotated[int, Field(ge=0, le=0xFF)] = Field()
    initial_nad: Annotated[int, Field(ge=0, le=0xFF)] = Field()
    product_id: Optional[str] = Field(default=None)
    response_error: Optional[str] = Field(default=None)
    receiver_frames: List[LINFrameRef] = Field(default_factory=list)


AnyLINInterface = Annotated[
    Union[LINMasterInterface, LINSlaveInterface],
    Field(discriminator="node_type"),
]
"""Discriminated union of LIN master and slave interface configs, keyed on ``node_type``."""
