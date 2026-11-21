"""Main module."""

import abc
import warnings
from collections import defaultdict
from collections.abc import Iterator
from typing import (
    Annotated,
    Any,
    List,
    Literal,
    Optional,
    Union,
)

from pydantic import (
    AfterValidator,
    BeforeValidator,
    ConfigDict,
    Field,
    IPvAnyAddress,
    field_serializer,
    model_validator,
)

from flync.core.annotations.external import External, OutputStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_minor
from flync.core.validators.address import validate_ip_multicast
from flync.core.validators.generic import none_to_empty_list
from flync.model.flync_4_metadata import SOMEIPServiceMetadata
from flync.model.flync_4_safety.e2e import E2EConfig
from flync.model.flync_4_someip.someip_complex_datatypes import AllTypes


class SOMEIPFieldTimings(FLYNCBaseModel):
    """
    Timings for field.

    Parameters
    ----------

    profile_id : str
        Name of the timing profile.

    type : Literal["field"]
        Type of this timing.

    getter_req_debounce : int
        Minimum time in milliseconds between 2 request messages for same getter method/service.
        Defaults to 0.

    getter_req_max_retention : int
        Maximum time in milliseconds indicating how long a getter request may be withheld.
        Defaults to 0.

    getter_res_max_retention: int
        Maximum time in milliseconds indicating how long a getter response may be withheld.
        Defaults to 0.

    setter_req_debounce: int
        Minimum time in milliseconds between 2 request messages for same setter method/service.
        Defaults to 0.

    setter_req_max_retention: int
        Maximum time in milliseconds indicating how long a setter request may be withheld.
        Defaults to 0.

    setter_res_max_retention: int
        Maximum time in milliseconds indicating how long a setter response may be withheld.
        Defaults to 0.

    notifier_debounce: int
        Minimum time in milliseconds between 2 notification messages for same service.
        Defaults to 0.

    notifier_max_retention: int
        Maximum time in milliseconds indicating how long a notification may be withheld.
        Defaults to 0.
    """

    profile_id: str = Field(description="Timing profile for fields.")
    type: Literal["field"] = Field(default="field")
    getter_req_debounce: int = Field(
        description="Minimum time in milliseconds between 2 request messages for same getter method/service.",
        default=0,
    )
    getter_req_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a getter request may be withheld.",
        default=0,
    )
    getter_res_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a getter response may be withheld.",
        default=0,
    )

    setter_req_debounce: int = Field(
        description="Minimum time in milliseconds between 2 request messages for same setter method/service.",
        default=0,
    )
    setter_req_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a setter request may be withheld.",
        default=0,
    )
    setter_res_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a setter response may be withheld.",
        default=0,
    )

    notifier_debounce: int = Field(
        description="Minimum time in milliseconds between 2 notification messages for same service.",
        default=0,
    )
    notifier_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a notification may be withheld.",
        default=0,
    )


class SOMEIPEventTimings(FLYNCBaseModel):
    """
    Timings for event.

    Parameters
    ----------

    profile_id : str
        Name of the timing profile.

    type : Literal["event"]
        Type of this timing.

    debounce: int
        Minimum time in milliseconds between 2 event messages for same service.
        Defaults to 0.

    max_retention: int
        Maximum time in milliseconds indicating how long an event may be withheld.
        Defaults to 0.
    """

    profile_id: str = Field(description="Timing profile for events.")
    type: Literal["event"] = Field(default="event")
    debounce: int = Field(
        description="Minimum time in milliseconds between 2 event messages for same service.",
        default=0,
    )
    max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long an event may be withheld.",
        default=0,
    )


