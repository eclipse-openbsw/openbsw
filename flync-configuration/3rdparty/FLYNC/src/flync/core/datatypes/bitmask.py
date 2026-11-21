"""Bit pattern with a mask, shared by TCAM frame masks and vehicle-state matching."""

import string
from typing import Optional, Self

from pydantic import Field, field_serializer, model_validator

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_minor

_LITERAL_FORMATS = 'a hexadecimal literal (e.g. "0x0800") or a binary literal (e.g. "0b1101")'


def _parse_literal(literal: str | int) -> tuple[int, int, int]:
    """Return ``(value, bit_width, base)`` of a ``0x``/``0b`` literal, or of a plain integer widened to whole bytes."""

    if isinstance(literal, int) and not isinstance(literal, bool):
        return literal, max(8, 8 * -(-literal.bit_length() // 8)), 16
    if isinstance(literal, str):
        compact = "".join(literal.split())
        prefix, digits = compact[:2].lower(), compact[2:]
        if prefix == "0x" and digits and all(char in string.hexdigits for char in digits):
            return int(digits, 16), 4 * len(digits), 16
        if prefix == "0b" and digits and set(digits) <= {"0", "1"}:
            return int(digits, 2), len(digits), 2
    raise ValueError(f"{literal!r} is not {_LITERAL_FORMATS}")


class Bitmask(FLYNCBaseModel):
    """
    Bit pattern together with a mask selecting the bits that are significant.

    A candidate value matches when ``(candidate & mask) == data``.

    Parameters
    ----------
    data : int
        Expected bit pattern, written as a quoted hexadecimal (``"0x0800"``) or binary
        (``"0b0000 1000 0000 0000"``) literal. Whitespace inside the literal groups nibbles
        or bytes and is ignored. A plain integer is accepted as well and is widened to whole
        bytes. Both the bit width of the literal (leading zeros included) and its notation are
        kept and written back on dump, so the pattern survives a load/dump cycle unchanged.
        Must be greater or equal to 0.

    mask : int, optional
        Bits that are significant, written like ``data`` and describing the same number of
        bits. Defaults to all bits of ``data``; that default is not written back on dump.
        Must be greater or equal to 1.
    """

    data: int = Field(ge=0)
    mask: Optional[int] = Field(default=None, ge=1)
    _width: int = 8
    _base: int = 16

    @model_validator(mode="wrap")
    @classmethod
    def _parse_data_and_mask(cls, value, handler) -> Self:
        """Parse the ``data``/``mask`` literals into ints, keeping the bit width and notation they were written in."""

        if not isinstance(value, dict):
            return handler(value)
        fields, parsed = dict(value), {}
        for name in ("data", "mask"):
            if fields.get(name) is None:
                continue
            try:
                fields[name], width, base = _parse_literal(fields[name])
            except ValueError as error:
                raise err_minor(
                    f"{name} must be {_LITERAL_FORMATS}; got {value[name]!r}",
                    category=Category.VALUE_RANGE,
                    error_number="177",
                ) from error
            parsed[name] = (width, base)
        if len(parsed) == 2 and parsed["data"][0] != parsed["mask"][0]:
            raise err_minor("'data' and 'mask' must describe the same number of bits", category=Category.VALUE_RANGE, error_number="178")
        instance = handler(fields)
        width, base = parsed["data"]
        object.__setattr__(instance, "_width", width)
        object.__setattr__(instance, "_base", base)
        return instance

    @model_validator(mode="after")
    def validate_mask(self) -> Self:
        """Default the mask to all bits of ``data``, and reject ``data`` bits the mask ignores."""

        mask = self.mask if self.mask is not None else (1 << self._width) - 1
        if self.mask is None:
            object.__setattr__(self, "mask", mask)
        if self.data & ~mask:
            raise err_minor("'data' has bits set outside 'mask'", category=Category.STRUCTURAL, error_number="234")
        return self

    @field_serializer("data", "mask")
    def serialize_literal(self, value: Optional[int]) -> Optional[str]:
        """Dump as a literal of the original width and notation, so ``bits`` and byte lengths survive a load/dump cycle."""

        if value is None:
            return None
        return f"0x{value:0{self._width // 4}X}" if self._base == 16 else f"0b{value:0{self._width}b}"

    @property
    def bits(self) -> str:
        """Binary view of ``data``, zero-padded to its bit width."""

        return f"{self.data:0{self._width}b}"
