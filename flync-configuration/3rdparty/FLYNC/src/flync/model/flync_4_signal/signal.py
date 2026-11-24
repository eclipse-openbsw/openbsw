"""
Defines signals and signal groups for FLYNC.

Provides :class:`Signal` and :class:`SignalGroup` as the reusable definitions, their placed counterparts
:class:`SignalInstance` and :class:`SignalGroupInstance`, the supported base types in :class:`SignalDataType`
and the packing order in :class:`InstancePlacement`. Also validates that initial values fit the declared
data type and that placed bit ranges stay consistent.
"""

from enum import Enum
from typing import List, Literal, Optional

from pydantic import Field, field_serializer, field_validator, model_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major
from flync.core.validators.bit_ranges import (
    BitRange,
    check_bit_ranges_no_overlap,
    collect_bit_ranges,
)
from flync.model.flync_4_signal.value_encoding import (
    BitfieldGroup,
    BitfieldState,
    BitfieldTextTable,
    BitmaskFlag,
    BitmaskFlags,
    TextEntry,
    TextTable,
    ValueEncoding,
)

__all__ = [
    "SignalDataType",
    "InstancePlacement",
    "Signal",
    "SignalInstance",
    "SignalGroup",
    "SignalGroupInstance",
    "TextEntry",
    "TextTable",
    "BitfieldState",
    "BitfieldGroup",
    "BitfieldTextTable",
    "BitmaskFlag",
    "BitmaskFlags",
    "ValueEncoding",
]


class SignalDataType(str, Enum):
    """
    Supported signal base data types for CAN, LIN, FlexRay, and Ethernet.
    """

    UINT8 = "uint8"
    UINT16 = "uint16"
    UINT32 = "uint32"
    UINT64 = "uint64"
    INT8 = "int8"
    INT16 = "int16"
    INT32 = "int32"
    INT64 = "int64"
    FLOAT32 = "float32"
    FLOAT64 = "float64"
    CHAR = "char"
    BYTEARRAY = "bytearray"

    def natural_bit_width(self) -> Optional[int]:
        """Canonical bit width for this type. Size for single element for ``char`` and ``bytearray``."""
        _widths = {
            SignalDataType.UINT8: 8,
            SignalDataType.UINT16: 16,
            SignalDataType.UINT32: 32,
            SignalDataType.UINT64: 64,
            SignalDataType.INT8: 8,
            SignalDataType.INT16: 16,
            SignalDataType.INT32: 32,
            SignalDataType.INT64: 64,
            SignalDataType.FLOAT32: 32,
            SignalDataType.FLOAT64: 64,
            SignalDataType.CHAR: 8,
            SignalDataType.BYTEARRAY: 8,
        }
        return _widths.get(self)

    def is_float(self) -> bool:
        """Return ``True`` for float types (``float32``, ``float64``)."""
        return self in (SignalDataType.FLOAT32, SignalDataType.FLOAT64)

    def is_unsigned_integer(self) -> bool:
        """Return ``True`` for unsigned integer types."""
        return self in (
            SignalDataType.UINT8,
            SignalDataType.UINT16,
            SignalDataType.UINT32,
            SignalDataType.UINT64,
        )

    def is_signed_integer(self) -> bool:
        """Return ``True`` for signed integer types."""
        return self in (
            SignalDataType.INT8,
            SignalDataType.INT16,
            SignalDataType.INT32,
            SignalDataType.INT64,
        )

    def is_complex_datattype(self) -> bool:
        """Return ``True`` for complex datatypes."""
        return self in (
            SignalDataType.CHAR,
            SignalDataType.BYTEARRAY,
        )


class InstancePlacement(FLYNCBaseModel):
    """
    Shared placement metadata for signal and signal-group instances within
    a PDU.

    Parameters
    ----------
    bit_position : int, optional
        Non-negative bit offset in the PDU.
    update_indication_bit_position : int, optional
        Bit position used to indicate that the value has been updated.
    endianness : Literal["BE", "LE"]
        Byte order for this instance.  Defaults to ``"little_endian"``.
    """

    bit_position: Optional[int] = Field(default=None, ge=0)
    update_indication_bit_position: Optional[int] = Field(default=None)
    endianness: Literal["BE", "LE"] = Field(default="LE")


