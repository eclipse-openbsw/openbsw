"""
Defines UDS Diagnostic Trouble Codes (DTCs), reported via ReadDTCInformation (0x19).

A DTC carries the records ReadDTCInformation returns alongside it: the *snapshot* (freeze
frame) records captured when the fault was stored, and the *extended data* records the
server maintains for it. Both are keyed by their record number, which is what the tester
asks for, and both describe their payload with the same
:class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord` used for DID and
routine data.
"""

from typing import Annotated, List, Literal, Optional

from pydantic import BeforeValidator, Field, model_validator

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.validators.generic import validate_list_items_unique

from .datatypes import DiagDataRecord
from .subfunctions import coerce_int

DTCFormat = Literal["iso_14229_1", "saej1939_73", "iso_11992_4"]
DTCSeverity = Literal["no_severity", "maintenance_only", "check_at_next_halt", "check_immediately"]

#: A DTC snapshot or extended data record number, also accepting a hex string in YAML.
RecordNumber = Annotated[int, BeforeValidator(coerce_int), Field(ge=0x00, le=0xFF)]


class DTCRecord(FLYNCBaseModel):
    """
    A numbered data record ReadDTCInformation reports for a DTC.

    Snapshot (freeze frame) records and extended data records have the same shape, so both are
    described by this class; which kind a record is follows from the list it appears in -
    :attr:`DiagnosticTroubleCode.snapshot_records` or
    :attr:`DiagnosticTroubleCode.extended_data_records`.

    Parameters
    ----------
    record_number : int
        Number identifying this record, i.e. the ``DTCSnapshotRecordNumber`` for a snapshot
        record and the ``DTCExtDataRecordNumber`` for an extended data record.

    data : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`
        Layout of the data the record carries.

    description : str, optional
        Human-readable description of the record.
    """

    record_number: RecordNumber = Field()
    data: DiagDataRecord = Field()
    description: Optional[str] = Field(default=None)


class DiagnosticTroubleCode(FLYNCBaseModel):
    """
    A UDS Diagnostic Trouble Code (DTC), reported via ReadDTCInformation (0x19) and cleared
    via ClearDiagnosticInformation (0x14).

    Stored one per file under ``communication/diagnostics/uds/dtcs/``; the file name provides
    :attr:`name`.

    Parameters
    ----------
    name : str
        Name of the DTC, implied from the file name on disk.

    dtc : int
        The 3-byte DTC number, e.g. ``0x010203``.

    format : Literal["iso_14229_1", "saej1939_73", "iso_11992_4"], optional
        Format of :attr:`dtc` as reported by ReadDTCInformation. Defaults to ``"iso_14229_1"``.

    severity : Literal["no_severity", "maintenance_only", "check_at_next_halt", \
    "check_immediately"], optional
        UDS DTC severity mask. Defaults to ``"no_severity"``.

    functional_unit : int, optional
        The ``DTCFunctionalUnit`` reported together with :attr:`severity` by ReadDTCInformation
        sub-function 0x09. ``None`` means the server reports no functional unit for this DTC.

    snapshot_records : list of :class:`DTCRecord`, optional
        Snapshot (freeze frame) records the server stores for this DTC, reported by
        ReadDTCInformation sub-function 0x04.

    extended_data_records : list of :class:`DTCRecord`, optional
        Extended data records the server stores for this DTC, reported by ReadDTCInformation
        sub-functions 0x06 and 0x16.

    failure_type : str, optional
        Free-text classification of the underlying failure, e.g. ``"short_to_ground"``.

    description : str, optional
        Human-readable description of the DTC.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    dtc: Annotated[int, BeforeValidator(coerce_int), Field(ge=0x000000, le=0xFFFFFF)] = Field()
    format: DTCFormat = Field(default="iso_14229_1")
    severity: DTCSeverity = Field(default="no_severity")
    functional_unit: Optional[RecordNumber] = Field(default=None)
    snapshot_records: List[DTCRecord] = Field(default_factory=list)
    extended_data_records: List[DTCRecord] = Field(default_factory=list)
    failure_type: Optional[str] = Field(default=None)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_record_numbers_unique(self) -> "DiagnosticTroubleCode":
        """
        Raise when two snapshot or two extended data records of this DTC share a record number.
        """

        validate_list_items_unique(
            [record.record_number for record in self.snapshot_records],
            f"snapshot record numbers of DTC '{self.name}'",
        )
        validate_list_items_unique(
            [record.record_number for record in self.extended_data_records],
            f"extended data record numbers of DTC '{self.name}'",
        )
        return self
