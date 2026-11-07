"""
Defines the UDS services a :class:`~flync.model.flync_4_diagnostics.uds.server.UDSServer` supports.

:class:`GenericUDSService` is the base of every service and is usable on its own for services
that need no dedicated structure. Services that do carry structure (sessions, security levels,
DTC groups, routines, reset types, DTC report types) inherit from it and narrow ``service``
and ``sid`` to a ``Literal``.

Selection uses a *callable* discriminator (:data:`UDSServiceEntry`) rather than the usual
``Field(discriminator=...)``: the set of UDS service ids is open-ended - OEM-specific and
not-yet-modelled services must still load - so no single ``Literal`` field can cover every
member. The callable maps a modelled ``sid`` to its tag and everything else to ``"generic"``.

Services that carry no configuration of their own - WriteDataByIdentifier (0x2E),
TransferData (0x36), RequestTransferExit (0x37), TesterPresent (0x3E) and the rest - stay
:class:`GenericUDSService` on purpose. Everything they would hold already lives elsewhere: a
DID's writability on :class:`~flync.model.flync_4_diagnostics.uds.data_identifier.DataIdentifier`,
the transfer block length on :class:`RequestDownloadService`, ``S3_server`` on the server's
timing profile. They are still spell-checked, because :data:`CANONICAL_SERVICE_NAMES` pins the
``service`` name of every service id ISO 14229-1 standardises.
"""

from typing import Annotated, Any, List, Literal, Optional, Union

from pydantic import BeforeValidator, Discriminator, Field, Tag, field_serializer, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes import DurationMs, serialize_duration_ms
from flync.core.utils.exceptions import Category, err_major, warn
from flync.core.validators.generic import validate_list_items_unique

from .routine import Routine
from .subfunctions import (
    ALL_DTC_GROUPS,
    DEPRECATED_DTC_REPORT_TYPES,
    USER_DEF_MEMORY_DTC_REPORT_TYPES,
    DTCReportType,
    ResetType,
    coerce_int,
)


def coerce_sid(value: Any) -> Any:
    """
    Normalise a service id written as a string (e.g. ``"0x10"``) to an ``int``.

    Anything that is not a parsable string is returned unchanged, so the field's own
    validation reports it.
    """

    return coerce_int(value)


#: A UDS service identifier, also accepting a hex or decimal string in YAML.
ServiceId = Annotated[int, BeforeValidator(coerce_sid), Field(ge=0x00, le=0xFF)]

#: A sub-function or record number, also accepting a hex or decimal string in YAML.
SubfunctionValue = Annotated[int, BeforeValidator(coerce_int), Field(ge=0x00, le=0xFF)]

#: The canonical ISO 14229-1 name of every standardised service id.
#:
#: A ``services`` entry whose ``sid`` appears here must use the matching ``service`` name, so
#: a typo is caught even for the many service ids that have no dedicated model. Service ids
#: absent from this table - OEM-specific ones - keep an unconstrained name.
CANONICAL_SERVICE_NAMES: dict[int, str] = {
    0x10: "diagnostic_session_control",
    0x11: "ecu_reset",
    0x14: "clear_diagnostic_information",
    0x19: "read_dtc_information",
    0x22: "read_data_by_identifier",
    0x23: "read_memory_by_address",
    0x24: "read_scaling_data_by_identifier",
    0x27: "security_access",
    0x28: "communication_control",
    0x29: "authentication",
    0x2A: "read_data_by_periodic_identifier",
    0x2C: "dynamically_define_data_identifier",
    0x2E: "write_data_by_identifier",
    0x2F: "input_output_control_by_identifier",
    0x31: "routine_control",
    0x34: "request_download",
    0x35: "request_upload",
    0x36: "transfer_data",
    0x37: "request_transfer_exit",
    0x38: "request_file_transfer",
    0x3D: "write_memory_by_address",
    0x3E: "tester_present",
    0x83: "access_timing_parameter",
    0x84: "secured_data_transmission",
    0x85: "control_dtc_setting",
    0x86: "response_on_event",
    0x87: "link_control",
}


class UDSSubfunction(FLYNCBaseModel):
    """
    A named sub-function value of a UDS service.

    Parameters
    ----------
    id : int
        The sub-function identifier.

    name : str
        Human-readable name of the sub-function.
    """

    id: Annotated[int, Field(ge=0x00, le=0xFF)] = Field()
    name: str = Field()


