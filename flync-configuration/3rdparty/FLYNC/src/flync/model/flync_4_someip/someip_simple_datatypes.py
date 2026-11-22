"""defines the simple SOME/IP datatypes (primitives, bitfields, enums and strings)"""

from typing import Annotated, ClassVar, List, Literal, Optional

from pydantic import (
    BaseModel,
    Field,
    ValidationInfo,
    field_validator,
    model_validator,
)

from flync.core.datatypes import Datatype
from flync.core.utils.exceptions import Category, err_minor


class PrimitiveDatatype(Datatype):
    """
    Base class for primitive datatypes such as integers, floating-point values, or booleans.

    Parameters
    ----------
    name : str
        Unique name of the datatype.

    description : str, optional
        Human-readable description of the datatype.

    type : str
        Discriminator identifying the concrete primitive datatype kind.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").
    """


class Boolean(PrimitiveDatatype):
    """
    Boolean primitive datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"BOOLEAN"``.

    type : Literal["boolean"]
        Discriminator identifying the primitive boolean datatype.

    signed : Literal[False]
        Indicates that the boolean is unsigned.

    endianness : Literal["BE"]
        Byte order used for encoding. Big-Endian ("BE").

    bit_size : int
        Storage size in bits: 8.
        Defaults to 8.

    """

    name: str = Field(default="BOOLEAN")
    type: Literal["boolean"] = Field("boolean")  # type: ignore
    signed: Literal[False] = Field(False)
    endianness: Literal["BE"] = "BE"
    bit_size: Annotated[int, Field(ge=8, le=8, default=8)]


class BaseInt(PrimitiveDatatype):
    """
    Base class for all integer primitive datatypes.

    This class provides shared semantics for signed and unsigned integer representations and defines common descriptive metadata.

    """


class BaseFloat(PrimitiveDatatype):
    """
    Base class for all floating-point primitive datatypes.

    This class provides shared semantics for floating-point representations and defines common descriptive metadata.

    """


class UInt8(BaseInt):
    """
    Unsigned 8-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"UINT8"``.

    type : Literal["uint8"]
        Discriminator identifying this datatype.

    signed : Literal[False]
        Indicates that the integer is unsigned.

    endianness : Literal["BE"]
        Byte order used for encoding. Big-Endian ("BE").

    bit_size : int
        Storage size in bits: 8.
        Defaults to 8.
    """

    name: str = Field(default="UINT8")
    type: Literal["uint8"] = Field("uint8")  # type: ignore
    signed: Literal[False] = Field(False)
    endianness: Literal["BE"] = Field("BE")
    bit_size: Annotated[int, Field(8)]


class UInt16(BaseInt):
    """
    Unsigned 16-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"UINT16"``.

    type : Literal["uint16"]
        Discriminator identifying this datatype.

    signed : Literal[False]
        Indicates that the integer is unsigned.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 16.
        Defaults to 16.
    """

    name: str = Field(default="UINT16")
    type: Literal["uint16"] = Field("uint16")  # type: ignore
    signed: Literal[False] = Field(False)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=16, le=16, default=16)]


class UInt32(BaseInt):
    """
    Unsigned 32-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"UINT32"``.

    type : Literal["uint32"]
        Discriminator identifying this datatype.

    signed : Literal[False]
        Indicates that the integer is unsigned.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 32.
        Defaults to 32.
    """

    name: str = Field(default="UINT32")
    type: Literal["uint32"] = Field("uint32")  # type: ignore
    signed: Literal[False] = Field(False)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=32, le=32, default=32)]


class UInt64(BaseInt):
    """
    Unsigned 64-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"UINT64"``.

    type : Literal["uint64"]
        Discriminator identifying this datatype.

    signed : Literal[False]
        Indicates that the integer is unsigned.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 64.
        Defaults to 64.
    """

    name: str = Field(default="UINT64")
    type: Literal["uint64"] = Field("uint64")  # type: ignore
    signed: Literal[False] = Field(False)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=64, le=64, default=64)]


class Int8(BaseInt):
    """
    Signed 8-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"INT8"``.

    type : Literal["int8"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the integer is signed.

    endianness : Literal["BE"]
        Byte order used for encoding. Big-Endian ("BE").

    bit_size : int
        Storage size in bits: 8.
        Defaults to 8.
    """

    name: str = Field(default="INT8")
    type: Literal["int8"] = Field("int8")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE"] = "BE"
    bit_size: Annotated[int, Field(ge=8, le=8, default=8)]