class SOMEIPMethodTimings(FLYNCBaseModel):
    """
    Timings for method invocation.

    Parameters
    ----------

    profile_id : str
        Name of the timing profile.

    type : Literal["method"]
        Type of this timing.

    req_debounce : int
        Minimum time in milliseconds between 2 request messages for same method/service.
        Defaults to 0.

    req_max_retention : int
        Maximum time in milliseconds indicating how long a request may be withheld.
        Defaults to 0.

    res_max_retention: int
        Maximum time in milliseconds indicating how long a response may be withheld.
        Defaults to 0.
    """

    profile_id: str = Field(description="Timing profile for methods.")
    type: Literal["method"] = Field(default="method")
    req_debounce: int = Field(
        description="Minimum time in milliseconds between 2 request messages for same method/service.",
        default=0,
    )
    req_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a request may be withheld.",
        default=0,
    )
    res_max_retention: int = Field(
        description="Maximum time in milliseconds indicating how long a response may be withheld.",
        default=0,
    )


SOMEIPEvenTimingsProfilesUnion = Annotated[Union[SOMEIPEventTimings | SOMEIPFieldTimings | SOMEIPMethodTimings], Field(discriminator="type")]


class SOMEIPTimingProfile(FLYNCBaseModel):
    """
    Timings for field.

    Parameters
    ----------

    profiles : List[ :class:`~SOMEIPEventTimings` | :class:`~SOMEIPFieldTimings` | :class:`~SOMEIPMethodTimings` ]
        List of timing profiles for SOME/IP.

    defaults : List[ :class:`~SOMEIPEventTimings` | :class:`~SOMEIPFieldTimings` | :class:`~SOMEIPMethodTimings` ]
        List of default timing profiles for SOME/IP.
    """

    profiles: Annotated[
        List[SOMEIPEvenTimingsProfilesUnion],
        Field(description="List of timing profiles for SOME/IP"),
    ] = Field(default_factory=list)

    defaults: Annotated[
        List[SOMEIPEvenTimingsProfilesUnion],
        Field(description="List of default timing profiles for SOME/IP"),
    ] = Field(default_factory=list)


class SOMEIPField(FLYNCBaseModel):
    """
    Base datastructure to design a SOME/IP Field

    Parameters
    ----------
    name : str
        Name of the Field.

    parameters : list of :class:`~SOMEIPParameter`, optional
        List of Parameters of the Field.
        Defaults to an empty list.

    description : str, optional
        Description of the Field.

    notifier_id : int, optional
        Identifies the Field Notifier.
        Must be greater than 0 and lower or equal than 0xFFFF.
        Defaults to 1.

    setter_id : int, optional
        Identifies the Field Setter.
        Must be greater than 0 and lower or equal than 0xFFFF.

    getter_id : int, optional
        Identifies the Field Getter.
        Must be greater than 0 and lower or equal than 0xFFFF.

    reliable : bool
        Indicates whether the event is transmitted reliably.
        Defaults to False.

    notifier_e2e : :class:`~E2EConfig`, optional
        E2E configuration for the field notifier.

    someip_timing : str, optional
        Name of the timing definition.
        Defaults to "field_default".
    """

    model_config = ConfigDict(frozen=True)
    name: str = Field(description="name of the field")
    parameters: Annotated[
        Optional[List["SOMEIPParameter"]],
        Field(description="name of the parameter"),
    ] = Field(default=[])
    description: Optional[str] = Field(default="")

    notifier_id: Optional[Annotated[int, Field(gt=0, le=0xFFFF, strict=True)]] = Field(description="identifies the field setter", default=None)
    setter_id: Optional[Annotated[int, Field(gt=0, le=0xFFFF, strict=True)]] = Field(description="identifies the field setter", default=None)
    getter_id: Optional[Annotated[int, Field(gt=0, le=0xFFFF, strict=True)]] = Field(description="identifies the field getter", default=None)

    reliable: bool = Field(default=False)
    notifier_e2e: Optional[E2EConfig] = Field(default=None)
    someip_timing: Optional[str] = Field(description="SOME/IP timings for the field", default="field_default")

    @property
    def id(self):
        return self.notifier_id

    @model_validator(mode="after")
    def validate_at_least_one_identifier_to_be_defined(self):
        """
        Validate that at least one identifier of the
        field is defined. [feat_req_someip_632]"""

        if self.notifier_id is None and self.setter_id is None and self.getter_id is None:
            raise err_minor(
                f'Field "{self.name}": [feat_req_someip_632] - '
                "A field without a setter and without a getter and without a notifier shall not exist.",
                category=Category.REQUIRED,
                error_number="135",
            )
        return self


