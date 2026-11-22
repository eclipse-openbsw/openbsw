"""
Defines a UDS server: the access profiles (session + security-level bundles), the supported
UDS services, and the DIDs/DTCs it offers.

A UDS server is the ISO 14229 server side of a diagnostic connection - one ECU, or one
diagnostic entity inside an ECU. The DoIP transport that carries it (logical address, DoIP
timers) is modelled separately by
:class:`~flync.model.flync_4_diagnostics.doip.deployment.DoIPServerDeployment`.

The DID and DTC catalogs are system-wide while the services are per server, so the two are
tied together here rather than on the service models: a server that offers a readable DID
must offer ReadDataByIdentifier (0x22), one that offers a writable DID must offer
WriteDataByIdentifier (0x2E), and so on. The checks that only need the service ids run as
ordinary model validators; the ones that need a resolved
:class:`~flync.model.flync_4_diagnostics.uds.data_identifier.DataIdentifier` or
:class:`~flync.model.flync_4_diagnostics.uds.dtc.DiagnosticTroubleCode` run at the end of
:meth:`UDSServer.bind`.
"""

from typing import List, Literal, Optional

from pydantic import Field, model_validator
from typing_extensions import Annotated

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.annotations.reference import Reference
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major, warn
from flync.core.validators.generic import validate_list_items_unique

from .data_identifier import DataIdentifier
from .dtc import DiagnosticTroubleCode
from .routine import Routine
from .services import (
    DiagnosticSessionControlService,
    ReadDTCInformationService,
    RoutineControlService,
    SecurityAccessService,
    UDSServiceEntry,
)
from .subfunctions import EXT_DATA_DTC_REPORT_TYPES, SEVERITY_DTC_REPORT_TYPES, SNAPSHOT_DTC_REPORT_TYPES
from .timings import UDSTimingProfile

#: Service ids referenced by the capability cross-checks below.
SID_CLEAR_DIAGNOSTIC_INFORMATION = 0x14
SID_READ_DTC_INFORMATION = 0x19
SID_READ_DATA_BY_IDENTIFIER = 0x22
SID_WRITE_DATA_BY_IDENTIFIER = 0x2E
SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER = 0x2F
SID_REQUEST_DOWNLOAD = 0x34
SID_REQUEST_UPLOAD = 0x35
SID_TRANSFER_DATA = 0x36
SID_REQUEST_TRANSFER_EXIT = 0x37
SID_TESTER_PRESENT = 0x3E
SID_CONTROL_DTC_SETTING = 0x85


class AccessProfile(FLYNCBaseModel):
    """
    A named, reusable bundle of the sessions (and optional security level) required to
    access a service, DID, or routine.

    Parameters
    ----------
    name : str
        Name of the access profile, unique within its UDS server.

    default : bool, optional
        Whether this is the server's default access profile. Exactly one access profile of
        a UDS server must set this. Defaults to ``False``.

    sessions : list of str
        Names of the sessions (declared in the ``diagnostic_session_control`` service) in
        which access is granted.

    security_level : int or Literal["Locked"], optional
        Security level (declared in the ``security_access`` service) that must be unlocked.
        ``None`` means no security access is required.
    """

    name: str = Field()
    default: bool = Field(default=False)
    sessions: List[str] = Field(default_factory=list)
    security_level: Optional[int | Literal["Locked"]] = Field(default=None)


