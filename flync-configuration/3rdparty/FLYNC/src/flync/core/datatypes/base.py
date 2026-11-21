"""Defines the base class each datatype shares"""

from typing import Literal, Optional

from pydantic import ConfigDict, Field

from flync.core.base_models.base_model import FLYNCBaseModel


class Datatype(FLYNCBaseModel):
    """
    Base class of every datatype.

    Parameters
    ----------
    name : str
        Unique name of the datatype.

    description : str, optional
        Human-readable description of the datatype.

    type : str
        Discriminator identifying the concrete datatype kind.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values. Defaults to big-endian ("BE").

    member_name : str, optional
        When this datatype is stored as a struct member, this field holds the member's name within the struct (which may differ from the
        type's own ``name``).
        None when the datatype is not a struct member or when member name equals the type name.
    """

    model_config = ConfigDict(frozen=True)
    name: str = Field()
    description: Optional[str] = Field(default="")
    type: str | object = Field()
    endianness: Literal["BE", "LE"] = Field("BE")
    member_name: Optional[str] = Field(default=None)