class SOMEIPParameter(FLYNCBaseModel):
    """
    Definition of Parameters for SOME/IP.

    Parameters
    ----------

    name : str
        Identifies the parameter.

    description : str, optional
        Human-readable description of the datatype.

    datatype : :class:`~flync.model.flync_4_someip.someip_complex_datatypes.AllTypes`
        Datatype of the Parameter.
    """

    name: str = Field(description="identifies the parameter")
    description: Optional[str] = Field("", description="Optional description")
    datatype: "AllTypes"


class SOMEIPEvent(FLYNCBaseModel):
    """
    Defines a SOME/IP event definition.

    This model is used to describe a SOME/IP event, including its identifier,
    reliability settings, optional E2E protection configuration, and the list
    of parameters that the event transports.

    Parameters
    ----------
    name : str
        Name of the event.

    description : str, optional
        Human-readable description of the event.

    id : int
        Unique identifier of the event.
        Must be greater than 0 and lower or equal than 0xFFFF.

    reliable : bool
        Indicates whether the event is transmitted reliably.
        Defaults to False.

    e2e : :class:`~E2EConfig`, optional
        E2E configuration for the event.

    parameters : list of :class:`~SOMEIPParameter`, optional
        Parameters of the event.
        Defaults to an empty list.

    someip_timing : str, optional
        Name of the timings definition.
        Defaults to "event_default".
    """

    model_config = ConfigDict(frozen=True)
    name: str = Field(description="name of the event")
    description: Optional[str] = Field(default="")
    id: Annotated[int, Field(gt=0, le=0xFFFF, strict=True)] = Field(description="identifies the event")
    reliable: bool = Field(default=False)
    e2e: Optional[E2EConfig] = Field(default=None)
    parameters: Annotated[
        Optional[List["SOMEIPParameter"]],
        Field(description="name of the parameter"),
    ] = Field(default=[])
    someip_timing: Optional[str] = Field(default="event_default")


class SOMEIPEventgroup(FLYNCBaseModel):
    """
    Main datastructure to model a SOME/IP Eventgroup.

    Parameters
    ----------
    name : str
        Name of the Eventgroup.

    description : str, optional

    id : int
        Identifies the Eventgroup.
        Must be greater than 0 and lower or equal than 0xFFFF.

    events: list[:class:`~SOMEIPEvent` | :class:`~SOMEIPField`]
        The events and fields this eventgroup contains.
    """

    model_config = ConfigDict(frozen=True)
    name: str = Field(description="name of the eventgroup")
    description: Optional[str] = Field(default="")
    id: Annotated[int, Field(gt=0, le=0xFFFF, strict=True)] = Field(description="identifies the eventgroup")
    events: List[SOMEIPEvent | SOMEIPField] = Field(description="the events this eventgroup contains")

    @property
    def fields(self) -> List[SOMEIPField]:
        """fields Returns the list of fields that this eventgroup contains."""
        return [e for e in self.events if isinstance(e, SOMEIPField)]


class SOMEIPTP(FLYNCBaseModel):
    """
    SOME/IP Transport Protocol configuration.

    Parameters
    ----------

    enabled : bool
        Indicates whether SOME/IP-TP is enabled or not.
        Defaults to False.

    max_segment_length : int
        maximum segment length.
        Defaults to 0.
    """

    enabled: bool = Field(default=False)
    max_segment_length: int = Field(default=0, description="maximum segment length")


