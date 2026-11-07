"""Defines sockets, IP address endpoints, and TCP/UDP models in FLYNC"""

from typing import (
    Annotated,
    List,
    Literal,
    Optional,
)

from pydantic import (
    Field,
    RootModel,
    StrictBool,
    ValidationError,
    field_serializer,
    field_validator,
    model_validator,
)
from pydantic.networks import IPvAnyAddress

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes.ipaddress import (
    IPv4AddressEntry,
    IPv6AddressEntry,
)
from flync.core.utils.exceptions import Category, err_major, err_minor, warn
from flync.model.flync_4_diagnostics import DoIPDiscoveryDeployment, DoIPServerDeployment
from flync.model.flync_4_signal.forwarder import PDUForwarder
from flync.model.flync_4_signal.pdu_deployment import PDUReceiver, PDUSender
from flync.model.flync_4_someip import (
    SOMEIPSDDeployment,
    SOMEIPServiceConsumer,
    SOMEIPServiceProvider,
)


class DeploymentUnion(RootModel):
    """
    Union type representing a deployment configuration for a socket. This model wraps a union of different deployment models and uses the
    ``deployment_type`` field as a discriminator to determine which specific deployment model is present.

    Possible types
    --------------
    :class:`~flync.model.flync_4_someip.SOMEIPServiceConsumer`
    or
    :class:`~flync.model.flync_4_someip.SOMEIPServiceProvider`
    or
    :class:`~flync.model.flync_4_someip.SOMEIPSDDeployment`
    or
    :class:`~flync.model.flync_4_signal.pdu_deployment.PDUSender`
    or
    :class:`~flync.model.flync_4_signal.pdu_deployment.PDUReceiver`
    or
    :class:`~flync.model.flync_4_signal.frame.PDUForwarder`
    or
    :class:`~flync.model.flync_4_diagnostics.DoIPServerDeployment`
    or
    :class:`~flync.model.flync_4_diagnostics.DoIPDiscoveryDeployment`

    """

    root: (
        SOMEIPServiceConsumer
        | SOMEIPServiceProvider
        | SOMEIPSDDeployment
        | PDUSender
        | PDUReceiver
        | PDUForwarder
        | DoIPServerDeployment
        | DoIPDiscoveryDeployment
    ) = Field(discriminator="deployment_type")


def get_endpoint_type_from_address(
    address: IPvAnyAddress,
) -> Literal["multicast", "unicast"]:
    """
    Determine the endpoint type (multicast or unicast) based on the given IP address.
    """

    if address.is_multicast:
        return "multicast"
    else:
        return "unicast"


