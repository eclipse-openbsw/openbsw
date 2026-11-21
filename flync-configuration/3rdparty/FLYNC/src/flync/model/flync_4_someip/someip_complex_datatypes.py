"""defines the complex SOME/IP datatypes (struct, array, union, typedef)"""

from typing import Annotated, List, Literal, Optional

from pydantic import Field, field_validator

from flync.core.base_models import FLYNCBaseModel
from flync.core.datatypes import Datatype
from flync.model.flync_4_someip.someip_simple_datatypes import (
    Bitfield,
    Boolean,
    DynamicLengthString,
    Enum,
    FixedLengthString,
    Floats,
    Ints,
)


class ComplexDatatype(Datatype):
    """
    Base class for complex datatypes such as structures, arrays, or unions.

    Parameters
    ----------
    name : str
        Unique name of the datatype.

    description : str, optional
        Human-readable description of the datatype.

    type : str
        Discriminator identifying the concrete complex datatype kind.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values. Defaults to
        big-endian ("BE").
    """


class ArrayType(ComplexDatatype):
    """
    Generic multidimensional array type.

    Parameters
    ----------
    name : str, optional
        Name of Array (defaults to ``"Array"``).

    type : Literal["array"]
        Discriminator identifying this datatype as an array.

    dimensions : List[:class:`ArrayDimension`]
        Ordered list of array dimensions (outer → inner).
        Must contain at least one dimension.

    element_type : :class:`AllTypes`
        Datatype of the innermost array element.
        This may itself be a primitive, struct, union, or another array type.
    """

    name: str = Field(default="Array")
    type: Literal["array"] = Field("array")
    dimensions: List["ArrayDimension"] = Field(
        min_length=1,
        description="Ordered list of array dimensions (outer → inner)",
    )
    element_type: "AllTypes" = Field(description="Datatype of the innermost array element")


class ArrayDimension(FLYNCBaseModel):
    """
    Describes a single array dimension.

    Parameters
    ----------
    kind : Literal["fixed", "dynamic"]
        Specifies whether the dimension has a fixed size or a dynamically encoded length.

    length : int, optional
        Number of elements for a fixed-length dimension.
        Must be greater than 0. Only valid when ``kind="fixed"``.

    length_of_length_field : Literal[0, 8, 16, 32], optional
        Size in bits of the length field that precedes the array data for a dynamic dimension.
        Only valid when ``kind="dynamic"``.

    upper_limit : int, optional
        Upper bound on the number of elements.
        Must be greater than 0.

    lower_limit : int, optional
        Lower bound on the number of elements.
        Must be greater than or equal to 0.

    bit_alignment : Literal[8, 16, 32, 64, 128, 256], optional
        Optional padding alignment in bits applied after this dimension.
    """

    kind: Literal["fixed", "dynamic"]
    # Fixed-length dimension
    length: Optional[int] = Field(
        default=None,
        gt=0,
        description="Number of elements for fixed-length dimension",
    )
    # Dynamic-length dimension
    length_of_length_field: Optional[Literal[0, 8, 16, 32]] = Field(
        default=None,
        description="Length of length-field in bits for dynamic dimension",
    )
    upper_limit: Optional[int] = Field(default=None, gt=0, description="Upper bound of elements")
    lower_limit: Optional[int] = Field(default=None, ge=0, description="Lower bound of elements")
    bit_alignment: Optional[Literal[8, 16, 32, 64, 128, 256]] = Field(
        default=None,
        description="Optional padding alignment after this dimension",
    )

    @field_validator("length_of_length_field", mode="after")
    @classmethod
    def validate(cls, value, info):
        kind = info.data["kind"]
        if kind == "dynamic":
            assert value > 0, "Length of length-field must be > 0 for dynamic arrays"

        return value