class SOMEIPMethod(FLYNCBaseModel):
    """
    Datastructure to model SOME/IP methods.

    Parameters
    ----------
    name : str
        Name of the Method.

    description : str, optional
        Description of the Method.

    type : Literal["request_response", "fire_and_forget"]
        Type of the Method.

    id : int
        Unique method identifier for the service interface.
        Must be greater than 0 and lower or equal than 0xFFFF.

    reliable : bool
        Indicates whether the event is transmitted reliably.
        Defaults to False.

    someip_tp : :class:`~SOMEIPTP`, optional
        SOME/IP Transport Protocol configuration for this method.

    input_parameters : list of :class:`~SOMEIPParameter`, optional
        The parameters of the Request.
        Defaults to an empty list.

    someip_timing : str, optional
        Name of the timings definition.
        Defaults to "method_default".
    """

    name: str = Field(description="name of the method")
    description: Optional[str] = Field(description="a short description on what this method does", default="")
    type: Literal["request_response", "fire_and_forget"]
    id: Annotated[int, Field(gt=0, le=0xFFFF, strict=True)] = Field(
        gt=0,
        strict=True,
        description="method identifier unique for the service interface",
    )
    reliable: bool = Field(default=False)
    someip_tp: Optional[SOMEIPTP] = Field(
        description="SOME/IP Transport Protocol configuration for this method",
        default=None,
    )
    input_parameters: Annotated[
        Optional[List["SOMEIPParameter"]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])

    someip_timing: Optional[str] = Field(default="method_default")

    @abc.abstractmethod
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


