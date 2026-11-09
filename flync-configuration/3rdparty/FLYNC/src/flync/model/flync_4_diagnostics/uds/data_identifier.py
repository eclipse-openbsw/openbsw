"""
Defines UDS Data Identifiers (DIDs) as read via 0x22, written via 0x2E, and controlled
via 0x2F.

InputOutputControlByIdentifier (0x2F) is configured *here* rather than on a service model:
ISO 14229-1 carries the ``inputOutputControlParameter`` per identifier, so which control
operations exist - and what the ``controlState`` looks like - is a property of the
identifier, exactly like :attr:`DataIdentifier.read_data` and
:attr:`DataIdentifier.write_data`.
"""

from typing import Annotated, List, Literal, Optional, Union

from pydantic import BeforeValidator, Field, model_validator

from flync.core.annotations import Implied, ImpliedStrategy
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.generic import validate_list_items_unique

from .datatypes import DiagDataRecord
from .subfunctions import IOControlParameter, coerce_int

DIDAccess = Literal["read", "write", "read_write"]

#: One ``inputOutputControlParameter``: an ISO 14229-1 name, or a manufacturer-specific value.
IOControlParameterEntry = Union[IOControlParameter, Annotated[int, Field(ge=0x00, le=0xFF)]]


def _default_io_control_parameters() -> List[IOControlParameterEntry]:
    """Return the default set of supported control parameters: shortTermAdjustment only."""

    return ["short_term_adjustment"]


class DIDIOControl(FLYNCBaseModel):
    """
    Declares that a DID can be controlled with InputOutputControlByIdentifier (0x2F), and
    with which control parameters.

    Parameters
    ----------
    supported_parameters : list of Literal["return_control_to_ecu", "reset_to_default", \
    "freeze_current_state", "short_term_adjustment"] or int, optional
        The ``inputOutputControlParameter`` values accepted for this DID. An ``int`` carries
        a vehicle-manufacturer specific parameter (0x04-0xFF). Defaults to
        ``["short_term_adjustment"]``.

    control_state : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Layout of the ``controlState`` record sent with ``short_term_adjustment``. Required
        when that parameter is supported.

    description : str, optional
        Human-readable description of the control behaviour.
    """

    supported_parameters: List[IOControlParameterEntry] = Field(default_factory=_default_io_control_parameters)
    control_state: Optional[DiagDataRecord] = Field(default=None)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_control_state_present_for_short_term_adjustment(self) -> "DIDIOControl":
        """
        Raise when ``short_term_adjustment`` is offered without the record it carries.
        """

        validate_list_items_unique(list(self.supported_parameters), "IO control parameters")
        if "short_term_adjustment" in self.supported_parameters and self.control_state is None:
            raise err_major(
                "IO control supports 'short_term_adjustment' but declares no control_state",
                category=Category.REQUIRED,
                error_number="296",
            )
        return self

    def control_enable_mask_byte_length(self) -> int:
        """
        Return the width in bytes of the ``controlEnableMaskRecord`` for this DID.

        ISO 14229-1 derives it from the number of individually controllable items, i.e. the
        number of fields of :attr:`control_state`, rather than configuring it separately.
        """

        if self.control_state is None:
            return 0
        return -(-len(self.control_state.fields) // 8)


class DataIdentifier(FLYNCBaseModel):
    """
    A UDS Data Identifier, readable with ReadDataByIdentifier (0x22) and/or writable with
    WriteDataByIdentifier (0x2E).

    Stored one per file under ``communication/diagnostics/uds/dids/``; the file name provides
    :attr:`name`.

    Parameters
    ----------
    name : str
        Name of the DID, implied from the file name on disk.

    did : int
        The 16-bit data identifier, e.g. ``0xF190``.

    access : Literal["read", "write", "read_write"], optional
        How the DID may be accessed. Defaults to ``"read"``.

    read_data : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Layout of the positive response to ReadDataByIdentifier. Required when the DID is
        readable.

    write_data : :class:`~flync.model.flync_4_diagnostics.uds.datatypes.DiagDataRecord`, optional
        Layout of the WriteDataByIdentifier request data. Required when the DID is writable.
        When omitted for a ``"read_write"`` DID the layout of :attr:`read_data` applies.

    io_control : :class:`DIDIOControl`, optional
        Declares that InputOutputControlByIdentifier (0x2F) may control this DID. ``None``
        means the DID is not IO-controllable.

    access_profile : str, optional
        Name of the access profile (declared by the UDS server offering this DID) required
        to access the DID. ``None`` means that server's default access profile applies.

    description : str, optional
        Human-readable description of the DID.
    """

    name: Annotated[str, Implied(strategy=ImpliedStrategy.FILE_NAME)] = Field()
    did: Annotated[int, BeforeValidator(coerce_int), Field(ge=0x0000, le=0xFFFF)] = Field()
    access: DIDAccess = Field(default="read")
    read_data: Optional[DiagDataRecord] = Field(default=None)
    write_data: Optional[DiagDataRecord] = Field(default=None)
    io_control: Optional[DIDIOControl] = Field(default=None)
    access_profile: Optional[str] = Field(default=None)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_data_matches_access(self) -> "DataIdentifier":
        """
        Raise when a DID declares a data layout its ``access`` does not use, or omits one it needs.
        """

        if self.access in ("read", "read_write") and self.read_data is None:
            raise err_major(
                "DID '{did_name}' is declared '{access}' but has no read_data",
                category=Category.REQUIRED,
                error_number="259",
                did_name=self.name,
                access=self.access,
            )
        if self.access == "write" and self.write_data is None:
            raise err_major(
                "DID '{did_name}' is declared 'write' but has no write_data",
                category=Category.REQUIRED,
                error_number="260",
                did_name=self.name,
            )
        if self.access == "read" and self.write_data is not None:
            raise err_major(
                "DID '{did_name}' is declared 'read' but defines write_data",
                category=Category.CONSISTENCY,
                error_number="261",
                did_name=self.name,
            )
        if self.access == "write" and self.read_data is not None:
            raise err_major(
                "DID '{did_name}' is declared 'write' but defines read_data",
                category=Category.CONSISTENCY,
                error_number="283",
                did_name=self.name,
            )
        return self
