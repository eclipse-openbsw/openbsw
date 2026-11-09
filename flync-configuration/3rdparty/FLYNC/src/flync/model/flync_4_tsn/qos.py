"""
Define QoS models in FLYNC.
"""

from ipaddress import IPv4Address, IPv6Address
from typing import Annotated, List, Literal, Optional, Self

from pydantic import (
    BeforeValidator,
    Field,
    PlainSerializer,
    field_serializer,
    field_validator,
    model_validator,
)
from pydantic_extra_types.mac_address import MacAddress

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.datatypes import (
    Ethertype,
    IPv4AddressEntry,
    IPv6AddressEntry,
    MACAddressEntry,
    ValueRange,
    serialize_ethertype,
    validate_ethertype_input,
)
from flync.core.utils.exceptions import Category, err_minor
from flync.core.validators.address import validate_vlan_id
from flync.core.validators.generic import none_to_empty_list


class ATSInstance(FLYNCBaseModel):
    """
    Defines an ATS (Asynchronous Traffic Shaping) instance configuration for an ingress stream. ATS shaping is applied on egress when an
    :class:`ATSShaper` is configured.

    Parameters
    ----------
    committed_information_rate : int
        Guaranteed data rate in kilobits per second (kbps).
        Must be greater or equal to 0.

    committed_burst_size : int
        Maximum burst size allowed within the committed rate in kilobytes (kB).
        Must be greater or equal to 0.

    max_residence_time : int
        Maximum time a frame can reside within the switch in microseconds (µs).
        Must be greater or equal to 0.
    """

    committed_information_rate: int = Field(..., ge=0)
    committed_burst_size: int = Field(..., ge=0)
    max_residence_time: int = Field(..., ge=0)


class ATSShaper(FLYNCBaseModel):
    """
    Specifies a shaper using the ATS (Asynchronous Traffic Shaping) mechanism.

    This shaper applies to a specific stream ATS instance configured on an :class:`ATSInstance`.

    Parameters
    ----------
    type : Literal["ats"]
        Literal identifier specifying the shaper type.
        Defaults to "ats" for schema identification.
    """

    type: Literal["ats"] = Field(default="ats")


class CBSShaper(FLYNCBaseModel):
    """
    Credit-Based Shaper (CBS) configuration model.

    This class represents the configuration for a CBS traffic shaping mechanism, which manages bandwidth allocation and latency for
    time-sensitive networking (TSN) streams.
    CBS uses a credit-based algorithm to control frame transmission rates.

    Parameters
    ----------
    type : Literal["cbs"]
        Literal identifier specifying the shaper type.
        Defaults to "cbs" for schema identification.

    idleslope : int
        The rate at which credits accumulate when the stream is not transmitting. Determines bandwidth allocation.
        Value specified in kilobits per second (kbps).
        Must be between 0 and 1000000.
    """

    type: Literal["cbs"] = Field(default="cbs")
    idleslope: int = Field(..., ge=0, le=1000000)


class SingleRateTwoColorMarker(FLYNCBaseModel):
    """
    Policer model implementing a single-rate, two-color marker.

    This model marks traffic based on a single committed rate (CIR), with two traffic colors: conforming and exceeding.
    It does not support excess information rate (EIR) and operates without coupling.

    Parameters
    ----------
    type : Literal["single_rate_two_color"]
        Literal identifier specifying the policer type.
        Defaults to "single_rate_two_color" for schema identification.

    cir : int
        Committed Information Rate in kilobits per second (kbps).
        Must be greater than 0.

    cbs : int
        Committed Burst Size in kilobytes (kB).
        Must be greater than 0.

    eir : Literal[0]
        Excess Information Rate.
        Fixed at 0 for this model.

    ebs : int
        Excess Burst Size in kilobytes (kB).
        Must be greater than or equal to 0.

    coupling : Literal[False]
        Coupling flag.
        Disabled for the two-color model.
    """

    type: Literal["single_rate_two_color"] = Field(default="single_rate_two_color")
    cir: int = Field(..., gt=0)
    cbs: int = Field(..., gt=0)
    eir: Literal[0] = Field(0)
    ebs: int = Field(..., ge=0)
    coupling: Literal[False] = Field(False)


