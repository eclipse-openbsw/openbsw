"""
Diagnostics-local payload description.

UDS payloads (DID data, routine request/response data) are plain byte records rather than
signal- or SOME/IP-typed structures, so this module carries its own small, self-contained
field/record model instead of depending on :mod:`flync.model.flync_4_signal` or
:mod:`flync.model.flync_4_someip`. Only the neutral primitives from
:mod:`flync.core.datatypes` (:class:`~flync.core.datatypes.ValueRange` and
:class:`~flync.core.datatypes.ValueTable`) are reused.
"""

from typing import Annotated, List, Literal, Optional

from pydantic import Field, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes import ValueRange, ValueTable
from flync.core.utils.exceptions import Category, err_major

DiagPrimitiveType = Literal[
    "bool",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
    "int8",
    "int16",
    "int32",
    "int64",
    "float32",
    "float64",
    "ascii",
    "bytes",
]

#: Fixed width in bits of every primitive type that has one. ``ascii`` and ``bytes`` are
#: variable-width and take their length from :attr:`DiagField.bit_length`.
FIXED_WIDTHS: dict[str, int] = {
    "bool": 1,
    "uint8": 8,
    "uint16": 16,
    "uint32": 32,
    "uint64": 64,
    "int8": 8,
    "int16": 16,
    "int32": 32,
    "int64": 64,
    "float32": 32,
    "float64": 64,
}


class DiagScaling(FLYNCBaseModel):
    """
    Linear conversion from the raw transmitted value to the physical value.

    ``physical = raw * factor + offset``

    Parameters
    ----------
    factor : float, optional
        Multiplier applied to the raw value. Defaults to ``1.0``.

    offset : float, optional
        Constant added after scaling. Defaults to ``0.0``.

    unit : str, optional
        Physical unit of the scaled value, e.g. ``"km/h"``.
    """

    factor: float = Field(default=1.0)
    offset: float = Field(default=0.0)
    unit: Optional[str] = Field(default=None)


class DiagField(FLYNCBaseModel):
    """
    A single field inside a diagnostic data record.

    Parameters
    ----------
    name : str
        Name of the field, unique within its record.

    type : DiagPrimitiveType
        Primitive type of the field.

    bit_offset : int
        Offset of the field's first bit from the start of the record. Bit 0 is the
        most significant bit of the first byte.

    bit_length : int, optional
        Width of the field in bits. Required for the variable-width types ``ascii`` and
        ``bytes``; for every other type it defaults to the type's fixed width and, when
        given explicitly, must match it.

    endianness : Literal["BE", "LE"], optional
        Byte order of the field. Defaults to big-endian (``"BE"``), which is what UDS uses.

    scaling : :class:`DiagScaling`, optional
        Raw-to-physical conversion for this field.

    value_range : :class:`~flync.core.datatypes.ValueRange`, optional
        Inclusive range of valid raw values.

    value_table : list of :class:`~flync.core.datatypes.ValueTable`, optional
        Enumeration of named raw values.

    description : str, optional
        Human-readable description of the field.
    """

    name: str = Field()
    type: DiagPrimitiveType = Field()
    bit_offset: Annotated[int, Field(ge=0)] = Field()
    bit_length: Optional[Annotated[int, Field(gt=0)]] = Field(default=None)
    endianness: Literal["BE", "LE"] = Field(default="BE")
    scaling: Optional[DiagScaling] = Field(default=None)
    value_range: Optional[ValueRange] = Field(default=None)
    value_table: Optional[List[ValueTable]] = Field(default=None)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_bit_length_matches_type(self) -> "DiagField":
        """
        Default ``bit_length`` from the type's fixed width and reject a contradicting value.
        """

        fixed = FIXED_WIDTHS.get(self.type)
        if fixed is None:
            if self.bit_length is None:
                raise err_major(
                    "Field '{field_name}' of variable-width type '{type_name}' must declare a bit_length",
                    category=Category.REQUIRED,
                    error_number="254",
                    field_name=self.name,
                    type_name=self.type,
                )
            return self

        if self.bit_length is None:
            # `validate_assignment` is on, so assign through __dict__ to avoid re-running validators.
            self.__dict__["bit_length"] = fixed
        elif self.bit_length != fixed:
            raise err_major(
                "Field '{field_name}' of type '{type_name}' has bit_length {given} but the type is {expected} bits wide",
                category=Category.CONSISTENCY,
                error_number="255",
                field_name=self.name,
                type_name=self.type,
                given=self.bit_length,
                expected=fixed,
            )
        return self

    @property
    def bit_end(self) -> int:
        """
        Offset of the first bit *after* this field.
        """

        return self.bit_offset + (self.bit_length or 0)


class DiagDataRecord(FLYNCBaseModel):
    """
    An ordered, non-overlapping set of fields making up one UDS payload.

    Parameters
    ----------
    byte_length : int
        Total length of the record in bytes.

    fields : list of :class:`DiagField`, optional
        Fields contained in the record. May be empty for a record whose contents are not
        modelled in detail.

    description : str, optional
        Human-readable description of the record.
    """

    byte_length: Annotated[int, Field(ge=0)] = Field()
    fields: List[DiagField] = Field(default_factory=list)
    description: Optional[str] = Field(default=None)

    @model_validator(mode="after")
    def validate_field_names_unique(self) -> "DiagDataRecord":
        """
        Raise when two fields of the record share a name.
        """

        seen: set[str] = set()
        for field in self.fields:
            if field.name in seen:
                raise err_major(
                    "Duplicate field name '{field_name}' in diagnostic data record",
                    category=Category.UNIQUENESS,
                    error_number="256",
                    field_name=field.name,
                )
            seen.add(field.name)
        return self

    @model_validator(mode="after")
    def validate_fields_fit_and_do_not_overlap(self) -> "DiagDataRecord":
        """
        Raise when a field reaches beyond ``byte_length`` or overlaps a previous field.
        """

        capacity = self.byte_length * 8
        for field in self.fields:
            if field.bit_end > capacity:
                raise err_major(
                    "Field '{field_name}' ends at bit {end} which exceeds the record length ({capacity} bits)",
                    category=Category.VALUE_RANGE,
                    error_number="257",
                    field_name=field.name,
                    end=field.bit_end,
                    capacity=capacity,
                )

        ordered = sorted(self.fields, key=lambda f: f.bit_offset)
        for earlier, later in zip(ordered, ordered[1:]):
            if later.bit_offset < earlier.bit_end:
                raise err_major(
                    "Field '{later_name}' overlaps field '{earlier_name}' in the diagnostic data record",
                    category=Category.CONSISTENCY,
                    error_number="258",
                    later_name=later.name,
                    earlier_name=earlier.name,
                )
        return self