class UDSServer(FLYNCBaseModel):
    """
    A named UDS server (ISO 14229 server side, typically one ECU).

    Stored one per file under ``communication/diagnostics/uds/servers/``; the file name
    provides :attr:`name`. Referenced by name from a ``doip_server`` socket deployment,
    which adds the DoIP logical address and DoIP timings of the transport.

    Parameters
    ----------
    name : str
        Name of the UDS server, implied from the file name on disk.

    uds_timings_profile : str
        Name of the :class:`~flync.model.flync_4_diagnostics.uds.timings.UDSTimingProfile`
        used by this server.

    access_profiles : list of :class:`AccessProfile`, optional
        Named session/security-level bundles referenced by services, DIDs, and routines.

    services : list of :class:`~flync.model.flync_4_diagnostics.uds.services.DiagnosticSessionControlService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.EcuResetService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.ClearDiagnosticInformationService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.ReadDTCInformationService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.ReadDataByIdentifierService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.SecurityAccessService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.RoutineControlService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.RequestDownloadService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.RequestUploadService` or \
    :class:`~flync.model.flync_4_diagnostics.uds.services.GenericUDSService`, optional
        The UDS services supported by this server, selected by their ``sid``.

    dids : list of str, optional
        Names of the :class:`~flync.model.flync_4_diagnostics.uds.data_identifier.DataIdentifier`
        entries (from ``communication/diagnostics/uds/dids/``) offered by this server.

    dtcs : list of str, optional
        Names of the :class:`~flync.model.flync_4_diagnostics.uds.dtc.DiagnosticTroubleCode`
        entries (from ``communication/diagnostics/uds/dtcs/``) reported by this server.

    description : str, optional
        Human-readable description of the UDS server.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    uds_timings_profile: Annotated[str, Reference(source="_timings_ref")] = Field()
    access_profiles: List[AccessProfile] = Field(default_factory=list)
    services: List[UDSServiceEntry] = Field(default_factory=list)
    dids: List[str] = Field(default_factory=list)
    dtcs: List[str] = Field(default_factory=list)
    description: Optional[str] = Field(default=None)

    _timings_ref: Optional[UDSTimingProfile] = None
    _did_refs: List[DataIdentifier] = []
    _dtc_refs: List[DiagnosticTroubleCode] = []

    def _session_control(self) -> Optional[DiagnosticSessionControlService]:
        return next((s for s in self.services if isinstance(s, DiagnosticSessionControlService)), None)

    def _security_access(self) -> Optional[SecurityAccessService]:
        return next((s for s in self.services if isinstance(s, SecurityAccessService)), None)

    @model_validator(mode="after")
    def validate_access_profile_names_unique_and_defaulted(self) -> "UDSServer":
        validate_list_items_unique([profile.name for profile in self.access_profiles], f"access profile names of '{self.name}'")
        defaults = [profile for profile in self.access_profiles if profile.default]
        if self.access_profiles and len(defaults) != 1:
            raise err_major(
                "UDS server '{server_name}' must have exactly one default access profile, found {count}",
                category=Category.CONSISTENCY,
                error_number="267",
                server_name=self.name,
                count=len(defaults),
            )
        return self

    @model_validator(mode="after")
    def validate_access_profiles_reference_known_sessions_and_levels(self) -> "UDSServer":
        session_control = self._session_control()
        known_sessions = {session.name for session in session_control.sessions} if session_control else set()
        security_access = self._security_access()
        known_levels = {level.security_level for level in security_access.security_levels} if security_access else set()

        for profile in self.access_profiles:
            unknown_sessions = sorted(set(profile.sessions) - known_sessions, key=str)
            if unknown_sessions:
                raise err_major(
                    "Access profile '{profile_name}' of UDS server '{server_name}' references unknown session(s) {unknown}",
                    category=Category.REFERENCE,
                    error_number="268",
                    profile_name=profile.name,
                    server_name=self.name,
                    unknown=unknown_sessions,
                )
            if profile.security_level is not None and profile.security_level not in known_levels:
                raise err_major(
                    "Access profile '{profile_name}' of UDS server '{server_name}' references unknown security level '{level}'",
                    category=Category.REFERENCE,
                    error_number="269",
                    profile_name=profile.name,
                    server_name=self.name,
                    level=profile.security_level,
                )
        return self

    @model_validator(mode="after")
    def validate_service_access_profiles_known(self) -> "UDSServer":
        for service in self.services:
            if service.access_profile is not None and service.access_profile not in self._known_access_profiles():
                raise err_major(
                    "Service '{service_name}' of UDS server '{server_name}' references unknown access profile '{profile}'",
                    category=Category.REFERENCE,
                    error_number="270",
                    service_name=service.service,
                    server_name=self.name,
                    profile=service.access_profile,
                )
        return self

    @model_validator(mode="after")
    def validate_service_names_and_sids_unique(self) -> "UDSServer":
        validate_list_items_unique([service.service for service in self.services], f"service names of '{self.name}'")
        validate_list_items_unique([service.sid for service in self.services], f"service ids of '{self.name}'")
        return self

    def _offered_sids(self) -> set[int]:
        return {service.sid for service in self.services}

    def _read_dtc_information(self) -> Optional[ReadDTCInformationService]:
        return next((s for s in self.services if isinstance(s, ReadDTCInformationService)), None)

    @model_validator(mode="after")
    def validate_dtc_services_match_declared_dtcs(self) -> "UDSServer":
        """
        Raise when DTCs are declared with no service able to report or clear them, and warn
        on the reverse - a DTC service on a server that declares no DTC.
        """

        sids = self._offered_sids()
        reports_dtcs = SID_READ_DTC_INFORMATION in sids
        clears_dtcs = SID_CLEAR_DIAGNOSTIC_INFORMATION in sids

        if self.dtcs and not (reports_dtcs or clears_dtcs):
            raise err_major(
                "UDS server '{server_name}' declares DTCs but offers neither ReadDTCInformation (0x19) nor ClearDiagnosticInformation (0x14)",
                category=Category.CONSISTENCY,
                error_number="297",
                server_name=self.name,
            )
        if not self.dtcs:
            if reports_dtcs:
                warn(
                    "UDS server '{server_name}' offers ReadDTCInformation (0x19) but declares no DTCs",
                    category=Category.CONSISTENCY,
                    error_number="300",
                    server_name=self.name,
                )
            if clears_dtcs:
                warn(
                    "UDS server '{server_name}' offers ClearDiagnosticInformation (0x14) but declares no DTCs",
                    category=Category.CONSISTENCY,
                    error_number="301",
                    server_name=self.name,
                )
        if SID_CONTROL_DTC_SETTING in sids and not (reports_dtcs or clears_dtcs):
            warn(
                "UDS server '{server_name}' offers ControlDTCSetting (0x85) but no DTC service (0x19/0x14) it applies to",
                category=Category.CONSISTENCY,
                error_number="302",
                server_name=self.name,
            )
        return self

    @model_validator(mode="after")
    def validate_session_keepalive_and_block_transfer(self) -> "UDSServer":
        """
        Raise on a block transfer set that cannot work, and warn when a non-default session
        has no TesterPresent to keep ``S3_server`` from expiring.
        """

        sids = self._offered_sids()
        session_control = self._session_control()
        non_default_sessions = [s for s in session_control.sessions if s.name != "default"] if session_control else []
        if non_default_sessions and SID_TESTER_PRESENT not in sids:
            warn(
                "UDS server '{server_name}' declares non-default session(s) {sessions} but does not offer "
                "TesterPresent (0x3E); S3_server cannot be kept alive",
                category=Category.CONSISTENCY,
                error_number="303",
                server_name=self.name,
                sessions=[session.name for session in non_default_sessions],
            )

        sets_up_transfer = bool(sids & {SID_REQUEST_DOWNLOAD, SID_REQUEST_UPLOAD})
        if SID_TRANSFER_DATA in sids and not sets_up_transfer:
            raise err_major(
                "UDS server '{server_name}' offers TransferData (0x36) but neither RequestDownload (0x34) "
                "nor RequestUpload (0x35) to set a transfer up",
                category=Category.CONSISTENCY,
                error_number="298",
                server_name=self.name,
            )
        if sets_up_transfer and SID_TRANSFER_DATA not in sids:
            raise err_major(
                "UDS server '{server_name}' sets up block transfers but does not offer TransferData (0x36)",
                category=Category.CONSISTENCY,
                error_number="299",
                server_name=self.name,
            )
        if sets_up_transfer and SID_REQUEST_TRANSFER_EXIT not in sids:
            warn(
                "UDS server '{server_name}' offers block transfer but not RequestTransferExit (0x37)",
                category=Category.CONSISTENCY,
                error_number="304",
                server_name=self.name,
            )
        return self

    def _known_access_profiles(self) -> set[str]:
        return {profile.name for profile in self.access_profiles}

    def bind(self, timings_by_id: dict, dids_by_name: dict, dtcs_by_name: dict, routines_by_name: dict) -> None:
        """
        Resolve :attr:`uds_timings_profile`, :attr:`dids`, :attr:`dtcs`, and any
        ``routine_control`` service's routine names against the UDS catalogs, and check
        that every resolved DID/routine only requires an access profile this server declares.
        """

        timings = timings_by_id.get(self.uds_timings_profile)
        if timings is None:
            raise err_major(
                "UDS server '{server_name}' references unknown UDS timings profile '{profile_id}'",
                category=Category.REFERENCE,
                error_number="277",
                server_name=self.name,
                profile_id=self.uds_timings_profile,
            )
        self._timings_ref = timings

        resolved_dids = []
        for did_name in self.dids:
            did = dids_by_name.get(did_name)
            if did is None:
                raise err_major(
                    "UDS server '{server_name}' references unknown DID '{did_name}'",
                    category=Category.REFERENCE,
                    error_number="271",
                    server_name=self.name,
                    did_name=did_name,
                )
            resolved_dids.append(did)
        self._did_refs = resolved_dids

        resolved_dtcs = []
        for dtc_name in self.dtcs:
            dtc = dtcs_by_name.get(dtc_name)
            if dtc is None:
                raise err_major(
                    "UDS server '{server_name}' references unknown DTC '{dtc_name}'",
                    category=Category.REFERENCE,
                    error_number="272",
                    server_name=self.name,
                    dtc_name=dtc_name,
                )
            resolved_dtcs.append(dtc)
        self._dtc_refs = resolved_dtcs

        resolved_routines: List[Routine] = []
        for service in self.services:
            if not isinstance(service, RoutineControlService):
                continue
            service_routines = []
            for routine_name in service.routines:
                routine = routines_by_name.get(routine_name)
                if routine is None:
                    raise err_major(
                        "UDS server '{server_name}' references unknown routine '{routine_name}'",
                        category=Category.REFERENCE,
                        error_number="273",
                        server_name=self.name,
                        routine_name=routine_name,
                    )
                service_routines.append(routine)
            service._routine_refs = service_routines
            resolved_routines.extend(service_routines)

        self._validate_catalog_access_profiles(resolved_dids, resolved_routines)
        self._validate_service_capability_coverage(resolved_dids, resolved_dtcs)

    def _validate_catalog_access_profiles(self, dids: List[DataIdentifier], routines: List[Routine]) -> None:
        """
        Raise when a DID or routine offered by this server requires an access profile the
        server does not declare - the catalogs are system-wide, the profiles are per server.
        """

        known = self._known_access_profiles()
        for did in dids:
            if did.access_profile is not None and did.access_profile not in known:
                raise err_major(
                    "DID '{did_name}' offered by UDS server '{server_name}' requires unknown access profile '{profile}'",
                    category=Category.REFERENCE,
                    error_number="281",
                    did_name=did.name,
                    server_name=self.name,
                    profile=did.access_profile,
                )
        for routine in routines:
            if routine.access_profile is not None and routine.access_profile not in known:
                raise err_major(
                    "Routine '{routine_name}' offered by UDS server '{server_name}' requires unknown access profile '{profile}'",
                    category=Category.REFERENCE,
                    error_number="282",
                    routine_name=routine.name,
                    server_name=self.name,
                    profile=routine.access_profile,
                )

    def _validate_service_capability_coverage(self, dids: List[DataIdentifier], dtcs: List[DiagnosticTroubleCode]) -> None:
        """
        Raise when a DID this server offers has no service able to access it, and warn on the
        reverse - an access service with nothing to act on - and on DTC configuration that
        the declared ReadDTCInformation report types cannot deliver.

        Runs from :meth:`bind` because it needs the resolved catalog entries: ``access`` and
        ``io_control`` live on the DID, not on the server's name list.
        """

        self._validate_did_service_coverage(dids)
        self._validate_dtc_format_uniformity(dtcs)
        self._validate_dtc_report_type_coverage(dtcs)

    def _validate_did_service_coverage(self, dids: List[DataIdentifier]) -> None:
        """
        Raise when a DID this server offers has no service able to access it, and warn on the
        reverse - an access service with no DID it can act on.
        """

        sids = self._offered_sids()
        readable = [did for did in dids if did.access in ("read", "read_write")]
        writable = [did for did in dids if did.access in ("write", "read_write")]
        controllable = [did for did in dids if did.io_control is not None]

        self._validate_dids_have_access_service(sids, readable, writable, controllable)
        self._warn_access_services_without_dids(sids, readable, writable, controllable)

    def _validate_dids_have_access_service(
        self,
        sids: set[int],
        readable: List[DataIdentifier],
        writable: List[DataIdentifier],
        controllable: List[DataIdentifier],
    ) -> None:
        """Raise when a DID needs an access service this server does not offer."""

        error_msg = "UDS server '{server_name}' offers DID '{did_name}' but not {service_label} to access it"

        # Spelled out rather than looped: the error catalog is built by a static AST scan, so
        # every error_number has to be a literal at its call site.
        if readable and SID_READ_DATA_BY_IDENTIFIER not in sids:
            raise err_major(
                error_msg,
                category=Category.REQUIRED,
                error_number="305",
                server_name=self.name,
                did_name=readable[0].name,
                service_label="ReadDataByIdentifier (0x22)",
            )
        if writable and SID_WRITE_DATA_BY_IDENTIFIER not in sids:
            raise err_major(
                error_msg,
                category=Category.REQUIRED,
                error_number="306",
                server_name=self.name,
                did_name=writable[0].name,
                service_label="WriteDataByIdentifier (0x2E)",
            )
        if controllable and SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER not in sids:
            raise err_major(
                error_msg,
                category=Category.REQUIRED,
                error_number="307",
                server_name=self.name,
                did_name=controllable[0].name,
                service_label="InputOutputControlByIdentifier (0x2F)",
            )

    def _warn_access_services_without_dids(
        self,
        sids: set[int],
        readable: List[DataIdentifier],
        writable: List[DataIdentifier],
        controllable: List[DataIdentifier],
    ) -> None:
        """Warn when this server offers a DID access service but no DID it can act on."""

        warn_msg = "UDS server '{server_name}' offers {service_label} but no DID it offers can be accessed that way"

        if SID_READ_DATA_BY_IDENTIFIER in sids and not readable:
            warn(
                warn_msg,
                category=Category.CONSISTENCY,
                error_number="308",
                server_name=self.name,
                service_label="ReadDataByIdentifier (0x22)",
            )
        if SID_WRITE_DATA_BY_IDENTIFIER in sids and not writable:
            warn(
                warn_msg,
                category=Category.CONSISTENCY,
                error_number="309",
                server_name=self.name,
                service_label="WriteDataByIdentifier (0x2E)",
            )
        if SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER in sids and not controllable:
            warn(
                warn_msg,
                category=Category.CONSISTENCY,
                error_number="310",
                server_name=self.name,
                service_label="InputOutputControlByIdentifier (0x2F)",
            )

    def _validate_dtc_format_uniformity(self, dtcs: List[DiagnosticTroubleCode]) -> None:
        """Warn when the declared DTCs do not share the single DTCFormatIdentifier a response can carry."""

        formats = {dtc.format for dtc in dtcs}
        if len(formats) > 1:
            warn(
                "UDS server '{server_name}' reports DTCs in more than one format ({formats}); ReadDTCInformation "
                "reports a single DTCFormatIdentifier",
                category=Category.CONSISTENCY,
                error_number="311",
                server_name=self.name,
                formats=sorted(formats),
            )

    def _validate_dtc_report_type_coverage(self, dtcs: List[DiagnosticTroubleCode]) -> None:
        """Warn on ReadDTCInformation report types that none of the declared DTCs can supply data for."""

        read_dtc = self._read_dtc_information()
        if read_dtc is None or not dtcs:
            return
        report_types = read_dtc.named_report_types()
        if SNAPSHOT_DTC_REPORT_TYPES.intersection(report_types) and not any(dtc.snapshot_records for dtc in dtcs):
            warn(
                "UDS server '{server_name}' offers snapshot report type(s) but no DTC it declares has snapshot_records",
                category=Category.CONSISTENCY,
                error_number="312",
                server_name=self.name,
            )
        if EXT_DATA_DTC_REPORT_TYPES.intersection(report_types) and not any(dtc.extended_data_records for dtc in dtcs):
            warn(
                "UDS server '{server_name}' offers extended-data report type(s) but no DTC it declares has extended_data_records",
                category=Category.CONSISTENCY,
                error_number="313",
                server_name=self.name,
            )
        if SEVERITY_DTC_REPORT_TYPES.intersection(report_types) and all(dtc.severity == "no_severity" for dtc in dtcs):
            warn(
                "UDS server '{server_name}' offers severity-based report type(s) but every DTC it declares has severity 'no_severity'",
                category=Category.CONSISTENCY,
                error_number="314",
                server_name=self.name,
            )