class Signal(FLYNCBaseModel):
    """
    Logical or physical data element transmitted within a communication
    message.

    Parameters
    ----------
    name : str
        Name of the signal.
    description : str, optional
        Optional textual description of the signal.
    bit_length : int
        Length of the signal in bits.
        Must be greater than 0.
    data_type : :class:`SignalDataType`
        Base data type of the signal.
    factor : float
        Multiplication factor applied to the raw value to obtain the
        physical value.  Defaults to ``1.0``.
    offset : float
        Additive offset applied after scaling to obtain the physical
        value.  Defaults to ``0.0``.
    lower_limit : float, optional
        Minimum physical value of the signal.
    upper_limit : float, optional
        Maximum physical value of the signal.
    unit : str, optional
        Physical unit of the signal (e.g. ``"km/h"``, ``"°C"``).
    initial_value : float | int | bytes | str, optional
        Optional initial value of the signal at startup, expressed as the
        **raw** wire value (no ``factor``/``offset`` applied).  For ``char``
        signals pass a ``str``; for ``bytearray`` signals pass ``bytes``.
    value_encoding : :class:`ValueEncoding`, optional
        Optional conversion from raw values to text labels.  One of
        :class:`TextTable` (inclusive range → label; omit ``to_value``
        for a single value, it defaults to ``from_value``),
        :class:`BitfieldTextTable` (named bitfield groups, one active
        state per group), or :class:`BitmaskFlags` (independent on/off
        flags, multiple may be active simultaneously).  May be combined
        with ``factor``/``offset``/``unit`` to express a mixed
        linear-and-text-table conversion.
    """

    name: str = Field()
    description: Optional[str] = Field(default=None)
    bit_length: int = Field(gt=0)
    data_type: SignalDataType = Field()
    factor: float = Field(default=1.0)
    offset: float = Field(default=0.0)
    lower_limit: Optional[float] = Field(default=None)
    upper_limit: Optional[float] = Field(default=None)
    unit: Optional[str] = Field(default=None)
    initial_value: Optional[float | int | bytes | str] = Field(default=None)
    value_encoding: Optional[ValueEncoding] = Field(default=None)

    @field_serializer("data_type")
    def serialize_data_type(self, data_type: SignalDataType) -> str:
        return data_type.value

    @field_validator("factor")
    @classmethod
    def _factor_nonzero(cls, v: float) -> float:
        """Reject a zero factor, which would collapse all physical values."""
        if not v:
            raise err_major(
                "factor must not be zero; a zero factor collapses all physical values to the offset",
                category=Category.VALUE_RANGE,
                error_number="113",
            )
        return v

    @model_validator(mode="after")
    def _validate_bit_length_for_data_type(self) -> "Signal":
        natural = self.data_type.natural_bit_width()
        if self.data_type.is_complex_datattype():
            if natural is not None and (self.bit_length < natural or self.bit_length % natural != 0):
                raise err_major(
                    "{data_type} requires {natural} bits or a multiple of that; got bit_length={bit_length}",
                    data_type=self.data_type.value,
                    natural=natural,
                    bit_length=self.bit_length,
                    category=Category.VALUE_RANGE,
                    error_number="114",
                )
            return self
        elif self.data_type.is_float():
            if self.bit_length != natural:
                raise err_major(
                    "{data_type} requires exactly {natural} bits; got bit_length={bit_length}",
                    data_type=self.data_type.value,
                    natural=natural,
                    bit_length=self.bit_length,
                    category=Category.VALUE_RANGE,
                    error_number="115",
                )
        elif natural is not None and self.bit_length > natural:
            raise err_major(
                "bit_length={bit_length} exceeds the natural width of {data_type} ({natural} bits)",
                bit_length=self.bit_length,
                data_type=self.data_type.value,
                natural=natural,
                category=Category.VALUE_RANGE,
                error_number="116",
            )
        return self

    @model_validator(mode="after")
    def _validate_limits(self) -> "Signal":
        if self.lower_limit is not None and self.upper_limit is not None:
            if self.lower_limit > self.upper_limit:
                raise err_major(
                    "lower_limit ({lower_limit}) must not exceed upper_limit ({upper_limit})",
                    lower_limit=self.lower_limit,
                    upper_limit=self.upper_limit,
                    category=Category.CONSISTENCY,
                    error_number="117",
                )
        return self

    @model_validator(mode="after")
    def _validate_initial_value(self) -> "Signal":
        if self.initial_value is not None:
            _check_initial_value(self.initial_value, self.data_type, self.bit_length)
        return self

    @model_validator(mode="after")
    def _validate_value_encoding(self) -> "Signal":
        if self.value_encoding is None:
            return self
        dt = self.data_type
        if dt.is_float() or dt in (SignalDataType.CHAR, SignalDataType.BYTEARRAY):
            raise err_major(
                "value_encoding is not supported for {data_type} signals; only integer data types allow text-table or bitfield encodings",
                data_type=dt.value,
                category=Category.COMPATIBILITY,
                error_number="118",
            )
        if isinstance(self.value_encoding, TextTable):
            self._check_text_table_in_range(self.value_encoding)
        elif isinstance(self.value_encoding, BitfieldTextTable):
            self._check_bitfield_text_table_in_range(self.value_encoding)
        elif isinstance(self.value_encoding, BitmaskFlags):
            self._check_bitmask_flags_in_range(self.value_encoding)
        return self

    def _raw_value_bounds(self) -> tuple[int, int]:
        """Inclusive ``(lo, hi)`` representable raw range for this signal."""
        if self.data_type.is_unsigned_integer():
            return 0, (1 << self.bit_length) - 1
        return -(1 << (self.bit_length - 1)), (1 << (self.bit_length - 1)) - 1

    def _check_text_table_in_range(self, table: TextTable) -> None:
        lo, hi = self._raw_value_bounds()
        for entry in table.entries:
            assert entry.from_value is not None and entry.to_value is not None
            if not (lo <= entry.from_value <= hi and lo <= entry.to_value <= hi):
                raise err_major(
                    "TextTable entry '{label}' range [{from_value}, {to_value}] is outside "
                    "the representable range [{lo}, {hi}] for {data_type} with bit_length={bit_length}",
                    label=entry.label,
                    from_value=entry.from_value,
                    to_value=entry.to_value,
                    lo=lo,
                    hi=hi,
                    data_type=self.data_type.value,
                    bit_length=self.bit_length,
                    category=Category.VALUE_RANGE,
                    error_number="119",
                )

    def _check_bitfield_text_table_in_range(self, table: BitfieldTextTable) -> None:
        max_mask = (1 << self.bit_length) - 1
        for group in table.groups:
            if group.mask > max_mask:
                raise err_major(
                    "BitfieldGroup '{name}' mask {mask} exceeds the representable range for {data_type} with bit_length={bit_length}",
                    name=group.name,
                    mask=f"{group.mask:#x}",
                    data_type=self.data_type.value,
                    bit_length=self.bit_length,
                    category=Category.VALUE_RANGE,
                    error_number="120",
                )

    def _check_bitmask_flags_in_range(self, table: BitmaskFlags) -> None:
        max_mask = (1 << self.bit_length) - 1
        for flag in table.flags:
            if flag.mask > max_mask:
                raise err_major(
                    "BitmaskFlag '{label}' mask {mask} exceeds the representable range for {data_type} with bit_length={bit_length}",
                    label=flag.label,
                    mask=f"{flag.mask:#x}",
                    data_type=self.data_type.value,
                    bit_length=self.bit_length,
                    category=Category.VALUE_RANGE,
                    error_number="121",
                )