class GenericUDSService(FLYNCBaseModel):
    """
    Base class of every UDS service, and the service model used for any service without
    dedicated structure of its own (e.g. WriteDataByIdentifier, TesterPresent,
    ControlDTCSetting, TransferData).

    Parameters
    ----------
    service : str
        Name of the service, e.g. ``"tester_present"``. Pinned to the ISO 14229-1 name
        whenever :attr:`sid` is a standardised service id.

    sid : int
        The UDS service identifier, e.g. ``0x3E``.

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one (e.g. ControlDTCSetting).
    """

    service: str = Field()
    sid: ServiceId = Field()
    access_profile: Optional[str] = Field(default=None)
    subfunctions: Optional[List[UDSSubfunction]] = Field(default=None)
    option_record: Optional[dict] = Field(default=None)

    @model_validator(mode="after")
    def validate_service_name_is_canonical(self) -> "GenericUDSService":
        """
        Raise when a standardised service id carries a name other than its ISO 14229-1 one.
        """

        canonical = CANONICAL_SERVICE_NAMES.get(self.sid)
        if canonical is not None and self.service != canonical:
            raise err_major(
                "Service id {sid:#04x} is '{canonical}' in ISO 14229-1, but is named '{service_name}'",
                category=Category.CONSISTENCY,
                error_number="284",
                sid=self.sid,
                canonical=canonical,
                service_name=self.service,
            )
        return self


class DiagnosticSessionDefinition(FLYNCBaseModel):
    """
    A diagnostic session offered by the ``diagnostic_session_control`` service.

    The session named ``"default"`` is the session the ECU starts in after reset; exactly
    one session of a :class:`DiagnosticSessionControlService` must use that name.

    Parameters
    ----------
    name : str
        Name of the session, e.g. ``"extended"``.

    id : int
        The 8-bit session sub-function value, e.g. ``0x03`` for the extended session.

    p2 : int, optional
        Server response time in ms for this session (``P2_server``). ``None`` falls back to
        the owning server's UDS timing profile.

    p2_star : int, optional
        Enhanced server response time in ms for this session (``P2*_server``). ``None``
        falls back to the owning server's UDS timing profile.
    """

    name: str = Field()
    id: Annotated[int, Field(ge=0x00, le=0xFF)] = Field()
    p2: Optional[DurationMs] = Field(default=None)
    p2_star: Optional[DurationMs] = Field(default=None)

    @field_serializer("p2", "p2_star")
    def serialize_p2_timings(self, value: Optional[int]) -> Optional[str]:
        return serialize_duration_ms(value)


class DiagnosticSessionControlService(GenericUDSService):
    """
    DiagnosticSessionControl (0x10): declares the sessions offered by the UDS server.

    Parameters
    ----------
    service : Literal["diagnostic_session_control"]

    sid : Literal[0x10]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    sessions : list of :class:`DiagnosticSessionDefinition`, optional
        Diagnostic sessions offered by this service.
    """

    service: Literal["diagnostic_session_control"] = Field(default="diagnostic_session_control")
    sid: Annotated[Literal[0x10], BeforeValidator(coerce_sid)] = Field(default=0x10)
    sessions: List[DiagnosticSessionDefinition] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_exactly_one_default_session(self) -> "DiagnosticSessionControlService":
        defaults = [session for session in self.sessions if session.name == "default"]
        if len(defaults) != 1:
            raise err_major(
                "diagnostic_session_control must declare exactly one session named 'default', found {count}",
                category=Category.CONSISTENCY,
                error_number="266",
                count=len(defaults),
            )
        return self

    @model_validator(mode="after")
    def validate_session_names_and_ids_unique(self) -> "DiagnosticSessionControlService":
        validate_list_items_unique([session.name for session in self.sessions], "session names")
        validate_list_items_unique([session.id for session in self.sessions], "session ids")
        return self


