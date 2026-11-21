"""
this module contains the necessary datastructures to model a SOME/IP deployment.
"""

import abc
from typing import Annotated, List, Literal, Optional

from pydantic import (
    AfterValidator,
    Field,
    IPvAnyAddress,
    field_serializer,
)

from flync.core.annotations.reference import Reference
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.base_utils import is_ip_multicast
from flync.core.validators.address import validate_ip_multicast
from flync.model.flync_4_someip.service_interface import (
    SDTimings,
    SOMEIPServiceInterface,
)

DeploymentTypes = Literal["someip", "someip_provider", "someip_consumer"]

MAJOR_VERSION_DESCRIPTION = "the major version of this service interface"
MINOR_VERSION_DESCRIPTION = "the minor version of this service interface"


class Layer4Endpoint(FLYNCBaseModel):
    """
    Layer4Endpoint Class method for Layer4 endpoint .

    Parameters
    ----------

    protocol : Literal["UDP", "TCP"]
        Protocol of the Layer4Endpoint.
        Defaults to UDP.

    port : int
        Layer4 Port.
        Must be greater than 0 and less or equal to 65535.
    """

    protocol: Literal["UDP", "TCP"] = "UDP"
    port: Annotated[int, Field(gt=0, le=65535)] = Field(description="the l4-port")


class BaseUDPDeployment(Layer4Endpoint):
    """Base class for deploying a SOME/IP service onto a UDP-endpoint."""

    protocol: Literal["UDP"] = "UDP"


class MulticastEndpoint(BaseUDPDeployment):
    """
    MulticastEndpoint for UDP Deployments.

    Parameters
    ----------

    ip_address : IPvAnyAddress
        IP-Address of the Multicast Endpoint.
    """

    ip_address: Annotated[IPvAnyAddress, is_ip_multicast] = Field()

    @field_serializer("ip_address")
    def serialize_addresses(self, ip_address):
        if ip_address is not None:
            return str(ip_address).upper()


class MulticastSDEndpoint(MulticastEndpoint):
    """
    MulticastSDEndpoint

    Parameters
    ----------

    ip_ttl : int
        IP Time-to-Live.
        Must be greater or equal to 0 and less or equal to 255.
    """

    ip_ttl: Annotated[int, Field(ge=0, le=255)] = Field(description="IP Time-to-Live")


class UDPDeployment(BaseUDPDeployment):
    """
    Allows deploying a SOME/IP service onto a UDP-endpoint (including multicast).

    Parameters
    ----------

    multicast : :class:`~MulticastEndpoint`, optional
        Multicast configuration for this endpoint.
    """

    multicast: Optional["MulticastEndpoint"] = Field(description="multicast configuration for this endpoint", default=None)


class TCPDeployment(Layer4Endpoint):
    """Base class for deploying a SOME/IP service onto a TCP-endpoint"""

    protocol: Literal["TCP"] = "TCP"


class SOMEIPSDDeployment(FLYNCBaseModel):
    """
    Defines the Service Discovery endpoint of SOME/IP.

    Parameters
    ----------

    deployment_type: Literal["someip_sd"]

    multicast : Optional[:class:`~MulticastSDEndpoint`], optional
        Multicast configuration for an SD endpoint.
    """

    deployment_type: Literal["someip_sd"] = Field(default="someip_sd")
    multicast: Optional["MulticastSDEndpoint"] = Field(description="multicast configuration for SD endpoint", default=None)


class SOMEIPServiceDeployment(abc.ABC, FLYNCBaseModel):
    """
    SOMEIPServiceDeployment Create a service deployment that will be used for provided service.

    Parameters
    ----------

    deployment_type : Literal["someip"]

    service : int
        Identifies the service.
        Must be greater than 0 and less than 0xFFFF.

    major_version : int
        The major version of this service interface.
        Must be greater than 0 and less or equal 255. Defaults to 0.

    instance_id: int
        Id of the Service Instance.
        Must be greater than 0 and less than 0xFFFF.

    someip_sd_timings_profile: str
        The SOME/IP timings profile_id used for the deployment.
    """

    deployment_type: DeploymentTypes
    service: Annotated[int, Reference(source="_service_ref")] = Field(description="identifies the service", gt=0, lt=0xFFFF, strict=True)
    major_version: Annotated[int, Field(gt=0, le=255, strict=True)] = Field(description=MAJOR_VERSION_DESCRIPTION, default=0)
    instance_id: Annotated[int, Field(gt=0, lt=0xFFFF)] = Field(description="The id of the service instance")
    someip_sd_timings_profile: str = Field(description="The SOME/IP timings profile ussed for the deployment.")

    _service_ref: SOMEIPServiceInterface | None = None
    _sd_timing_ref: SDTimings | None = None

    @abc.abstractmethod
    def model_post_init(self, __context):
        return super().model_post_init(__context)

    def bind(self, services_by_key: dict, sd_timings_by_id: dict) -> None:
        svc = services_by_key.get((self.service, self.major_version))
        assert svc, f"No service found for id={self.service:#06x}, major_version={self.major_version}"
        self._service_ref = svc
        sd = sd_timings_by_id.get(self.someip_sd_timings_profile)
        assert sd, f"No SD timings profile '{self.someip_sd_timings_profile}'"
        self._sd_timing_ref = sd

    @field_serializer("service")
    def _serialize_field_as_service(self, service):
        if isinstance(service, SOMEIPServiceInterface):
            return service.id
        return service