class SingleRateThreeColorMarker(FLYNCBaseModel):
    """
    Policer model implementing a single-rate, three-color marker.

    This model uses a single committed rate (CIR) and marks packets as conforming, exceeding, or violating based on bucket fill levels.
    Coupling is enabled to allow the excess bucket to draw from the committed bucket.

    Parameters
    ----------
    type : Literal["single_rate_three_color"]
        Literal identifier specifying the policer type.
        Defaults to "single_rate_three_color" for schema identification.

    cir : int
        Committed Information Rate in kilobits per second (kbps).
        Must be greater than 0.

    cbs : int
        Committed Burst Size in kilobytes (kB).
        Must be greater than 0.

    eir : Literal[0]
        Excess Information Rate.
        Fixed at 0 for this model.

    ebs : int
        Excess Burst Size in kilobytes (kB).
        Must be greater than 0.

    coupling : Literal[True]
        Coupling flag.
        Enabled for three-color behavior.
    """

    type: Literal["single_rate_three_color"] = Field(default="single_rate_three_color")
    cir: int = Field(..., gt=0)
    cbs: int = Field(..., gt=0)
    eir: Literal[0] = Field(0)
    ebs: int = Field(..., gt=0)
    coupling: Literal[True] = Field(True)


class DoubleRateThreeColorMarker(FLYNCBaseModel):
    """
    Policer model implementing a double-rate, three-color marker.

    This model supports both committed and excess information rates (CIR and EIR) and marks traffic with three colors:
    conforming, exceeding, and violating.
    Coupling behavior is configurable.

    Parameters
    ----------
    type : Literal["double_rate_three_color"]
        Literal identifier specifying the policer type.
        Defaults to "double_rate_three_color" for schema identification.

    cir : int
        Committed Information Rate in kilobits per second (kbps).
        Must be greater than 0.

    cbs : int
        Committed Burst Size in kilobytes (kB).
        Must be greater than 0.

    eir : int
        Excess Information Rate in kilobits per second (kbps).
        Must be greater than 0.

    ebs : int
        Excess Burst Size in kilobytes (kB).
        Must be greater than 0.

    coupling : bool, optional
        Coupling flag (defaults to ``True``).
        Determines whether the excess bucket draws from the committed
        bucket.
    """

    type: Literal["double_rate_three_color"] = Field(default="double_rate_three_color")
    cir: int = Field(..., gt=0)
    cbs: int = Field(..., gt=0)
    eir: int = Field(..., gt=0)
    ebs: int = Field(..., gt=0)
    coupling: bool = Field(default=True)