class Socket(FLYNCBaseModel):
    """
    Defines a virtual-interface socket that is bound to a specific IP
    address.

    Parameters
    ----------
    name : str
        A readable identifier for the socket.

    endpoint_address : :class:`IPv4Address` or :class:`IPv6Address`
        The IP address assigned to the socket.

    port_no : int
        The port number the socket uses for communication.

    deployments : list of :class:`DeploymentUnion`, optional
        Deployments of the socket.

    endpoint_type : Literal["multicast", "unicast"], optional
        The type of the socket endpoint, which can be either "multicast" or "unicast".  This field is per default automatically determined
        based on the value of ``endpoint_address``, but can be overridden by explicitly providing a value.

    multicast_tx : list of :class:`~pydantic.networks.IPvAnyAddress`, optional
        Multicast addresses that the socket is allowed to transmit to (only applicable for sockets with a multicast endpoint_type).
    """

    name: str = Field()
    endpoint_address: IPvAnyAddress = Field()
    port_no: int = Field()
    deployments: Optional[List[DeploymentUnion]] = Field(default_factory=list)
    endpoint_type: Optional[Literal["multicast", "unicast"]] = Field(default_factory=lambda x: get_endpoint_type_from_address(x["endpoint_address"]))
    multicast_tx: Optional[List[IPvAnyAddress]] = Field(default_factory=list)

    @field_validator("deployments", mode="before")
    def drop_invalid_deployment(cls, deployment):
        """
        Drop invalid deployments from the list of deployments.
        """

        valid_deployment = []
        idx = 0
        for dep in deployment:
            try:
                DeploymentUnion.model_validate(dep)
                valid_deployment.append(dep)
            except ValidationError as e:
                detail = "; ".join(
                    "{loc}: {msg}".format(
                        loc=".".join(str(x) for x in err.get("loc", ())),
                        msg=err.get("msg", ""),
                    )
                    for err in e.errors()
                )
                raise err_minor(
                    f"Validation error in deployment {idx} of socket - {detail}. Skipping to the next deployment.",
                    category=Category.STRUCTURAL,
                    error_number="082",
                )
            idx = idx + 1
        return valid_deployment

    @model_validator(mode="after")
    def validate_unique_forwarder_per_pdu(self) -> "Socket":
        """
        Raise ``err_major`` if two ``PDUForwarder`` deployments on this socket target the same ``pdu_ref``.
        """

        seen: set = set()
        duplicates: set = set()
        for dep_root in self.deployments or []:
            dep = dep_root.root
            if isinstance(dep, PDUForwarder):
                if dep.pdu_ref in seen:
                    duplicates.add(dep.pdu_ref)
                seen.add(dep.pdu_ref)
        if duplicates:
            raise err_major(
                "Socket '{name}': duplicate PDUForwarder deployment(s) for pdu_ref(s): {dups}",
                name=self.name,
                dups=sorted(duplicates),
                category=Category.UNIQUENESS,
                error_number="083",
            )
        return self

    @model_validator(mode="after")
    def validate_unique_someip_deployments(self) -> "Socket":
        """
        Raise ``err_major`` if two deployments of the same role (provider or consumer) on this socket target the
        same ``(service, major_version, instance_id)`` triple - indistinguishable on the wire.
        """

        for deployment_type in (SOMEIPServiceProvider, SOMEIPServiceConsumer):
            seen: set = set()
            for dep_root in self.deployments or []:
                dep = dep_root.root
                if not isinstance(dep, deployment_type):
                    continue
                key = (dep.service, dep.major_version, dep.instance_id)
                if key in seen:
                    raise err_major(
                        f"Socket '{self.name}': duplicate {deployment_type.__name__} deployment for service instance "
                        f"(service={dep.service:#06x}, major_version={dep.major_version}, instance_id={dep.instance_id})",
                        category=Category.UNIQUENESS,
                        error_number="244",
                    )
                seen.add(key)
        return self

    @model_validator(mode="after")
    def validate_doip_deployment_protocol(self) -> "Socket":
        """
        Raise ``err_major`` if a ``doip_server`` deployment sits on a UDP socket, or a
        ``doip_discovery`` deployment sits on a TCP socket - DoIP diagnostic messaging (ISO
        13400) runs over TCP, DoIP discovery runs over UDP.
        """

        protocol = getattr(self, "protocol", None)
        for dep_root in self.deployments or []:
            dep = dep_root.root
            if isinstance(dep, DoIPServerDeployment) and protocol != "tcp":
                raise err_major(
                    f"Socket '{self.name}': DoIPServerDeployment '{dep.name}' requires a TCP socket, found '{protocol}'",
                    category=Category.COMPATIBILITY,
                    error_number="274",
                )
            if isinstance(dep, DoIPDiscoveryDeployment) and protocol != "udp":
                raise err_major(
                    f"Socket '{self.name}': DoIPDiscoveryDeployment requires a UDP socket, found '{protocol}'",
                    category=Category.COMPATIBILITY,
                    error_number="275",
                )
        return self

    @field_serializer("endpoint_address")
    def serialize_endpoint_address(self, endpoint):
        if endpoint is not None:
            return str(endpoint).upper()

    @field_serializer("multicast_tx")
    def serialize_multicast_tx(self, multicast_tx):
        if multicast_tx is not None:
            return [str(endpoint).upper() for endpoint in multicast_tx]


class TCPOption(FLYNCBaseModel):
    """
    TCP options that can be enabled for a connection.

    Parameters
    ----------
    tcp_profile_id : int
        Unique identifier of the TCP profile.

    nagle : bool, optional
        Enable or disable Nagle algorithm. Defaults to ``False``.

    keepalive_enabled : bool, optional
        Enable or disable the TCP keep-alive option. Defaults to ``True``.

    keepidle : int, optional
        Seconds the connection must stay idle before the first keep-alive probe is sent.
        Defaults to ``10``.

    keepcount : int, optional
        Maximum number of keep-alive probes that may be sent before the connection is dropped.
        Defaults to ``10``.

    keepintvl : int, optional
        Seconds between successive keep-alive probes. Defaults to ``2``.

    user_timeout : int, optional
        Maximum time in seconds that unacknowledged data may remain before the connection is closed.
        Defaults to ``28``.

    congestion_avoidance : Literal["reno", "cubic", "bbr"], optional
        Congestion-avoidance algorithm to use. Defaults to ``"reno"``.

    tcp_maxseg : int, optional
        Maximum segment size for outgoing TCP packets. Defaults to ``1460``.

    tcp_quickack : bool, optional
        Enable or disable the "quick-ack" feature. Defaults to ``False``.

    tcp_syncnt : int, optional
        Number of SYN retransmissions TCP may perform before aborting the connection attempt.
        Defaults to ``6``.
    """

    tcp_profile_id: int = Field()
    nagle: Optional[StrictBool] = Field(default=False)
    keepalive_enabled: Optional[StrictBool] = Field(default=True)
    keepidle: Optional[int] = Field(default=10)
    keepcount: Optional[int] = Field(default=10)
    keepintvl: Optional[int] = Field(default=2)
    user_timeout: Optional[int] = Field(default=28)
    congestion_avoidance: Optional[Literal["reno", "cubic", "bbr"]] = Field(default="reno")
    tcp_maxseg: Optional[int] = Field(default=1460)
    tcp_quickack: Optional[StrictBool] = Field(default=False)
    tcp_syncnt: Optional[int] = Field(default=6)


