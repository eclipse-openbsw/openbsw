"""PDU socket deployments (sender/receiver) used by ``Socket.DeploymentUnion``."""

from typing import Literal

from pydantic import Field

from flync.core.base_models import FLYNCBaseModel


class PDUSender(FLYNCBaseModel):
    """
    Deployment that publishes a PDU onto a socket.

    Transport (TCP/UDP, IP address, port) is owned by the enclosing socket; this model only binds a PDU to that socket.
    The publishing ECU is the owner of the socket carrying this deployment.

    Parameters
    ----------
    deployment_type : Literal["pdu_sender"]
        Discriminator value for :class:`~flync.model.flync_4_ecu.sockets.DeploymentUnion`.
    pdu_ref : str
        Name of a PDU declared under ``communication.channels`` — any of
        :class:`~flync.model.flync_4_signal.pdu.StandardPDU`,
        :class:`~flync.model.flync_4_signal.pdu.MultiplexedPDU`, or
        :class:`~flync.model.flync_4_signal.pdu.ContainerPDU`.
        Validated at workspace level once the full model is assembled.
    """

    deployment_type: Literal["pdu_sender"] = Field(default="pdu_sender")
    pdu_ref: str = Field()


class PDUReceiver(FLYNCBaseModel):
    """
    Deployment that subscribes to a PDU on a socket.

    Transport (TCP/UDP, IP address, port) is owned by the enclosing socket; this model only binds a PDU to that socket.
    The receiving ECU is the owner of the socket carrying this deployment.

    Parameters
    ----------
    deployment_type : Literal["pdu_receiver"]
        Discriminator value for :class:`~flync.model.flync_4_ecu.sockets.DeploymentUnion`.
    pdu_ref : str
        Name of a PDU declared under ``communication.channels`` — any of
        :class:`~flync.model.flync_4_signal.pdu.StandardPDU`,
        :class:`~flync.model.flync_4_signal.pdu.MultiplexedPDU`, or
        :class:`~flync.model.flync_4_signal.pdu.ContainerPDU`.
        Validated at workspace level once the full model is assembled.
    """

    deployment_type: Literal["pdu_receiver"] = Field(default="pdu_receiver")
    pdu_ref: str = Field()