class FrameFilter(FLYNCBaseModel):
    """
    Defines filtering rules for frames based on MAC/IP addresses, VLAN, and transport protocol ports.
    On embedded devices (e.g. in the TCAM rules of a Switch) frame filters are usually restricted to single values, so value ranges and lists
    may not be supported there.

    Parameters
    ----------

    ethertype : :class:`~flync.core.datatypes.ethertypes.Ethertype` or \
    list of :class:`~flync.core.datatypes.ethertypes.Ethertype`, optional
        EtherType(s) to filter by. Accepts an :class:`Ethertype` member, its
        name (e.g. ``"AVTP"``) or its numeric value (e.g. ``0x22F0``).

    src_mac : :class:`MacAddress` or :class:`~flync.core.datatypes.MACAddressEntry` \
    or list of  (MacAddress | :class:`~flync.core.datatypes.MACAddressEntry`), optional
        Source MAC address(es) to filter by.
        Acceptable formats are ``xx:xx:xx:xx:xx:xx`` or ``xx-xx-xx-xx-xx-xx`` (hexadecimal).

    dst_mac : :class:`MacAddress` or :class:`~flync.core.datatypes.MACAddressEntry` \
    or list of (MacAddress | :class:`~flync.core.datatypes.MACAddressEntry`), optional
        Destination MAC address(es) to filter by.
        Same format rules as ``src_mac``.

    vlan_tagged : bool, optional
        Whether the frame has a 802.1Q VLAN tag.

    vlanid : int or :class:`~flync.core.datatypes.value_range.ValueRange` or \
    list of (int | :class:`~flync.core.datatypes.value_range.ValueRange`), optional
        VLAN Identifier(s) to match.
        Integer values must be in the range 0-4094; 4095 is reserved by IEEE 802.1Q and emits a warning when used.
        ``None`` matches untagged frames. ``ValueRange`` can specify an inclusive interval.

    pcp : int or list of int, optional
        Priority Code Point (IEEE 802.1p traffic class). Values 0-7.

    src_ipv4 : :class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` or :class:`IPv4Address` or \
    list of (:class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` | :class:`IPv4Address`), optional
        Source IPv4 address(es) to filter by.

    dst_ipv4 : :class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` or :class:`IPv4Address` or \
    list of (:class:`~flync.core.datatypes.ipaddress.IPv4AddressEntry` | :class:`IPv4Address`), optional
        Destination IPv4 address(es) to filter by.

    src_ipv6 : :class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry` or :class:`IPv6Address` or \
    list of (:class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry` | :class:`IPv6Address`), optional
        Source IPv6 address(es) to filter by.

    dst_ipv6 : :class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry` or :class:`IPv6Address` or \
    list of (:class:`~flync.core.datatypes.ipaddress.IPv6AddressEntry` | :class:`IPv6Address`), optional
        Destination IPv6 address(es) to filter by.

    protocol : Literal["tcp", "udp"], optional
        Transport protocol to filter by.

    src_port : int or :class:`~flync.core.datatypes.value_range.ValueRange` or \
    list of (int | :class:`~flync.core.datatypes.value_range.ValueRange`), optional
        Source transport layer port(s). Integers must be > 0.

    dst_port : int or :class:`~flync.core.datatypes.value_range.ValueRange` or \
    list of (int | :class:`~flync.core.datatypes.value_range.ValueRange`), optional
        Destination transport layer port(s). Integers must be > 0.
    """

    ethertype: Optional[Annotated[Ethertype | List[Ethertype], PlainSerializer(serialize_ethertype), BeforeValidator(validate_ethertype_input)]] = (
        Field(default=None)
    )
    src_mac: Optional[str | MACAddressEntry | List[str | MACAddressEntry]] = Field(default=None)
    dst_mac: Optional[str | MACAddressEntry | List[str | MACAddressEntry]] = Field(default=None)
    vlan_tagged: Optional[bool] = Field(default=None)
    vlanid: Optional[int | ValueRange | List[int | ValueRange]] = Field(default=None)
    pcp: Optional[int | List[int]] = Field(default=None)
    src_ipv4: Optional[IPv4AddressEntry | IPv4Address | List[IPv4AddressEntry | IPv4Address]] = Field(default=None)
    dst_ipv4: Optional[IPv4AddressEntry | IPv4Address | List[IPv4AddressEntry | IPv4Address]] = Field(default=None)
    src_ipv6: Optional[IPv6AddressEntry | IPv6Address | List[IPv6AddressEntry | IPv6Address]] = Field(default=None)
    dst_ipv6: Optional[IPv6AddressEntry | IPv6Address | List[IPv6AddressEntry | IPv6Address]] = Field(default=None)
    protocol: Optional[Literal["tcp"] | Literal["udp"]] = Field(default=None)
    src_port: Optional[int | ValueRange | List[int | ValueRange]] = Field(default=None)
    dst_port: Optional[int | ValueRange | List[int | ValueRange]] = Field(default=None)

    @staticmethod
    def vlan_validator(value):
        """Validate one VLAN ID via :func:`validate_vlan_id`."""
        validate_vlan_id(value)

    @staticmethod
    def pcp_validator(value):
        if value < 0 or value > 7:
            raise err_minor(
                "pcp value must be greater than or equal to 0 and less than or equal to 7", category=Category.VALUE_RANGE, error_number="150"
            )

    @field_validator("vlanid", mode="after")
    @classmethod
    def validate_vlanids(cls, value):
        if isinstance(value, int):
            cls.vlan_validator(value)
        elif isinstance(value, ValueRange):
            cls.vlan_validator(value.from_value)
            cls.vlan_validator(value.to_value)
        elif isinstance(value, list):
            for v in value:
                if isinstance(v, int):
                    cls.vlan_validator(v)
                if isinstance(v, ValueRange):
                    cls.vlan_validator(v.from_value)
                    cls.vlan_validator(v.to_value)

        return value

    @field_validator("pcp", mode="after")
    @classmethod
    def validate_pcps(cls, value):
        if isinstance(value, int):
            cls.pcp_validator(value)
        if isinstance(value, list):
            for v in value:
                cls.pcp_validator(v)
        return value

    @field_validator("src_mac", "dst_mac", mode="after")
    @classmethod
    def validate_port_mac(cls, value):
        """MAC addresses must be valid."""
        if isinstance(value, List):
            for element in value:
                value = MacAddress.validate_mac_address(element.encode())
        else:
            value = MacAddress.validate_mac_address(value.encode())
        return value

    @field_validator("src_port", "dst_port", mode="after")
    @classmethod
    def validate_port_assignment(cls, value):
        """UDP / TCP Ports must be greater than 0."""
        msg = "Protocol port must be greater than 0."
        if isinstance(value, ValueRange):
            if value.from_value <= 0 or value.to_value <= 0:
                raise err_minor(msg, category=Category.VALUE_RANGE, error_number="151")
        if isinstance(value, int) and value <= 0:
            raise err_minor(msg, category=Category.VALUE_RANGE, error_number="152")
        return value

    @field_serializer("src_ipv4", "dst_ipv4", "src_ipv6", "dst_ipv6")
    def serialize_ip_address(self, value):
        serialized = value
        if isinstance(value, list):
            serialized = [self.serialize_ip_address(v) for v in value]

        if isinstance(value, IPv4AddressEntry) or isinstance(value, IPv6AddressEntry):
            serialized = value.model_dump()

        if isinstance(value, IPv4Address) or isinstance(value, IPv6Address):
            serialized = str(value).upper()

        return serialized

    @field_serializer("vlanid", "src_port", "dst_port")
    def serialize_value_range(self, value):
        if isinstance(value, list):
            return [self.serialize_value_range(v) for v in value]

        if isinstance(value, ValueRange):
            return value.model_dump()

        return value


