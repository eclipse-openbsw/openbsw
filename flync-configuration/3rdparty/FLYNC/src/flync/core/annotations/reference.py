"""Annotations to mark a model field as a reference to an already loaded FLYNC object."""

from dataclasses import dataclass
from enum import IntFlag
from typing import Optional


class ReferenceStrategy(IntFlag):
    """
    The strategy on how a concrete flync object will be referenced.
    """

    AUTO = 1
    PRIVATE_ATTR = AUTO


@dataclass(frozen=True)
class Reference:
    """
    Indicates this field is a reference to an already loaded/generated field.

    Args:
        source: Name of the private attribute on the model that holds the referenced Pydantic model object.
        source_key: Attribute name on the referenced model whose value is used as the reference field's content. Defaults to ``name``.
        reference_strategy: Controls how the reference is stored and resolved.
    """

    source: str
    source_key: str = "name"
    reference_strategy: ReferenceStrategy = ReferenceStrategy.PRIVATE_ATTR


def resolve_reference(model, field_name: str) -> Optional[object]:
    """
    Return the concrete object a field's ``Reference`` annotation points to.

    Looks up ``field_name`` (accepting either the Python name or its alias) on
    ``model``'s type, reads the field's :class:`Reference` metadata, and returns
    the value of the referenced private attr (``ref.source``). Returns ``None``
    when the model has no fields, the field does not exist, the field carries no
    ``Reference``, or the referenced attr is unset.
    """
    fields = getattr(type(model), "model_fields", None) or {}
    if field_name not in fields:
        field_name = next((n for n, i in fields.items() if i.alias == field_name), field_name)
    info = fields.get(field_name)
    if info is None:
        return None
    ref = next((m for m in info.metadata if isinstance(m, Reference)), None)
    if ref is None or ReferenceStrategy.PRIVATE_ATTR not in ref.reference_strategy:
        return None
    return getattr(model, ref.source, None)
