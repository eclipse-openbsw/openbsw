"""Defines the Application model for FLYNC including its service provider and consumer references."""

from typing import Annotated, List, Literal, Optional

from pydantic import Field, model_validator

from flync.core.annotations import (
    Implied,
    ImpliedStrategy,
)
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, warn


class ServiceConsumerReference(FLYNCBaseModel):
    """
    Reference to resolve a SOME/IP Consumer Instance.

    Parameters
    ----------
    type: Literal["consumer"]
        Type of the service reference.

    service_id: int
        Service ID of the referenced service instance (> 0 and < 65535).

    instance_id: int
        Instance ID of the referenced service instance.

    major_version: int
        Major version of the referenced service instance.
    """

    type: Literal["consumer"] = Field(default="consumer", description="Type of the service reference.")
    service_id: Annotated[int, Field(gt=0, lt=0xFFFF)] = Field(description="Service ID of the referenced service instance.")
    instance_id: int = Field(description="Instance ID of the referenced service instance.")
    major_version: int = Field(description="Major version of the referenced service instance.")


class ServiceProviderReference(FLYNCBaseModel):
    """
    Reference to resolve a SOME/IP Provider Instance.

    Parameters
    ----------
    type: Literal["provider"]
        Type of the service reference.

    service_id: int
        Service ID of the referenced service instance (> 0 and < 65535).

    instance_id: int
        Instance ID of the referenced service instance.

    major_version: int
        Major version of the referenced service instance.
    """

    type: Literal["provider"] = Field(default="provider", description="Type of the service reference.")
    service_id: Annotated[int, Field(gt=0, lt=0xFFFF)] = Field(description="Service ID of the referenced service instance.")
    instance_id: int = Field(description="Instance ID of the referenced service instance.")
    major_version: int = Field(description="Major version of the referenced service instance.")


class App(FLYNCBaseModel):
    """
    Definition of an application in the system.

    Parameters
    ----------
    name: str
        Name of this application. Implied from filename.

    service_consumer_refs: list of :class:`~ServiceConsumerReference`
        Reference of all Consumer Instances of this application.

    service_provider_refs: list of :class:`~ServiceProviderReference`
        Reference of all Provider Instances of this application.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field(description="Name of this application.")
    service_consumer_refs: Optional[List[ServiceConsumerReference]] = Field(
        description="Reference of all Consumer Instances of this application.", default_factory=list
    )
    service_provider_refs: Optional[List[ServiceProviderReference]] = Field(
        description="Reference of all Provider Instances of this application.", default_factory=list
    )

    @model_validator(mode="after")
    def warn_self_consumed_instances(self):
        """Warn when a service instance is referenced in both service_consumer_refs and service_provider_refs."""
        provided = {(ref.service_id, ref.instance_id, ref.major_version) for ref in self.service_provider_refs or []}
        for ref in self.service_consumer_refs or []:
            if (ref.service_id, ref.instance_id, ref.major_version) in provided:
                warn(
                    f"App '{self.name}' both consumes and provides the same service instance "
                    f"(service_id={ref.service_id:#06x}, instance_id={ref.instance_id}, major_version={ref.major_version}).",
                    category=Category.CONSISTENCY,
                    error_number="242",
                )
        return self