class EcuResetService(GenericUDSService):
    """
    EcuReset (0x11): declares the reset types the UDS server accepts.

    Parameters
    ----------
    service : Literal["ecu_reset"]

    sid : Literal[0x11]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    reset_types : list of Literal["hard_reset", "key_off_on_reset", "soft_reset", \
    "enable_rapid_power_shutdown", "disable_rapid_power_shutdown"] or int, optional
        Reset types this service accepts. An ``int`` carries a system-supplier specific
        reset type (0x40-0x5F) that ISO 14229-1 does not name.

    power_down_time : int, optional
        Minimum standby time in **seconds** the server reports for
        ``enable_rapid_power_shutdown``. 0xFF is reserved by ISO 14229-1 for "failure or
        time not available", so the configurable maximum is 0xFE.
    """

    service: Literal["ecu_reset"] = Field(default="ecu_reset")
    sid: Annotated[Literal[0x11], BeforeValidator(coerce_sid)] = Field(default=0x11)
    reset_types: List[ResetType | SubfunctionValue] = Field(default_factory=list)
    power_down_time: Optional[Annotated[int, BeforeValidator(coerce_int), Field(ge=0x00, le=0xFE)]] = Field(default=None)

    @model_validator(mode="after")
    def validate_reset_types(self) -> "EcuResetService":
        """
        Raise on duplicate reset types, and on a ``power_down_time`` no reset type reports.
        """

        validate_list_items_unique(list(self.reset_types), "ecu_reset reset types")
        if self.power_down_time is not None and "enable_rapid_power_shutdown" not in self.reset_types:
            raise err_major(
                "ecu_reset declares a power_down_time but does not offer 'enable_rapid_power_shutdown'",
                category=Category.CONSISTENCY,
                error_number="285",
            )
        if not self.reset_types:
            warn(
                "ecu_reset declares no reset_types; no reset can be requested from this server",
                category=Category.REQUIRED,
                error_number="286",
            )
        return self


class DTCGroup(FLYNCBaseModel):
    """
    A named group (mask) of DTCs, used by ClearDiagnosticInformation.

    Parameters
    ----------
    name : str
        Name of the group, e.g. ``"powertrain"``.

    id : int
        The 3-byte DTC group mask, e.g. ``0xFFFFFF`` for "all groups".
    """

    name: str = Field()
    id: Annotated[int, Field(ge=0x000000, le=0xFFFFFF)] = Field()


class ClearDiagnosticInformationService(GenericUDSService):
    """
    ClearDiagnosticInformation (0x14): declares the clearable DTC groups.

    Parameters
    ----------
    service : Literal["clear_diagnostic_information"]

    sid : Literal[0x14]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    dtc_groups : list of :class:`DTCGroup`, optional
        DTC groups (masks) that ClearDiagnosticInformation may clear.

    memory_selections : list of int, optional
        User-defined memories this service can clear, as the optional ``MemorySelection``
        byte added by ISO 14229-1:2020. Empty means the primary memory only.
    """

    service: Literal["clear_diagnostic_information"] = Field(default="clear_diagnostic_information")
    sid: Annotated[Literal[0x14], BeforeValidator(coerce_sid)] = Field(default=0x14)
    dtc_groups: List[DTCGroup] = Field(default_factory=list)
    memory_selections: List[SubfunctionValue] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_dtc_groups(self) -> "ClearDiagnosticInformationService":
        """
        Raise on duplicate DTC groups, and warn when the declared groups look incomplete.
        """

        validate_list_items_unique([group.name for group in self.dtc_groups], "DTC group names")
        validate_list_items_unique([group.id for group in self.dtc_groups], "DTC group ids")
        validate_list_items_unique(list(self.memory_selections), "clear_diagnostic_information memory selections")
        if not self.dtc_groups:
            warn(
                "clear_diagnostic_information declares no dtc_groups; no DTC can be cleared",
                category=Category.REQUIRED,
                error_number="292",
            )
        elif not any(group.id == ALL_DTC_GROUPS for group in self.dtc_groups):
            warn(
                "clear_diagnostic_information declares no group with the 'all groups' mask {mask:#08x}",
                category=Category.CONSISTENCY,
                error_number="293",
                mask=ALL_DTC_GROUPS,
            )
        return self