class Int16(BaseInt):
    """
    Signed 16-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"INT16"``.

    type : Literal["int16"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the integer is signed.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 16.
        Defaults to 16.
    """

    name: str = Field(default="INT16")
    type: Literal["int16"] = Field("int16")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=16, le=16, default=16)]


class Int32(BaseInt):
    """
    Signed 32-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"INT32"``.

    type : Literal["int32"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the integer is signed.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 32. Defaults to 32.
    """

    name: str = Field(default="INT32")
    type: Literal["int32"] = Field("int32")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=32, le=32, default=32)]


class Int64(BaseInt):
    """
    Signed 64-bit integer datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"INT64"``.

    type : Literal["int64"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the integer is signed.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 64. Defaults to 64.
    """

    name: str = Field(default="INT64")
    type: Literal["int64"] = Field("int64")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=64, le=64, default=64)]


class Float32(PrimitiveDatatype):
    """
    32-bit floating-point datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"FLOAT32"``.

    type : Literal["float32"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the float is signed.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 32. Defaults to 32.
    """

    name: str = Field(default="FLOAT32")
    type: Literal["float32"] = Field("float32")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=32, le=32, default=32)]


class Float64(BaseFloat):
    """
    64-bit floating-point datatype.

    Parameters
    ----------
    name : str
        Datatype name. Defaults to ``"FLOAT64"``.

    type : Literal["float64"]
        Discriminator identifying this datatype.

    signed : Literal[True]
        Indicates that the float is signed.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    bit_size : int
        Storage size in bits: 64. Defaults to 64.
    """

    name: str = Field(default="FLOAT64")
    type: Literal["float64"] = Field("float64")  # type: ignore
    signed: Literal[True] = Field(True)
    endianness: Literal["BE", "LE"] = "BE"
    bit_size: Annotated[int, Field(ge=64, le=64, default=64)]


class BitfieldEntryValue(BaseModel):
    """
    Represents a named value within a bitfield entry.

    Parameters
    ----------
    value : int
        Numeric value represented by this bitfield entry value.

    name : str
        Symbolic name associated with the value.

    description : str, optional
        Human-readable description of the value.
    """

    value: int = Field()
    name: str = Field()
    description: Optional[str] = Field("", description="Optional description")


class BitfieldEntry(BaseModel):
    """
    Describes a single field within a bitfield.

    Parameters
    ----------
    name : str
        Name of the individual bitfield.

    bitposition : int
        Bit position of the individual bitfield within the enclosing bitfield datatype. Must be greater than or equal to 0.

    description : str, optional
        Human-readable description of the field.

    values : list of :class:`BitfieldEntryValue`, optional
        Optional enumeration of named values defined for this bitfield entry.
    """

    name: str = Field(..., description="Name of the individual bitfield")
    bitposition: Annotated[int, Field(ge=0)] = Field(..., description="Bitposition for the individual bitfield")
    description: Optional[str] = Field("", description="Optional description of the field")
    values: Optional[List[BitfieldEntryValue]] = Field(
        default_factory=list,
        description="Optional values defined for the entry",
    )