class SignalInstance(InstancePlacement):
    """
    Placement of a :class:`Signal` at a specific bit offset within a PDU.

    Parameters
    ----------
    signal : :class:`Signal`
        Signal being instantiated.
    """

    signal: Signal = Field()


class SignalGroup(FLYNCBaseModel):
    """
    A reusable group of signal instances transmitted together within a PDU.

    Each contained :class:`SignalInstance` carries a ``bit_position``
    interpreted as an offset **relative to the group's origin** — that is,
    relative to the :attr:`SignalGroupInstance.bit_position` where the group
    is placed inside a PDU.  Signal instances without a ``bit_position`` are
    treated as unplaced and skipped during overlap and footprint checks.

    Parameters
    ----------
    name : str
        Name of the signal group.
    description : str, optional
        Optional textual description of the group.
    signals : list of :class:`SignalInstance`
        Non-empty list of placed signal instances contained in this group.
    """

    name: str = Field()
    description: Optional[str] = Field(default=None)
    signals: List[SignalInstance] = Field(min_length=1)

    @model_validator(mode="after")
    def _validate_signals_no_overlap(self) -> "SignalGroup":
        """Reject signal instances whose ranges overlap within the group."""
        ranges = collect_bit_ranges(self.signals, _signal_instance_range)
        check_bit_ranges_no_overlap(f"SignalGroup '{self.name}'", ranges)
        return self