class Struct(ComplexDatatype):
    """
    Structured datatype composed of multiple ordered members.

    A struct groups several datatypes into a single composite element that is serialized in the order the members are defined.

    Parameters
    ----------
    type : Literal["struct"]
        Discriminator used to identify this datatype.

    members : List[AllTypes]
        Ordered list of datatypes that form the members of the struct.

    bit_alignment : Literal[8, 16, 32, 64, 128, 256]
        Optional padding alignment applied after the struct to ensure the next parameter starts at the specified bit boundary.

    length_of_length_field : Literal[0, 8, 16, 32]
        Size of the optional length field in bits that prefixes the struct.
        A value of 0 indicates that no length field is present.
    """

    type: Literal["struct"] = Field("struct")
    members: List["AllTypes"] = Field(description="the members of the struct")  # type: ignore
    bit_alignment: Literal[8, 16, 32, 64, 128, 256] = Field(
        default=8,
        description="defines the optional alignment padding that can be added after the variable length data element like struct to "
        "fix the alignment of the next parameter to 8, 16, 32, 64, 128, or 256 bits.",
    )
    length_of_length_field: Literal[0, 8, 16, 32] = Field(
        default=0,
        description="defines the length of the length-field in bits for the struct",
    )


class Typedef(ComplexDatatype):
    """
    Alias datatype that references another datatype definition.

    A typedef introduces an alternative name for an existing datatype without changing its underlying structure or serialization behavior.

    Parameters
    ----------
    type : Literal["typedef"]
        Discriminator used to identify this datatype.

    name : str
        Name of the typedef reference.

    datatyperef : AllTypes
        Referenced datatype definition that this typedef aliases.
    """

    type: Literal["typedef"] = Field("typedef")
    name: str = Field(description="Name of the typedef reference")
    datatyperef: "AllTypes" = Field(description="Referenced datatype definition")  # type: ignore


class UnionMember(Datatype):
    """
    Represents a single member entry of a union datatype.

    Each union member defines a possible datatype that may be present, together with its selector index and a descriptive name.

    Parameters
    ----------
    type : AllTypes
        Member datatype (discriminated by its ``type`` field).

    index : int
        Index of the union member.
        This value is used in the serialized union to indicate which member is currently active.
        Must be greater than or equal to 0.

    name : str
        Name of the union member.

    mandatory : bool, optional
        Whether the union member is mandatory (defaults to ``None``).
    """

    type: Annotated[
        "AllTypes",
        Field(description="member datatype (discriminated by its 'type' field)"),
    ]
    index: Annotated[int, Field(description="index of the union member", strict=True, ge=0)]
    name: Annotated[str, Field(description="name of the union member")]

    mandatory: Annotated[
        Optional[bool],
        Field(description="whether the union member is mandatory", default=None),
    ] = None

    @field_validator("type", mode="before")
    def _wrap_string_type(cls, v):
        if isinstance(v, str):
            s = v.strip().lower()
            return {"type": s}
        return v


class Union(Datatype):
    """
    Represents a union datatype.

    A union allows exactly one of several possible member datatypes to be encoded at runtime. The active member is identified using a type
    selector field.

    Parameters
    ----------
    name : str, optional
        Name of the Union (defaults to ``"Union"``).

    type : Literal["union"]
        Discriminator used to identify this datatype.

    members : list of :class:`UnionMember`
        List of the allowed datatypes a union can contain.

    bit_alignment : Literal[8, 16, 32, 64, 128, 256], optional
        Defines the optional alignment padding that can be added after the union to fix the alignment of the next parameter to 8, 16, 32, 64,
        128 or 256 bits.

    length_of_length_field : Literal[0, 8, 16, 32], optional
        Defines the length of the length-field in bits for the union.

    length_of_type_field : Literal[0, 8, 16, 32], optional
        Defines the length of the type-selector field in bits for the union.
    """

    name: str = Field(default="Union")
    type: Literal["union"] = Field("union")
    members: List[UnionMember] = Field(description="list of the allowed datatypes a union can have")
    bit_alignment: Literal[8, 16, 32, 64, 128, 256] = Field(
        default=8,
        description="defines the optional alignment padding that can be added after union to fix the alignment of the next parameter \
                to 8, 16, 32, 64, 128 or 256 bits.",
    )
    length_of_length_field: Literal[0, 8, 16, 32] = Field(
        default=32,
        description="defines the length of the length-field in bits for the union",
    )
    length_of_type_field: Literal[0, 8, 16, 32] = Field(
        default=32,
        description="defines the length of the type-selector-field in bits for the union",
    )


AllTypes = Annotated[
    Ints | Floats | Enum | Boolean | Struct | Typedef | Union | ArrayType | DynamicLengthString | FixedLengthString | Bitfield,
    Field(discriminator="type"),
]
"Collection of all dataypes"