class Stream(FLYNCBaseModel):
    """
    Represents an IEEE 802.1Qci stream with optional traffic policing,
    and filtering.

    Parameters
    ----------
    name : str
        Unique name of the stream.

    stream_identification : list of :class:`FrameFilter`, optional
        List of filters used to identify stream traffic (defaults to ``[]``).

    drop_at_ingress : bool, optional
        Whether to drop traffic at ingress. Default is False.

    max_sdu_size : int, optional
        Maximum size of the Service Data Unit in bytes.
        Default is 1522 bytes.
        Must be greater or equal to 0.

    policer : :class:`SingleRateTwoColorMarker`, \
    :class:`SingleRateThreeColorMarker`, \
    or :class:`DoubleRateThreeColorMarker`, optional
        Optional traffic policer configuration for this stream.
        Determines how traffic is metered or marked.
        The correct subclass is selected based on the `type` discriminator.

    ipv : int, optional
        Internal Priority Value (0-7) assigned to the stream, if used.

    ats : :class:`ATSInstance`, optional
        Optional Asynchronous Traffic Shaping configuration for ingress streams.
    """

    name: str = Field()
    stream_identification: List[FrameFilter] = Field([])
    drop_at_ingress: Optional[bool] = Field(default=False)
    max_sdu_size: Optional[int] = Field(default=1522, ge=0)
    policer: Optional[SingleRateTwoColorMarker | SingleRateThreeColorMarker | DoubleRateThreeColorMarker] = Field(default=None, discriminator="type")
    ipv: Optional[int] = Field(default=None, ge=0, le=7)
    ats: Optional[ATSInstance] = Field(default=None)