class ReadDTCInformationService(GenericUDSService):
    """
    ReadDTCInformation (0x19): declares the DTC report types and the status bits the UDS
    server supports.

    Parameters
    ----------
    service : Literal["read_dtc_information"]

    sid : Literal[0x19]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    report_types : list of \
    :data:`~flync.model.flync_4_diagnostics.uds.subfunctions.DTCReportType` or int, optional
        Report types (sub-functions) this service supports. An ``int`` carries a report type
        ISO 14229-1 does not name.

    dtc_status_availability_mask : int, optional
        The ``DTCStatusAvailabilityMask`` the server reports: which DTC status bits it
        maintains, e.g. ``0x7F``. Use
        :func:`~flync.model.flync_4_diagnostics.uds.subfunctions.status_mask_from_bits` and
        :func:`~flync.model.flync_4_diagnostics.uds.subfunctions.status_mask_bits` to convert
        between the byte and the
        :data:`~flync.model.flync_4_diagnostics.uds.subfunctions.DTCStatusBit` names an OEM
        specification lists.

    memory_selections : list of int, optional
        User-defined memory numbers addressable by the ``report_user_def_memory_*`` report
        types. Empty means the primary memory only.

    snapshot_record_numbers : list of int, optional
        DTC snapshot (freeze frame) record numbers the server stores.

    ext_data_record_numbers : list of int, optional
        DTC extended data record numbers the server stores.
    """

    service: Literal["read_dtc_information"] = Field(default="read_dtc_information")
    sid: Annotated[Literal[0x19], BeforeValidator(coerce_sid)] = Field(default=0x19)
    report_types: List[DTCReportType | SubfunctionValue] = Field(default_factory=list)
    dtc_status_availability_mask: Optional[Annotated[int, BeforeValidator(coerce_int), Field(ge=0x00, le=0xFF)]] = Field(default=None)
    memory_selections: List[SubfunctionValue] = Field(default_factory=list)
    snapshot_record_numbers: List[SubfunctionValue] = Field(default_factory=list)
    ext_data_record_numbers: List[SubfunctionValue] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_report_types_and_mask(self) -> "ReadDTCInformationService":
        """
        Raise on duplicate report types and an all-zero availability mask; warn on a service
        that declares nothing, and on report types whose supporting configuration is absent.
        """

        validate_list_items_unique(list(self.report_types), "read_dtc_information report types")
        validate_list_items_unique(list(self.memory_selections), "read_dtc_information memory selections")
        validate_list_items_unique(list(self.snapshot_record_numbers), "read_dtc_information snapshot record numbers")
        validate_list_items_unique(list(self.ext_data_record_numbers), "read_dtc_information extended data record numbers")

        if self.dtc_status_availability_mask == 0x00:
            raise err_major(
                "read_dtc_information declares an all-zero dtc_status_availability_mask; no DTC status bit is supported",
                category=Category.VALUE_RANGE,
                error_number="287",
            )
        if not self.report_types:
            warn(
                "read_dtc_information declares no report_types; no DTC can be read from this server",
                category=Category.REQUIRED,
                error_number="288",
            )
        if self.dtc_status_availability_mask is None:
            warn(
                "read_dtc_information declares no dtc_status_availability_mask",
                category=Category.REQUIRED,
                error_number="289",
            )

        deprecated = sorted(DEPRECATED_DTC_REPORT_TYPES.intersection(self.named_report_types()))
        if deprecated:
            warn(
                "read_dtc_information uses report type(s) {deprecated} that ISO 14229-1:2020 deprecates",
                category=Category.LIFECYCLE,
                error_number="290",
                deprecated=deprecated,
            )
        if USER_DEF_MEMORY_DTC_REPORT_TYPES.intersection(self.named_report_types()) and not self.memory_selections:
            warn(
                "read_dtc_information offers user-defined-memory report types but declares no memory_selections",
                category=Category.CONSISTENCY,
                error_number="291",
            )
        return self

    def named_report_types(self) -> set[str]:
        """
        Return the ISO-named report types of this service, ignoring the numeric escapes.
        """

        return {report_type for report_type in self.report_types if isinstance(report_type, str)}


class ReadDataByIdentifierService(GenericUDSService):
    """
    ReadDataByIdentifier (0x22): reads the DIDs its UDS server offers.

    Which DIDs those are is not repeated here - it is the server's ``dids`` list, filtered
    by each :class:`~flync.model.flync_4_diagnostics.uds.data_identifier.DataIdentifier`'s
    ``access``. This service only adds what the server itself declares about the request.

    Parameters
    ----------
    service : Literal["read_data_by_identifier"]

    sid : Literal[0x22]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    max_dids_per_request : int, optional
        Largest number of data identifiers the server accepts in one request. ``None``
        means the server declares no limit.
    """

    service: Literal["read_data_by_identifier"] = Field(default="read_data_by_identifier")
    sid: Annotated[Literal[0x22], BeforeValidator(coerce_sid)] = Field(default=0x22)
    max_dids_per_request: Optional[Annotated[int, Field(ge=1)]] = Field(default=None)