class Bitfield(Datatype):
    """
    Allows modeling of SOME/IP bitfields.

    Parameters
    ----------
    name : str
        Unique name of the datatype. Defaults to "Bitfield".

    description : str, optional
        Human-readable description of the datatype.

    type : Literal["bitfield"]
        Discriminator identifying this datatype as a bitfield.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    length : Literal[8, 16, 32, 64], optional
        Size of the bitfield in bits.

    fields : list of :class:`BitfieldEntry`, optional
        List of bitfield entries that define the individual bit ranges.
        Each entry must fit into ``length`` and claim a bitposition no other entry claims.
    """

    name: str = Field(default="Bitfield")

    type: Literal["bitfield"] = Field("bitfield")

    length: Literal[8, 16, 32, 64] = Field(
        default=8,
        description="defines the possible length of the bitfield",
    )

    fields: Optional[List[BitfieldEntry]] = Field(default=None, description="List of bitfield entries")

    @model_validator(mode="after")
    def validate_length_against_fields_size(self):
        """Validate the number of defined fields fits into the bitfield length"""
        if self.fields is not None and len(self.fields) > self.length:
            raise err_minor(
                f"{self.name}: Number of defined fields ({len(self.fields)}) exceeds the bitfield length ({self.length})",
                category=Category.CONSISTENCY,
                error_number="138",
            )
        return self

    @model_validator(mode="after")
    def validate_bitfieldposition_of_entries(self):
        """Validate bitfield position for all entries must be in range"""
        if self.fields is not None:
            for field in self.fields:
                if field.bitposition >= self.length:
                    raise err_minor(
                        f"{self.name}: Bitposition of {field.name} is out of range. Must be < {self.length}, got {field.bitposition}.",
                        category=Category.VALUE_RANGE,
                        error_number="139",
                    )
        return self

    @model_validator(mode="after")
    def validate_bitpositions_to_be_unique(self):
        """Validate each bitposition is claimed by at most one entry"""
        if self.fields is not None:
            owner_by_bitposition: dict[int, str] = {}
            for field in self.fields:
                if field.bitposition in owner_by_bitposition:
                    raise err_minor(
                        f"{self.name}: Bitposition {field.bitposition} is claimed by '{owner_by_bitposition[field.bitposition]}' "
                        f"and '{field.name}'.",
                        category=Category.UNIQUENESS,
                        error_number="246",
                    )
                owner_by_bitposition[field.bitposition] = field.name
        return self


class EnumEntry(BaseModel):
    """
    Represents a single entry in an enumeration.

    Parameters
    ----------
    value : int
        Numeric value associated with the enumeration entry.

    name : str
        Symbolic name of the enumeration entry.

    description : str, optional
        Human-readable description of the enumeration entry.
    """

    value: int = Field()
    name: str = Field()
    description: str = Field("")


class Enum(Datatype):
    """
    Allows modeling SOME/IP enumerations with value, name, and description.

    Parameters
    ----------
    name : str
        Unique name of the datatype. Defaults to "Enum".

    description : str, optional
        Human-readable description of the datatype.

    type : Literal["enum"]
        Datatype discriminator identifying this datatype as an enumeration.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    base_type : Ints, optional
        Underlying integer datatype used to encode enumeration values.
        Defaults to :class:`UInt8`.

    entries : list of :class:`EnumEntry`, optional
        List of enumeration entries defining the mapping between numeric values and symbolic names.
    """

    name: str = Field(default="Enum")
    type: Literal["enum"] = Field("enum")
    base_type: "Ints" = Field(default_factory=lambda: Enum.default_base_type())
    entries: List[EnumEntry] = Field(default_factory=list)
    BASE_TYPE_RANGES: ClassVar[dict[str, tuple[int, int]]] = {
        "UInt8": (0, 2**8 - 1),
        "UInt16": (0, 2**16 - 1),
        "UInt32": (0, 2**32 - 1),
        "UInt64": (0, 2**64 - 1),
        "Int8": (-(2**7), 2**7 - 1),
        "Int16": (-(2**15), 2**15 - 1),
        "Int32": (-(2**31), 2**31 - 1),
        "Int64": (-(2**63), 2**63 - 1),
    }

    @field_validator("entries")
    @classmethod
    def validate_entries(cls, entries: list["EnumEntry"], info: ValidationInfo) -> list["EnumEntry"]:
        """
        Check that enum entries have unique values that fit into the range of the base type.

        Validation is skipped when ``base_type`` is unavailable, i.e. when it failed validation itself.
        """

        base_type = info.data.get("base_type")
        if base_type is not None:
            base_type_name = base_type.__class__.__name__
            min_value, max_value = cls.BASE_TYPE_RANGES[base_type_name]
            seen = set()
            for entry in entries:
                if entry.value in seen:
                    raise err_minor(f"Duplicate enum value: {entry.value}", category=Category.UNIQUENESS, error_number="140")
                seen.add(entry.value)
                if not (min_value <= entry.value <= max_value):
                    raise err_minor(
                        f"Enum value {entry.value} exceeds valid range for {base_type_name} ({min_value} to {max_value})",
                        category=Category.VALUE_RANGE,
                        error_number="141",
                    )
        return entries

    @staticmethod
    def default_base_type() -> UInt8:
        return UInt8(type="uint8", endianness="BE", signed=False, bit_size=8)


