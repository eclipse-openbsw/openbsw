"""CAN interface configuration for ECU controllers."""

from typing import Annotated, List

from pydantic import Field, model_validator

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.model.flync_4_ecu.controller_interface import ControllerInterface
from flync.model.flync_4_signal.forwarder import CANFrameForwarder


class CANFrameRef(FLYNCBaseModel):
    """
    Reference to a CAN frame by bus and CAN ID.

    Parameters
    ----------
    bus_ref : str
        Name of the :class:`~flync.model.flync_4_bus.can_bus.CANBus` that owns the frame.
    frame_ref : int
        CAN ID of the :class:`~flync.model.flync_4_signal.frame.CANFrame` or
        :class:`~flync.model.flync_4_signal.frame.CANFDFrame` on the referenced bus.
    """

    bus_ref: str = Field()
    frame_ref: int = Field()


class CANInterface(ControllerInterface):
    """
    CAN interface of a controller, declaring which frames the controller sends, receives, and forwards on a CAN bus.

    Parameters
    ----------
    name : str
        Name of the CAN interface, implied from the file name on disk.

    bus_ref : str
        Name of the :class:`~flync.model.flync_4_bus.can_bus.CANBus` this interface connects to.
    sender_frames : list of :class:`CANFrameRef`
        Frames transmitted by this controller on the bus.
    receiver_frames : list of :class:`CANFrameRef`
        Frames received by this controller from the bus.
    forwarder_frames : list of \
:class:`~flync.model.flync_4_signal.frame.CANFrameForwarder`
        Frames received by this controller and re-emitted on one or more
        egresses.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    bus_ref: str = Field()
    sender_frames: List[CANFrameRef] = Field(default_factory=list)
    receiver_frames: List[CANFrameRef] = Field(default_factory=list)
    forwarder_frames: List[CANFrameForwarder] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_forwarder_frame_uniqueness(self) -> "CANInterface":
        """Raise ``err_major`` if the same ``frame_ref`` appears twice in ``forwarder_frames``."""

        seen: set = set()
        duplicates: set = set()
        for fwd in self.forwarder_frames:
            if fwd.frame_ref in seen:
                duplicates.add(fwd.frame_ref)
            seen.add(fwd.frame_ref)
        if duplicates:
            raise err_major(
                "CANInterface(bus_ref={bus}): duplicate frame_ref(s) in forwarder_frames: {dups}",
                bus=self.bus_ref,
                dups=sorted(duplicates),
                category=Category.UNIQUENESS,
                error_number="058",
            )
        return self
