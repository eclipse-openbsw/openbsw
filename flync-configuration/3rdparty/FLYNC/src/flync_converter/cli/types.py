"""Click parameter types and annotation helpers for the flync_converter CLI."""

import enum
from typing import Type, get_args

import click


class _EnumNameType(click.ParamType):
    """Click param type that accepts an enum member by name."""

    def __init__(self, enum_cls: Type[enum.Enum]) -> None:
        self.enum_cls = enum_cls
        self.name = enum_cls.__name__

    def get_metavar(self, param, ctx=None) -> str:
        """Return the metavar string listing valid enum member names."""
        return "[" + "|".join(m.name for m in self.enum_cls) + "]"

    def convert(self, value, param, ctx):
        """Convert a string value to the corresponding enum member, failing with choices on error."""
        if isinstance(value, self.enum_cls):
            return value
        try:
            return self.enum_cls[value]
        except KeyError:
            choices = [m.name for m in self.enum_cls]
            self.fail(
                f"{value!r} is not one of {choices}",
                param,
                ctx,
            )


def _annotation_to_click_type(annotation):  # NOSONAR
    """Map a pydantic field annotation to a Click param type."""
    origin = getattr(annotation, "__origin__", None)
    if origin is not None:
        # Unwrap Optional / Union — take first non-None arg
        inner_args = [a for a in get_args(annotation) if a is not type(None)]
        annotation = inner_args[0] if inner_args else str
    if isinstance(annotation, type):
        if issubclass(annotation, bool):
            return click.BOOL
        if issubclass(annotation, int):
            return click.INT
        if issubclass(annotation, float):
            return click.FLOAT
        if issubclass(annotation, enum.Enum):
            return _EnumNameType(annotation)
    return click.STRING
