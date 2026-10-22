"""Defines a duration datatype that accepts a plain millisecond count or a suffixed string."""

from typing import Annotated, Optional

from pydantic import BeforeValidator, Field


def parse_duration_ms(value: int | str) -> int:
    """
    Parse a duration into whole milliseconds.

    Accepts a plain integer (already milliseconds) or a string with an ``"ms"`` or ``"s"``
    suffix, e.g. ``"50ms"`` or ``"5s"``.
    """

    if isinstance(value, int):
        return value
    text = value.strip()
    if text.endswith("ms"):
        text, factor = text[:-2], 1
    elif text.endswith("s"):
        text, factor = text[:-1], 1000
    else:
        factor = 1
    return int(float(text) * factor)


def serialize_duration_ms(value: Optional[int]) -> Optional[str]:
    """
    Render a millisecond count back as an ``"ms"``-suffixed string, keeping ``None`` as ``None``.
    """

    return None if value is None else f"{value}ms"


#: A positive duration in whole milliseconds, also accepting ``"50ms"`` / ``"5s"`` in YAML.
DurationMs = Annotated[int, BeforeValidator(parse_duration_ms), Field(gt=0)]
