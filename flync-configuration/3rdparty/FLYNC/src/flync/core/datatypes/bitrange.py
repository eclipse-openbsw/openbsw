"""Defines the BitRange datatype for bit-level mapping."""

from pydantic import Field

from .base import Datatype


class BitRange(Datatype):
    """
    Represents a range of bits in a signal or data array.

    Parameters
    ----------
    name : str
        Unique name of the datatype.

    description : str, optional
        Human-readable description of the datatype.

    type : str
        Datatype discriminator inherited from :class:`Datatype`.

    endianness : Literal["BE", "LE"], optional
        Byte order used for encoding multibyte values.
        Defaults to big-endian ("BE").

    start : int
        Starting bit position (inclusive).

    end : int
        Ending bit position (inclusive).
    """

    start: int = Field()
    end: int = Field()
