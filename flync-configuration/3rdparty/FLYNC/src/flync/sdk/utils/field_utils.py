"""
Field utility helpers for the FLYNC SDK.

Provides functions for extracting metadata and display names from Pydantic model fields.
"""

from typing import Iterable

from pydantic import BaseModel


def get_metadata[T](meta: Iterable[object], cls: type[T]) -> T | None:
    """
    Return the first metadata object of the specified type.

    Args:
        meta: An iterable of metadata objects.
        cls: The class type to search for.

    Returns:
        An instance of `cls` if found; otherwise, None.
    """

    for m in meta:
        if isinstance(m, cls):
            return m
    return None


def get_name[T](named_object: T, attr_name: str, fallback_name: str | None = None) -> str:
    """
    Retrieve a display name for an object.

    Looks up ``attr_name`` on ``named_object``. Falls back to
    ``fallback_name`` if the attribute is absent or falsy, and finally to
    the class name if that is also absent.

    Args:
        named_object: The object whose name should be retrieved.
        attr_name: The attribute name to look up on the object.
        fallback_name: Optional fallback value when the attribute is missing.

    Returns:
        The resolved display name string.
    """

    attr_name = attr_name or "name"
    return getattr(named_object, attr_name, fallback_name) or type(named_object).__name__


def get_field_name_from_alias(model: type[BaseModel], alias: str):
    """
    Resolve a field name from its alias in a Pydantic model.

    Args:
        model (type[BaseModel]): The Pydantic model class to inspect.
        alias (str): The alias of the field to look up.

    Returns:
        str: The actual field name corresponding to the given alias.
            If no field with the alias exists, the alias itself is returned.
    """

    for name, field in model.model_fields.items():
        if field.alias == alias:
            return name
    return alias
