"""Forwarder deployments and egress sinks for the PDU gateway feature."""

from typing import List, Literal, Optional

from pydantic import Field, RootModel, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major


class CANFrameEgress(FLYNCBaseModel):
    """Forwarder egress that re-emits the forwarded PDU on a CAN frame.

    Parameters
    ----------
    egress_type : Literal["can_frame"]
        Type discriminator for this egress.

    bus_ref : str
        Reference to the target CAN bus.

    frame_ref : int
        Reference to the target CAN frame.

    extract_pdu_ref : str, optional
        Optional reference to a PDU to extract from the forwarded data.
    """

    egress_type: Literal["can_frame"] = Field(default="can_frame")
    bus_ref: str = Field()
    frame_ref: int = Field()
    extract_pdu_ref: Optional[str] = Field(default=None)


class EthSocketEgress(FLYNCBaseModel):
    """Forwarder egress that re-emits the forwarded PDU on an Ethernet socket (unicast/multicast follows the target's endpoint_address).

    Parameters
    ----------
    egress_type : Literal["eth_socket"]
        Type discriminator for this egress.

    socket_ref : str
        Reference to the target Ethernet socket.

    extract_pdu_ref : str, optional
        Optional reference to a PDU to extract from the forwarded data.
    """

    egress_type: Literal["eth_socket"] = Field(default="eth_socket")
    socket_ref: str = Field()
    extract_pdu_ref: Optional[str] = Field(default=None)


class ForwarderEgress(RootModel):
    """Discriminated-union wrapper over a single forwarder egress (``CANFrameEgress`` or ``EthSocketEgress``)."""

    root: CANFrameEgress | EthSocketEgress = Field(discriminator="egress_type")


class PDUForwarder(FLYNCBaseModel):
    """Socket deployment that consumes a PDU on its parent socket and re-emits it on one or more egresses.

    Parameters
    ----------
    deployment_type : Literal["pdu_forwarder"]
        Type discriminator for this forwarder.

    pdu_ref : str
        Reference to the PDU being forwarded.

    egresses : List[:class:`ForwarderEgress`]
        List of egress targets where the forwarded PDU is re-emitted. Must contain at least one egress.
    """

    deployment_type: Literal["pdu_forwarder"] = Field(default="pdu_forwarder")
    pdu_ref: str = Field()
    egresses: List[ForwarderEgress] = Field(min_length=1)

    @model_validator(mode="after")
    def validate_pdu_forwarder_egress_uniqueness(self) -> "PDUForwarder":
        """Ensure every egress in ``egresses`` points at a distinct target carrier."""

        _check_egress_uniqueness(self.egresses, owner=f"PDUForwarder(pdu_ref={self.pdu_ref})")
        return self


class CANFrameForwarder(FLYNCBaseModel):
    """CAN-interface forwarder that consumes an ingress frame and re-emits it on one or more egresses.

    Parameters
    ----------
    frame_ref : str
        Reference to the CAN frame being forwarded.

    egresses : List[:class:`ForwarderEgress`]
        List of egress targets where the forwarded frame is re-emitted. Must contain at least one egress.
    """

    frame_ref: str = Field()
    egresses: List[ForwarderEgress] = Field(min_length=1)

    @model_validator(mode="after")
    def validate_can_frame_forwarder_egress_uniqueness(self) -> "CANFrameForwarder":
        """Ensure every egress in ``egresses`` points at a distinct target carrier."""

        _check_egress_uniqueness(self.egresses, owner=f"CANFrameForwarder(frame_ref={self.frame_ref})")
        return self


def _check_egress_uniqueness(egresses: List[ForwarderEgress], owner: str) -> None:
    """Raise ``err_major`` if two egresses point at the exact same target carrier."""

    seen: set = set()
    for idx, egress_root in enumerate(egresses):
        egress = egress_root.root
        key: tuple
        if isinstance(egress, CANFrameEgress):
            key = ("can_frame", egress.bus_ref, egress.frame_ref)
        else:
            key = ("eth_socket", egress.socket_ref)
        if key in seen:
            raise err_major(
                "{owner}: egresses[{idx}] duplicates an earlier egress targeting the same target '{key}'.",
                owner=owner,
                idx=idx,
                key=key,
                category=Category.UNIQUENESS,
                error_number="102",
            )
        seen.add(key)