class SecurityLevelDeclaration(FLYNCBaseModel):
    """
    Declares one security level made available by the ``security_access`` service.

    Parameters
    ----------
    security_level : int or Literal["Locked"]
        Identifier of the security level. ``"Locked"`` represents no security access granted.
    """

    security_level: int | Literal["Locked"] = Field()


class SecurityAccessService(GenericUDSService):
    """
    SecurityAccess (0x27): declares the security levels offered by the UDS server.

    Parameters
    ----------
    service : Literal["security_access"]

    sid : Literal[0x27]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    security_levels : list of :class:`SecurityLevelDeclaration`, optional
        Security levels offered by this service.
    """

    service: Literal["security_access"] = Field(default="security_access")
    sid: Annotated[Literal[0x27], BeforeValidator(coerce_sid)] = Field(default=0x27)
    security_levels: List[SecurityLevelDeclaration] = Field(default_factory=list)


class RoutineControlService(GenericUDSService):
    """
    RoutineControl (0x31): declares the routines offered by the UDS server.

    Parameters
    ----------
    service : Literal["routine_control"]

    sid : Literal[0x31]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    routines : list of str, optional
        Names of the :class:`~flync.model.flync_4_diagnostics.uds.routine.Routine` entries
        (from ``communication/diagnostics/uds/routines/``) offered by this service.
    """

    service: Literal["routine_control"] = Field(default="routine_control")
    sid: Annotated[Literal[0x31], BeforeValidator(coerce_sid)] = Field(default=0x31)
    routines: List[str] = Field(default_factory=list)

    _routine_refs: List[Routine] = []


class TransferMemoryRegion(FLYNCBaseModel):
    """
    A memory region a block transfer service may address.

    Parameters
    ----------
    name : str
        Name of the region, e.g. ``"application"``.

    address : int
        Start address of the region.

    size : int
        Size of the region in bytes.

    description : str, optional
        Human-readable description of the region.
    """

    name: str = Field()
    address: Annotated[int, BeforeValidator(coerce_int), Field(ge=0)] = Field()
    size: Annotated[int, BeforeValidator(coerce_int), Field(ge=1)] = Field()
    description: Optional[str] = Field(default=None)


class TransferSetupService(GenericUDSService):
    """
    Common base of the block transfer set-up services RequestDownload (0x34) and
    RequestUpload (0x35), which share their whole parameter set.

    This class is not selectable on its own - it carries no ``sid`` of its own and is not a
    member of :data:`UDSServiceEntry`.

    Parameters
    ----------
    service : str
        Name of the service. Narrowed by the concrete subclasses.

    sid : int
        The UDS service identifier. Narrowed by the concrete subclasses.

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    max_block_length : int
        The ``maxNumberOfBlockLength`` the server reports: the largest TransferData request
        it accepts, including the service id and block sequence counter.

    memory_address_length : int, optional
        Number of bytes of the ``memoryAddress`` parameter - the low nibble of the
        ``addressAndLengthFormatIdentifier``. Defaults to ``4``.

    memory_size_length : int, optional
        Number of bytes of the ``memorySize`` parameter - the high nibble of the
        ``addressAndLengthFormatIdentifier``. Defaults to ``4``.

    data_format_identifiers : list of int, optional
        The ``dataFormatIdentifier`` values the server accepts. Defaults to ``[0x00]``, i.e.
        neither compressed nor encrypted.

    memory_regions : list of :class:`TransferMemoryRegion`, optional
        Memory regions this service may address.
    """

    max_block_length: Annotated[int, BeforeValidator(coerce_int), Field(ge=2)] = Field()
    memory_address_length: Annotated[int, Field(ge=1, le=15)] = Field(default=4)
    memory_size_length: Annotated[int, Field(ge=1, le=15)] = Field(default=4)
    data_format_identifiers: List[SubfunctionValue] = Field(default_factory=lambda: [0x00])
    memory_regions: List[TransferMemoryRegion] = Field(default_factory=list)

    @model_validator(mode="after")
    def validate_memory_regions_fit_the_format_identifier(self) -> "TransferSetupService":
        """
        Raise when a declared region cannot be expressed with the declared address/size widths.
        """

        validate_list_items_unique([region.name for region in self.memory_regions], f"{self.service} memory region names")
        validate_list_items_unique(list(self.data_format_identifiers), f"{self.service} data format identifiers")

        max_address = (1 << (8 * self.memory_address_length)) - 1
        max_size = (1 << (8 * self.memory_size_length)) - 1
        for region in self.memory_regions:
            if region.address > max_address:
                raise err_major(
                    "{service_name} memory region '{region_name}' address {address:#x} does not fit in memory_address_length ({length} bytes)",
                    category=Category.CONSISTENCY,
                    error_number="294",
                    service_name=self.service,
                    region_name=region.name,
                    address=region.address,
                    length=self.memory_address_length,
                )
            if region.size > max_size:
                raise err_major(
                    "{service_name} memory region '{region_name}' size {size:#x} does not fit in memory_size_length ({length} bytes)",
                    category=Category.CONSISTENCY,
                    error_number="295",
                    service_name=self.service,
                    region_name=region.name,
                    size=region.size,
                    length=self.memory_size_length,
                )
        return self