class BaseString(Datatype):
    """
    Base class for all string datatypes.

    Parameters
    ----------
    type : str
        Discriminator identifying the concrete string type.

    encoding : Literal["UTF-8", "UTF-16BE", "UTF-16LE"]
        Character encoding used for the string payload.
    """

    name: str = Field(default="BaseString")
    type: str = Field()
    encoding: Literal["UTF-8", "UTF-16BE", "UTF-16LE"] = Field(
        description="the encoding of the string\n\n.. needextract::\n" '\t:filter: id in ["feat_req_someip_234","feat_req_someip_235"]\n\n',
        default="UTF-8",
    )


class FixedLengthString(BaseString):
    """
    Fixed-length string datatype.

    This string occupies a fixed number of bytes on the wire.
    If the actual content is shorter than the configured length, it is padded with zero bytes.

    Parameters
    ----------
    name : str
        Name of the String. Defaults to "FixedLengthString".

    type : Literal["fixed_length_string"]
        Discriminator used to identify this datatype.

    length : int
        Total length of the string in bytes, including zero-termination and any padding. Must be greater than or equal to 1.

    length_of_length_field : Literal[0, 8, 16, 32]
        Size of the optional length field in bits.
        A value of 0 indicates that no length field is present.
    """

    name: str = Field(default="FixedLengthString")
    type: Literal["fixed_length_string"] = Field("fixed_length_string")
    length: Annotated[int, Field(ge=1)] = Field(
        description="the length of the string (including zero-termination!)\n"
        "\n"
        ".. needextract::\n"
        '\t:filter: id in ["feat_req_someip_234"]\n\n'
    )
    length_of_length_field: Literal[0, 8, 16, 32] = Field(
        default=0,
        description="defines the length of the length-field in bits of the fixed length string where 0 indicates that there is"
        "no length field present.",
    )


class DynamicLengthString(BaseString):
    """
    Dynamic-length string datatype.

    The encoded representation starts with a length field, followed by the string content and a zero-termination character.

    Parameters
    ----------
    name : str
        Name of the String. Defaults to "DynamicLengthString".

    type : Literal["dynamic_length_string"]
        Discriminator used to identify this datatype.

    max_length: Optional[int], optional
        Maximum string length in bytes. None means no limit. Must be greater than or equal to 0.

    min_length: Optional[int], optional
        Minimum string length in bytes. None means 0.

    length_of_length_field : Literal[8, 16, 32]
        Size of the length field in bits that precedes the string data.

    bit_alignment : Literal[8, 16, 32, 64, 128, 256]
        Optional padding alignment applied after the string so that the next parameter starts at the specified bit boundary.
    """

    name: str = Field(default="DynamicLengthString")
    type: Literal["dynamic_length_string"] = Field(
        default="dynamic_length_string",
        description="used internally by flync to efficiently determine the constructor to use from yaml",
    )
    max_length: Optional[int] = Field(
        default=None,
        ge=0,
        description="Maximum string length in bytes.None means no upper limit.",
    )
    min_length: Optional[int] = Field(
        default=None,
        ge=0,
        description="Minimum string length in bytes. None means 0.",
    )
    length_of_length_field: Literal[8, 16, 32] = Field(
        description="the length of the length field of the string\n\n"
        ".. needextract::\n"
        '\t:filter: id in ["feat_req_someip_237", "feat_req_someip_582", '
        '"feat_req_someip_581"]\n\n',
        default=32,
    )
    bit_alignment: Literal[8, 16, 32, 64, 128, 256] = Field(
        default=8,
        description="defines the optional alignment padding that can be added after the dynamic length string to fix the alignment of "
        "the next parameter to 8, 16, 32, 64, 128, or 256 bits.",
    )


SignedInts = Annotated[
    Int8 | Int16 | Int32 | Int64,
    Field(discriminator="type"),
]
"Collection of Signed Integer Types"

UnsignedInts = Annotated[
    UInt8 | UInt16 | UInt32 | UInt64,
    Field(discriminator="type"),
]
"Collection of Unsigned Integer Types"

Ints = Annotated[
    SignedInts | UnsignedInts,
    Field(discriminator="type"),
]
"Collection of Integer Types"

Floats = Annotated[
    Float32 | Float64,
    Field(discriminator="type"),
]
"Collection of Float Types"