class UDPOption(FLYNCBaseModel):
    """
    UDP options that can be enabled for a connection.

    Parameters
    ----------
    udp_cork : bool, optional
        Enables buffering of UDP messages before they are sent. Defaults to ``False``.
    """

    udp_cork: Optional[StrictBool] = Field(default=False)


class SocketTCP(Socket):
    """
    Represents a TCP socket.

    Parameters
    ----------
    protocol : Literal["tcp"]
        Transport protocol for the socket. Defaults to ``"tcp"``.
    tcp_profile : int
        The unique identifier of the TCP profile whose options are applied to the socket.
    """

    protocol: Literal["tcp"] = Field(default="tcp")
    tcp_profile: int = Field()

    def bind(self, tcp_by_id: dict) -> None:
        if self.tcp_profile not in tcp_by_id:
            warn(
                f"TCP Socket with TCP Option profile ID {self.tcp_profile} does not exist. Creating a profile with default options.",
                category=Category.REFERENCE,
                error_number="084",
            )


class SocketUDP(Socket):
    """
    Represents a UDP socket.

    Parameters
    ----------
    protocol : Literal["udp"]
        Transport protocol for the socket. Defaults to ``"udp"``.
    udp_options : :class:`UDPOption`, optional
        The UDP options that can be configured for this socket.
    """

    protocol: Literal["udp"] = Field(default="udp")
    udp_options: Optional[UDPOption] = Field(default=UDPOption(udp_cork=False))


class IPv4AddressEndpoint(IPv4AddressEntry):
    """
    Represents an IPv4 address endpoint for a network interface.

    Parameters
    ----------
    sockets : list of :class:`~flync.model.flync_4_ecu.sockets.SocketTCP` or \
    :class:`~flync.model.flync_4_ecu.sockets.SocketUDP`
        Assigned TCP and UDP socket endpoints.
    """

    sockets: List[Annotated[SocketTCP | SocketUDP, Field(discriminator="protocol")]] | None = Field(default_factory=list, exclude=True)

    @model_validator(mode="after")
    def check_if_sockets_have_the_same_ip(self):
        """
        Validate that every socket is bound to the same IPv4 address as the one defined in the class.

        Raises:
            err_minor: If any socket's ``endpoint_address`` differs from ``self.address``.
        """

        for socket in self.sockets:
            if str(socket.endpoint_address) != str(self.address):
                raise err_minor("Sockets must be tied to the same address as the IPv4 endpoint.", category=Category.CONSISTENCY, error_number="085")

        return self


class IPv6AddressEndpoint(IPv6AddressEntry):
    """
    Represents an IPv6 address endpoint for a network interface.

    Parameters
    ----------
    sockets : list of :class:`~flync.model.flync_4_ecu.sockets.SocketTCP` or \
    :class:`~flync.model.flync_4_ecu.sockets.SocketUDP`
        Assigned TCP and UDP socket endpoints.
    """

    sockets: List[Annotated[SocketTCP | SocketUDP, Field(discriminator="protocol")]] | None = Field(default_factory=list, exclude=True)

    @model_validator(mode="after")
    def check_if_sockets_have_the_same_ip(self):
        """
        Validate that every socket is bound to the same IPv6 address as the one defined in the class.

        Raises:
            err_minor: If any socket's ``endpoint_address``
            differs from ``self.address``.
        """

        for socket in self.sockets:
            if str(socket.endpoint_address) != str(self.address):
                raise err_minor("Sockets must be tied to the same address as the IPv6 endpoint.", category=Category.CONSISTENCY, error_number="086")
        return self


IPv4AddressEndpoint.model_rebuild()
IPv6AddressEndpoint.model_rebuild()