class SOMEIPServiceConsumer(SOMEIPServiceDeployment):
    """
    Defines the consumer of a SOME/IP service instance (like subscribing & calling methods).

    Parameters
    ----------

    deployment_type : Literal["someip_consumer"]

    major_version : int
        The major version of this service interface.
        Must be greater than 0 and less or equal 255. Defaults to 0.

    consumed_eventgroups : List[str], optional
    """

    deployment_type: Literal["someip_consumer"] = Field(default="someip_consumer")

    major_version: Annotated[int, Field(gt=0, le=255, strict=True)] = Field(description=MAJOR_VERSION_DESCRIPTION, default=0)

    consumed_eventgroups: Optional[List[str]] = Field(default=None)

    def model_post_init(self, __context):
        return super().model_post_init(__context)

    def bind(self, services_by_key: dict, sd_timings_by_id: dict) -> None:
        super().bind(services_by_key, sd_timings_by_id)
        if self.consumed_eventgroups is not None and self._service_ref is not None:
            consumed = set(self.consumed_eventgroups)
            provided = {eg.name for eg in (self._service_ref.eventgroups or [])}
            found = consumed.intersection(provided)
            assert found == consumed, f"Did not find eventgroups with names {consumed - found}"


class SOMEIPEventgroupMulticastConfig(FLYNCBaseModel):
    """
    Multicast Configuration for a SOME/IP Eventgroup.

    Parameters
    ----------
    ip_address : IPvAnyAddress
        Multicast IP address.

    port : int
        Multicast port.
        Must be greater than 0 and less than 0xFFFF.

    threshold : int
        Multicast threshold: number of consumers needed to switch to multicast.
        Must be greater than 0

    eventgroups : List[str]
        Eventgroup names for which this config should apply.
    """

    ip_address: Annotated[IPvAnyAddress, AfterValidator(validate_ip_multicast)] = Field(description="identifies the multicast address")
    port: Annotated[int, Field(gt=0, lt=0xFFFF)] = Field(description="identifies the multicast port")
    threshold: Annotated[int, Field(gt=0)] = Field(description="identifies the multicast threshold")
    eventgroups: List[str] = Field(description="identfies the eventgroups which can be sent via multicast")


class SOMEIPServiceProvider(SOMEIPServiceDeployment):
    """
    Defines the provider of a SOME/IP service instance (like offering & sending responses, events).

    Parameters
    ----------

    deployment_type : Literal["someip_provider"]

    major_version : int
        The major version of this service interface.
        Must be greater than 0 and less than 255.

    minor_version : int
        The minor version of this service interface.
        Must be greater than 0 and less than 0xFFFFFFFF. Defaults to 0.

    provided_eventgroups : List[str], optional
        If set, only the named eventgroups are offered/sent on this socket.
        None means all eventgroups of the service are provided.

    multicast_config : List[:class:`~SOMEIPEventgroupMulticastConfig`], optional
        If set, only the defined eventgroups are configured for multicast.
        None means no eventgroups of the service are configured for multicast.
    """

    deployment_type: Literal["someip_provider"] = Field(default="someip_provider")

    major_version: Annotated[int, Field(gt=0, lt=255, strict=True)] = Field(description=MAJOR_VERSION_DESCRIPTION)

    minor_version: Annotated[int, Field(ge=0, lt=0xFFFFFFFF, strict=True)] = Field(
        description=MINOR_VERSION_DESCRIPTION,
        default=0,
    )

    provided_eventgroups: Optional[List[str]] = Field(default=None)
    multicast_config: Optional[List[SOMEIPEventgroupMulticastConfig]] = Field(default=None)

    def model_post_init(self, __context):
        return super().model_post_init(__context)