class TrafficClass(FLYNCBaseModel):
    """
    Defines a traffic class for prioritizing and shaping traffic on device egress queues.

    Parameters
    ----------
    name : str
        Name or description of the traffic class.

    priority : int
        Traffic Class priority value.
        Valid range: 0-7.

    frame_priority_values : list of int, optional
        Mapped priority values from Ethernet frames (PCP values).
        Valid range for each entry: 0-7.
        Default is an empty list.

    internal_priority_values : list of int, optional
        Mapped internal switch priority values (IPV).
        Valid range for each entry: 0-7.
        Default is an empty list.

    selection_mechanisms : :class:`CBSShaper` or :class:`ATSShaper`, optional
        Optional shaping mechanism applied to the traffic class.
        Can be a CBS (Credit-Based Shaper) or ATS (Asynchronous Traffic Shaper) instance.
        The correct subclass is selected using the `type` discriminator.
    """

    name: str = Field()
    priority: int = Field(..., ge=0, le=7)
    frame_priority_values: Annotated[
        Optional[List[int]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    internal_priority_values: Annotated[
        Optional[List[int]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    selection_mechanisms: Optional[CBSShaper | ATSShaper] = Field(default=None, discriminator="type")

    @field_validator("frame_priority_values", "internal_priority_values", mode="after")
    @classmethod
    def validate_priority_values(cls, v):
        """Priority values in list must be in range 0 - 7."""
        if not v:
            return []

        invalid = [num for num in v if not (0 <= num <= 7)]
        if invalid:
            raise err_minor(f"Priority value out of range [0..7]: {invalid}", category=Category.VALUE_RANGE, error_number="153")
        return v

    @model_validator(mode="after")
    def validate_at_least_one_prio_list(self) -> Self:
        """
        At least one of frame_priority_values \
            or internal_priority_values must be set."""

        if not self.frame_priority_values and not self.internal_priority_values:
            raise err_minor(
                "At least one of frame_priority_values or internal_priority_values must be provided and non-empty",
                category=Category.REQUIRED,
                error_number="154",
            )
        return self


class HTBFilter(FrameFilter):
    """
    Defines a filter for HTB (Hierarchical Token Bucket) child
    classes.

    Parameters
    ----------
    filter_priority : int
        Priority of the filter.
    """

    filter_priority: int = Field()


class ChildClass(FLYNCBaseModel):
    """
    Defines a child class for HTB (Hierarchical Token Bucket).

    Parameters
    ----------
    classid : int
        Unique identifier for the child class.

    rate : int
        Minimum guaranteed bandwidth for this child class in Mbps.

    ceil : int
        Maximum bandwidth this class can consume if leftover capacity is available (in Mbps).

    priority : int
        Priority of the child class.

    filter : list of :class:`HTBFilter`, optional
        List of filters applied to identify traffic belonging to this class.

    child_classes : list of :class:`ChildClass`, optional
        Nested child classes under this HTB class.
    """

    classid: int = Field()
    rate: int = Field()
    ceil: int = Field()
    priority: int = Field()
    filter: Annotated[
        Optional[List[HTBFilter]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])
    child_classes: Annotated[
        Optional[List["ChildClass"]],
        BeforeValidator(none_to_empty_list),
    ] = Field(default=[])


class HTBInstance(FLYNCBaseModel):
    """
    Defines an HTB (Hierarchical Token Bucket) instance for traffic shaping and class-based bandwidth management.

    Parameters
    ----------
    root_id : str
        Identifier for the root of the HTB hierarchy.
        Must follow the format `"number:"`.

    default_class : int, optional
        Default class ID to which traffic is assigned if it does not match any child class.

    child_classes : list of :class:`ChildClass`
        List of child classes under the HTB root, defining traffic
        priorities, guaranteed rates, ceilings, and filters.
    """

    root_id: str = Field(..., pattern=r"^\d+:$")
    default_class: Optional[int] = Field(default=None)
    child_classes: list[ChildClass] = Field()

    @model_validator(mode="after")
    def validate_htb_config(self):
        """
        Validate the HTB (Hierarchical Token Bucket) configuration attached to the model instance.

        Parameters
        ----------
        self : :class:`HTBInstance` The model instance being validated.

        Returns
        -------
        self
            The same model instance, returned to satisfy the Pydantic ``model_validator`` contract after successful validation.

        Raises
        ------
        err_minor
            If any of the validation rules fail (missing default class, duplicate class IDs, rate/ceil inconsistencies, etc.).
        """

        # Default class should exist if specified and must be a leaf class.

        if self.default_class is not None:
            default = self.default_class
            exists = False
            for child in self.child_classes:
                if self.check_default(child, default):
                    exists = True
                    break
            if not exists:
                raise err_minor(
                    f"Validation Error in HTB Config. Removing config from the interface. Default class {default} should exist in the HTB config.",
                    category=Category.REFERENCE,
                    error_number="155",
                )

        # Prio of child classes are unique
        self.check_all_class_priority_unique(self.child_classes, [])

        # Ceil must be greater than rate for every child class.

        for child in self.child_classes:
            self.check_ceil_greater_than_rate(child)

        # Each classid must be unique
        self.check_all_classes_unique(self.child_classes, [])

        # Check that sum of child's rate should be less than parent's rate

        self.check_rate_consistency(self.child_classes)

        # Check if child's ceil is lower than parent's ceil

        self.check_ceil_consistency(self.child_classes)
        return self

    def check_default(self, child_class, default):
        """
        Verify that the ``default`` class identifier refers to a leaf class within the HTB hierarchy.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance being validated.
        child_class : :class:`HTBClass`
            A class (or subtree) from the HTB configuration to be inspected.
        default : int
            The class identifier that should correspond to a leaf class.

        Returns
        -------
        bool
            ``True`` if the ``default`` identifier is found and is a leaf class;``False`` otherwise.

        Raises
        ------
        err_minor
            If the ``default`` identifier is found but the corresponding class has child classes (i.e., it is not a leaf).
        """

        if child_class.classid == default:
            if child_class.child_classes:
                raise err_minor(
                    f"Validation Error in HTB Config. Removing config from the interface. Default class {default} "
                    "must be a leaf class in HTB config.",
                    category=Category.STRUCTURAL,
                    error_number="156",
                )
            return True
        exists = False
        if child_class.child_classes:

            for child in child_class.child_classes:
                if self.check_default(child, default):
                    exists = True
        return exists

    def check_ceil_greater_than_rate(self, child):
        """
        Ensure that the ``ceil`` value of an HTB class is not smaller than its ``rate`` value, and apply the same check recursively
        to any nested child classes.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance that owns the HTB configuration.
        child : :class:`HTBClass`
            The HTB class (or subtree) whose ``ceil`` and ``rate`` values are being validated.

        Returns
        -------
        None
            The function completes silently when validation passes.

        Raises
        ------
        err_minor
            If ``child.ceil`` is less than ``child.rate``.
            The error message also identifies the offending class via its ``classid``.
        """

        if child.ceil < child.rate:
            raise err_minor(
                "Validation Error in HTB Config. Removing config from the interface. Incompatible HTB config. "
                f"Ceil cannot be less than  rate. Class {child.classid}.",
                category=Category.CONSISTENCY,
                error_number="157",
            )
        else:
            if child.child_classes:
                for child_class in child.child_classes:
                    self.check_ceil_greater_than_rate(child_class)

    def check_all_class_priority_unique(self, child_classes, class_priority):
        """
        Walk the HTB tree and verify that every ``prio``
        appears only once.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance being validated.
        child_classes : list[:class:`HTBClass`]
            The current collection of HTB classes to inspect.
        class_priority : list[int]
            Accumulator of class priorities that have already been seen during
            the walk.

        Returns
        -------
        None
            The function returns silently when all class priorities are unique.

        Raises
        ------
        err_minor
            If a duplicate ``prio`` is encountered.
            The error message identifies the offending prio.
        """

        for child in child_classes:
            if child.priority in class_priority:
                raise err_minor(
                    f"Validation Error in HTB Config. Removing config from the interface. All priorities must be unique, prio {child.priority}.",
                    category=Category.UNIQUENESS,
                    error_number="158",
                )
            else:
                class_priority.append(child.priority)
                if child.child_classes:
                    self.check_all_class_priority_unique(child.child_classes, class_priority)

    def check_all_classes_unique(self, child_classes, names):
        """
        Walk the HTB tree and verify that every ``classid`` appears only once.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance being validated.
        child_classes : list[:class:`HTBClass`]
            The current collection of HTB classes to inspect.
        names : list[int]
            Accumulator of class IDs that have already been seen during the walk.

        Returns
        -------
        None
            The function returns silently when all IDs are unique.

        Raises
        ------
        err_minor
            If a duplicate ``classid`` is encountered.
            The error message identifies the offending ID.
        """

        for child in child_classes:
            if child.classid in names:
                raise err_minor(
                    f"Validation Error in HTB Config. Removing config from the interface. All classids must be unique, classid {child.classid}.",
                    category=Category.UNIQUENESS,
                    error_number="159",
                )
            else:
                names.append(child.classid)
                if child.child_classes:
                    self.check_all_classes_unique(child.child_classes, names)

    def check_rate_consistency(self, child_classes):
        """
        Verify that, for every parent class, the sum of the ``rate`` values of its direct child classes does not exceed the parent’s own ````.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance being validated.
        child_classes : list[:class:`HTBClass`]
            List of HTB classes (or a subtree) to be inspected.

        Returns
        -------
        int
            The total ``rate`` of the supplied ``child_classes``.
            This value ispropagated upward so that parent levels can compare their own ``rate`` against the cumulative child rate.

        Raises
        ------
        err_minor
            If the summed ``rate`` of a parent’s children is greater than the parent’s ``rate``.
            The error message includes the offending ``classid``.
        """

        rate = 0
        for child in child_classes:
            if child.child_classes:
                rate_sum_child = self.check_rate_consistency(child.child_classes)
                if rate_sum_child > child.rate:
                    raise err_minor(
                        f"Validation Error in HTB Config. Removing config from the interface. Incompatible HTB config. "
                        f"Sum of rate of child classes is greater than the rate of parent class. Class {child.classid}.",
                        category=Category.CONSISTENCY,
                        error_number="160",
                    )
            rate = rate + child.rate

        return rate

    def check_ceil_consistency(self, child_classes):
        """
        Ensure that each child class’s ``ceil`` does not exceed its parent’s ``ceil`` value.

        Parameters
        ----------
        self : :class:`HTBInstance`
            The model instance being validated.
        child_classes : list[:class:`HTBClass`]
            List of HTB classes (or a subtree) to be inspected.

        Returns
        -------
        int
            The maximum ``ceil`` value found among the supplied ``child_classes``.
            This value is returned so that higher‑level parents can compare their ``ceil`` against it.

        Raises
        ------
        err_minor
            If any child’s ``ceil`` is larger than the parent’s ``ceil``.
            The message identifies the offending ``classid``.
        """

        ceil = 0
        for child in child_classes:
            if child.child_classes:
                ceil_child = self.check_ceil_consistency(child.child_classes)
                if ceil_child > child.ceil:
                    raise err_minor(
                        f"Validation Error in HTB Config. Removing config from the interface."
                        f"Incompatible HTB config. Ceil of child class should be less than parent's class. Class {child.classid}.",
                        category=Category.CONSISTENCY,
                        error_number="161",
                    )
            ceil = max(ceil, child.ceil)
        return ceil