class SignalGroupInstance(InstancePlacement):
    """
    Placement of a :class:`SignalGroup` at a specific bit offset within a PDU.

    Parameters
    ----------
    signal_group : :class:`SignalGroup`
        Signal group being instantiated.
    """

    signal_group: SignalGroup = Field()


def _signal_instance_range(si: SignalInstance) -> Optional[BitRange]:
    """Return the bit range of a placed :class:`SignalInstance` or ``None``.

    The range is expressed in the same coordinate system as
    ``si.bit_position`` — absolute when the instance lives directly in a PDU
    and relative to the group origin when the instance lives in a
    :class:`SignalGroup`.
    """
    if si.bit_position is None:
        return None
    return (
        si.signal.name,
        si.bit_position,
        si.bit_position + si.signal.bit_length,
    )


def _signal_group_footprint(sg: "SignalGroup") -> int:
    """Bit footprint required by a :class:`SignalGroup`'s placed instances.

    Equals ``max(bit_position + bit_length)`` over all signal instances that
    have a ``bit_position``; ``0`` when every instance is unplaced.
    """
    max_end = 0
    for si in sg.signals:
        r = _signal_instance_range(si)
        if r is not None and r[2] > max_end:
            max_end = r[2]
    return max_end


def _check_initial_value(iv: object, dt: SignalDataType, bit_length: int) -> None:
    """Validate if the type is fit or not."""
    if dt == SignalDataType.BYTEARRAY:
        if not isinstance(iv, bytes):
            raise err_major(
                "initial_value for bytearray signal must be bytes; got {got}", got=type(iv).__name__, category=Category.FORMAT, error_number="122"
            )
    elif dt == SignalDataType.CHAR:
        if not isinstance(iv, str):
            raise err_major(
                "initial_value for char signal must be str; got {got}", got=type(iv).__name__, category=Category.FORMAT, error_number="123"
            )
    elif dt.is_float():
        if not isinstance(iv, (float, int)) or isinstance(iv, bool):
            raise err_major(
                "initial_value for {data_type} must be numeric; got {got}",
                data_type=dt.value,
                got=type(iv).__name__,
                category=Category.FORMAT,
                error_number="124",
            )
    elif dt.is_unsigned_integer() or dt.is_signed_integer():
        _check_integer_initial_value(iv, dt, bit_length)


def _check_integer_initial_value(iv: object, dt: SignalDataType, bit_length: int) -> None:
    """Validate that an integer is the right type and fits in bit_length."""
    if not isinstance(iv, int) or isinstance(iv, bool):
        raise err_major(
            "initial_value for {data_type} must be int; got {got}",
            data_type=dt.value,
            got=type(iv).__name__,
            category=Category.FORMAT,
            error_number="125",
        )
    if dt.is_unsigned_integer():
        lo, hi = 0, (1 << bit_length) - 1
    else:
        lo = -(1 << (bit_length - 1))
        hi = (1 << (bit_length - 1)) - 1
    if not (lo <= iv <= hi):
        raise err_major(
            "initial_value {value} is outside the representable range [{lo}, {hi}] for {data_type} with bit_length={bit_length}",
            value=iv,
            lo=lo,
            hi=hi,
            data_type=dt.value,
            bit_length=bit_length,
            category=Category.VALUE_RANGE,
            error_number="126",
        )