class RequestDownloadService(TransferSetupService):
    """
    RequestDownload (0x34): sets up a transfer from the tester into the UDS server.

    Parameters
    ----------
    service : Literal["request_download"]

    sid : Literal[0x34]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    max_block_length : int
        The ``maxNumberOfBlockLength`` the server reports.

    memory_address_length : int, optional
        Number of bytes of the ``memoryAddress`` parameter. Defaults to ``4``.

    memory_size_length : int, optional
        Number of bytes of the ``memorySize`` parameter. Defaults to ``4``.

    data_format_identifiers : list of int, optional
        The ``dataFormatIdentifier`` values the server accepts. Defaults to ``[0x00]``.

    memory_regions : list of :class:`TransferMemoryRegion`, optional
        Memory regions this service may write to.
    """

    service: Literal["request_download"] = Field(default="request_download")
    sid: Annotated[Literal[0x34], BeforeValidator(coerce_sid)] = Field(default=0x34)


class RequestUploadService(TransferSetupService):
    """
    RequestUpload (0x35): sets up a transfer from the UDS server to the tester.

    Parameters
    ----------
    service : Literal["request_upload"]

    sid : Literal[0x35]

    access_profile : str, optional
        Name of the access profile required to use this service. ``None`` means the owning
        UDS server's default access profile applies.

    subfunctions : list of :class:`UDSSubfunction`, optional
        Sub-function values supported by this service.

    option_record : dict, optional
        Free-form option record for services that carry one.

    max_block_length : int
        The ``maxNumberOfBlockLength`` the server reports.

    memory_address_length : int, optional
        Number of bytes of the ``memoryAddress`` parameter. Defaults to ``4``.

    memory_size_length : int, optional
        Number of bytes of the ``memorySize`` parameter. Defaults to ``4``.

    data_format_identifiers : list of int, optional
        The ``dataFormatIdentifier`` values the server accepts. Defaults to ``[0x00]``.

    memory_regions : list of :class:`TransferMemoryRegion`, optional
        Memory regions this service may read from.
    """

    service: Literal["request_upload"] = Field(default="request_upload")
    sid: Annotated[Literal[0x35], BeforeValidator(coerce_sid)] = Field(default=0x35)


#: UDS service ids that have a dedicated model, mapped to their union tag.
SPECIALISED_SIDS: dict[int, str] = {
    0x10: "0x10",
    0x11: "0x11",
    0x14: "0x14",
    0x19: "0x19",
    0x22: "0x22",
    0x27: "0x27",
    0x31: "0x31",
    0x34: "0x34",
    0x35: "0x35",
}


def uds_service_tag(value: Any) -> str:
    """
    Return the union tag of a service entry: its ``sid`` when modelled, else ``"generic"``.

    Accepts both raw YAML mappings (during validation) and already-constructed models.
    """

    sid = coerce_sid(value.get("sid") if isinstance(value, dict) else getattr(value, "sid", None))
    if not isinstance(sid, int):
        return "generic"
    return SPECIALISED_SIDS.get(sid, "generic")


UDSServiceEntry = Annotated[
    Union[
        Annotated[DiagnosticSessionControlService, Tag("0x10")],
        Annotated[EcuResetService, Tag("0x11")],
        Annotated[ClearDiagnosticInformationService, Tag("0x14")],
        Annotated[ReadDTCInformationService, Tag("0x19")],
        Annotated[ReadDataByIdentifierService, Tag("0x22")],
        Annotated[SecurityAccessService, Tag("0x27")],
        Annotated[RoutineControlService, Tag("0x31")],
        Annotated[RequestDownloadService, Tag("0x34")],
        Annotated[RequestUploadService, Tag("0x35")],
        Annotated[GenericUDSService, Tag("generic")],
    ],
    Discriminator(uds_service_tag),
]