class SOMEIPRequestResponseMethod(SOMEIPMethod):
    """
    Allows to model SOME/IP methods which will return a response.

    Parameters
    ----------

    output_parameters : list of :class:`~SOMEIPParameter`, optional
        The parameters of the Response

    """

    type: Literal["request_response"] = "request_response"
    output_parameters: Annotated[
        Optional[List["SOMEIPParameter"]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


class SOMEIPFireAndForgetMethod(SOMEIPMethod):
    """
    Allows to model SOME/IP methods which will not return a response.
    """

    type: Literal["fire_and_forget"] = "fire_and_forget"

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)


class SOMEIPServiceInterface(FLYNCBaseModel):
    """
    Class to create a SOME/IP service interface definition.

    Parameters
    ----------

    name : str
        Name of the service

    description : str, optional

    id : int
        Unique identifier for the service.
        Must be greater than 0 and lower or equal than 0xFFFF.

    major_version : int
        The major version of this service interface.
        Must be greater than 0 and lower or equal than 0xFF.
        Defaults to 0.

    minor_version : int
        The minor version of this service interface.
        Must be greater than 0 and lower or equal than 0xFFFFFFFF.
        Defaults to 0.

    fields : List[ :class:`~SOMEIPField`], optional
        Fields of the service.
        Defaults to an empty list.

    events : List[:class:`~SOMEIPEvent`], optional
        Events of the service.
        Defaults to an empty list.

    eventgroups : List[:class:`~SOMEIPEventgroup`], optional
        Eventgroups of the service.
        Defaults to an empty list.

    methods : List[ :class:`~SOMEIPFireAndForgetMethod` | \
    :class:`~SOMEIPRequestResponseMethod` ], optional
        Methods of the service.
        Defaults to an empty list.

    meta : :class:`~flync.model.flync_4_metadata.SOMEIPServiceMetadata`
        Metadata for the SOME/IP Service.
    """

    name: str = Field(description="name of the service")
    description: Optional[str] = Field(default="")
    id: Annotated[int, Field(gt=0, le=0xFFFF, strict=True)] = Field(description="identifies the service")

    major_version: Annotated[int, Field(ge=0, le=0xFF, strict=True)] = Field(description="the major version of this service interface", default=0)
    minor_version: Annotated[int, Field(ge=0, le=0xFFFFFFFF, strict=True)] = Field(
        description="the minor version of this service interface",
        default=0,
    )
    fields: Annotated[
        Optional[List[SOMEIPField]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[], description="fields of the service")
    events: Annotated[
        Optional[List[SOMEIPEvent]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[], description="events of the service")
    eventgroups: Annotated[
        Optional[List[SOMEIPEventgroup]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[], description="eventgroups of the service")
    methods: Annotated[
        List[Annotated[SOMEIPFireAndForgetMethod | SOMEIPRequestResponseMethod, Field(discriminator="type")]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[], description="methods of the service")
    meta: SOMEIPServiceMetadata = Field()

    @model_validator(mode="before")
    @classmethod
    def resolve_eventgroup_event_refs(cls, data):
        """Resolve event/field name strings in eventgroups before child validation."""
        if not isinstance(data, dict):
            return data
        all_by_name = {e["name"]: e for e in (data.get("events") or []) + (data.get("fields") or []) if isinstance(e, dict)}
        for eg in data.get("eventgroups") or []:
            if isinstance(eg, dict):
                eg["events"] = [all_by_name.get(e, e) if isinstance(e, str) else e for e in eg.get("events", [])]
        return data

    @model_validator(mode="after")
    def validate_for_notifiers_without_eventgroup(self):
        """Validate that all notifiers are in at least one eventgroup."""
        all_notifiers_in_eg = []
        for eg in self.eventgroups:
            all_notifiers_in_eg.extend(eg.events)
        field_notifiers = [f for f in self.fields if f.notifier_id is not None]
        for notifier in self.events + field_notifiers:
            if notifier not in all_notifiers_in_eg:
                warnings.warn(
                    f"Notifier '{notifier.name}' is not assigned to an eventgroup",
                    stacklevel=2,
                )
        return self

    @model_validator(mode="after")
    def validate_service_owns_elements_of_eventgroups(self):
        """
        Validate that eventgroups' elements are in events/fields of service."""

        for eg in self.eventgroups:
            for event in eg.events:
                if event not in ((self.events or []) + (self.fields or [])):
                    raise err_minor(
                        f'Eventgroup references "{event.name}" which is not in events/fields of Service "{self.name}"',
                        category=Category.REFERENCE,
                        error_number="136",
                    )
        return self

    @model_validator(mode="after")
    def validate_all_identifiers_to_be_unique(self):
        """
        Validate that all identifiers in service
        are unique. [feat_req_someip_56]"""

        ids = {}
        for event in self.events:
            ids.setdefault(event.id, []).append(("id", event))
        for field in self.fields:
            ids.setdefault(field.notifier_id, []).append(("notifier_id", field))
            ids.setdefault(field.getter_id, []).append(("getter_id", field))
            ids.setdefault(field.setter_id, []).append(("setter_id", field))
        for method in self.methods:
            ids.setdefault(method.id, []).append(("id", method))
        for identifier in ids:
            if identifier is None:
                continue
            entries = ids[identifier]
            if len(entries) > 1:
                raise err_minor(
                    f"Entities share same identifier: {identifier} | "
                    + ", ".join([f"'{entity.name}'({type(entity).__name__}.{attr_name})" for attr_name, entity in entries])
                    + " [feat_req_someip_56]",
                    category=Category.UNIQUENESS,
                    error_number="137",
                )
        return self


class SDTimings(FLYNCBaseModel):
    """
    Configurations for SOME/IP-SD Timings.

    Parameters
    ----------

    profile_id : str
        A unique ID for the SOME/IP-SD timings profile.

    initial_delay_min : int
        Initial delay in milliseconds: This parameter keeps back service offers to pack more entries together.
        Must be greater or equal to 0.
        Defaults to 10.

    initial_delay_max : int
        Initial delay in milliseconds: This parameter keeps back service offers to pack more entries together.
        Must be greater or equal to 0.
        Defaults to 10.

    repetitions_base_delay : int
        Repetitions Base delay in milliseconds: This parameter helps in fast startup and to make startup more robust.
        Loss of the first offer results in this delay.
        Must be greater or equal to 0.
        Defaults to 30.

    repetitions_max : int
        Number of repetitions while doubling delay.
        Must be greater or equal to 0.
        Defaults to 3.

    request_response_delay_min : int
        Request response delay in milliseconds: This parameter keeps back subscribes to pack more entries together.
        Must be greater or equal to 0.
        Defaults to 10.

    request_response_delay_max : int
        Request response delay in milliseconds: This parameter keeps back subscribes to pack more entries together.",
        Must be greater or equal to 0.
        Defaults to 10.

    offer_cyclic_delay: int, optional
        Offer cyclic delay in milliseconds: This parameter keeps system alive with cyclic offer.
        Must be greater or equal to 0.
        Defaults to 1000.

    offer_ttl : int, optional
        Time to live in milliseconds: This parameter determines how fast to age out state.
        Must be greater or equal to 0.
        Defaults to 3000.

    find_ttl : int, optional
        Offer cyclic delay in milliseconds: This parameter keeps system alive with cyclic offer.
        Must be greater or equal to 0.
        Defaults to 1000.

    subscribe_ttl: int, optional
        Time to live in milliseconds: This parameter determines how fast to age out state.
        Must be greater or equal to 0.
        Defaults to 3.
    """

    profile_id: str = Field(description="A unique ID for the SOME/IP-SD timings profile")

    initial_delay_min: int = Field(
        ge=0,
        description="Initial delay in milliseconds: This parameter keeps back service offers to pack more entries together.",
        default=10,
    )

    initial_delay_max: int = Field(
        ge=0,
        description="Initial delay in milliseconds: This parameter keeps back service offers to pack more entries together.",
        default=10,
    )

    repetitions_base_delay: int = Field(
        ge=0,
        description="Repetitions Base delay in milliseconds: This parameter helps in fast startup and to make startup more \
                robust.Loss of the first offer results in this delay.",
        default=30,
    )
    repetitions_max: int = Field(
        ge=0,
        description="Number of repetitions while doubling delay",
        default=3,
    )

    request_response_delay_min: int = Field(
        ge=0,
        description="Request response delay in milliseconds: This parameter keeps back subscribes to pack more entries together.",
        default=10,
    )

    request_response_delay_max: int = Field(
        ge=0,
        description="Request response delay in milliseconds: This parameter keeps back subscribes to pack more entries together.",
        default=10,
    )

    offer_cyclic_delay: Optional[int] = Field(
        ge=0,
        description="Offer cyclic delay in milliseconds: This parameter keeps system alive with cyclic offer.",
        default=1000,
    )

    offer_ttl: Optional[int] = Field(
        ge=0,
        description="Time to live in milliseconds: This parameter determines how fast to age out state.",
        default=3000,
    )

    find_ttl: Optional[int] = Field(
        ge=0,
        description="Offer cyclic delay in milliseconds: This parameter keeps system alive with cyclic offer.",
        default=1000,
    )

    subscribe_ttl: Optional[int] = Field(
        ge=0,
        description="Time to live in milliseconds: This parameter determines how fast to age out state.",
        default=3000,
    )


class SDConfig(FLYNCBaseModel):
    """
    allows to configure the SOME/IP Service-Discovery. Represent from the Chapter SD, the Endpoint and SD Endpoint.

    Parameters
    ----------

    ip_address : :class:`~IPvAnyAddress`
        IP on which the service discovery operates.

    port : int
        Port which the service discovery operates on.
        Must be greater than 0 and lower than 0xFFFF.
        Defaults to 30490.

    sd_timings : List[ :class:`~SDTimings` ]
        Timing Configurations for SOME/IP-SD.
    """

    ip_address: Annotated[IPvAnyAddress, AfterValidator(validate_ip_multicast)] = Field(description="IP on which the service discovery operates")
    port: Annotated[int, Field(gt=0, lt=0xFFFF)] = Field(
        default=30490,
        description="port which the service discovery operates on",
    )
    sd_timings: List[SDTimings] = Field()

    @field_serializer("ip_address")
    def serialize_addresses(self, ip_address):
        if ip_address is not None:
            return str(ip_address).upper()


class SOMEIPConfig(FLYNCBaseModel):
    """
    Basic configuration of SOME/IP for a target system.

    Parameters
    ----------

    version: Literal[ "1.0" ]
        The version of this config.

    sd_config: :class:`~SDConfig`
        Configuration of the Service-Discovery.

    services: list[ :class:`~SOMEIPServiceInterface` ]
        List of SOME/IP Services.

    someip_timings: :class:`~SOMEIPTimingProfile`
        Configuration of the SOME/IP timings.
    """

    version: Literal["1.0"] = Field(default="1.0", description="the version of this config")

    sd_config: Annotated[
        SDConfig,
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(description="configuration of the service discovery")
    services: Annotated[List[SOMEIPServiceInterface], External()] = Field(description="list of services")
    someip_timings: Annotated[
        SOMEIPTimingProfile,
        External(output_structure=OutputStrategy.SINGLE_FILE | OutputStrategy.OMMIT_ROOT),
    ] = Field(description="configuration of the SOME/IP timings")

    @staticmethod
    def _iter_e2e_elements(service: SOMEIPServiceInterface) -> Iterator[tuple[Any, E2EConfig]]:
        """
        Yields all (element, e2e) pairs of a service that carry an E2E configuration.
        """

        # fields/events are Optional in the annotation only, a BeforeValidator maps None to []
        for field in service.fields or []:
            if field.notifier_e2e is not None:
                yield field, field.notifier_e2e
        for event in service.events or []:
            if event.e2e is not None:
                yield event, event.e2e

    @model_validator(mode="after")
    def validate_all_e2e_identifiers_to_be_unique(self):
        """
        Validates that for each E2E profile all e2e.data_id values are unique.
        """

        # Mapping: profile -> data_id -> List[(service, element)]
        per_profile: dict[str, dict[int, list[tuple[Any, Any]]]] = defaultdict(lambda: defaultdict(list))

        for service in self.services:
            for element, e2e in self._iter_e2e_elements(service):
                per_profile[e2e.profile][e2e.data_id].append((service, element))

        errors = [
            f"Duplicate e2e.data_id '{data_id}' in Profil '{profile}': "
            + ", ".join(f"{type(service).__name__}.{type(element).__name__}" for service, element in entries)
            for profile, by_id in per_profile.items()
            for data_id, entries in by_id.items()
            if len(entries) > 1
        ]

        if errors:
            raise ValueError(" | ".join(errors))

        return self

    @model_validator(mode="after")
    def validate_timing_exist(self):
        all_timings = self.someip_timings.profiles + self.someip_timings.defaults
        field_ids = {t.profile_id for t in all_timings if isinstance(t, SOMEIPFieldTimings)}
        event_ids = {t.profile_id for t in all_timings if isinstance(t, SOMEIPEventTimings)}
        method_ids = {t.profile_id for t in all_timings if isinstance(t, SOMEIPMethodTimings)}
        for service_inst in self.services:
            for service_element in service_inst.events + service_inst.fields + service_inst.methods:
                if service_element.someip_timing is not None:
                    if isinstance(service_element, SOMEIPField) and service_element.someip_timing not in field_ids:
                        raise ValueError(
                            f"{service_element.id} - "
                            f"{service_element.name}.someip_timing "
                            f'"{service_element.someip_timing}" '
                            "dont exist in SOMEIPFieldTimings"
                        )
                    elif isinstance(service_element, SOMEIPEvent) and service_element.someip_timing not in event_ids:
                        raise ValueError(
                            f"{service_element.id} - "
                            f"{service_element.name}.someip_timing "
                            f'"{service_element.someip_timing}" '
                            "dont exist in SOMEIPEventTimings"
                        )
                    elif isinstance(service_element, SOMEIPMethod) and service_element.someip_timing not in method_ids:
                        raise ValueError(
                            f"{service_element.id} - "
                            f"{service_element.name}.someip_timing "
                            f'"{service_element.someip_timing}" '
                            "dont exist in SOMEIPMethodTimings"
                        )
        return self
