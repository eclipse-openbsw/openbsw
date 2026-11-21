"""
Generic reusable validators for FLYNC models: sub-model and list removal
validators, plus helpers to turn ``None``/singletons into lists and to check
uniqueness and element membership.
"""

from typing import Any, Iterable, Optional

from pydantic import TypeAdapter, ValidationError, ValidationInfo

import flync.core.utils.base_utils as utils
from flync.core.utils.exceptions import (
    Category,
    _validation_warnings,
    err_major,
    err_minor,
)

_LOCATION_SYSTEM = "in system"


def _resolve_location(info: ValidationInfo) -> str:
    """
    Return a human-readable location string from validation context.
    """

    data = info.data if hasattr(info, "data") and info.data else {}
    parent_name = data.get("name")
    if parent_name:
        return f"in {parent_name}"
    if "vlan_id" in data:
        return f"for VLAN Id {data['vlan_id']}"
    return _LOCATION_SYSTEM


def validate_or_remove(label: str, field_type: Any, severity: str = "minor"):
    """
    Factory that returns a BeforeValidator for sub-model fields.

    Use inside ``Annotated`` to pre-validate a field before Pydantic processes it.
    If the raw data fails validation all sub-errors are packed into a single error.

    - ``"minor"`` severity: the field is removed and the parent model still loads without it.
        The message says "Removing {label}…".
    - ``"major"`` severity: the parent model will fail regardless (the field is required).
        The message reports the validation failure without implying graceful removal.

    The parent object's ``name`` field is included in the error message when available via ``info.data``.

    Parameters
    ----------
    label : str
        Human-readable field label used in the error message.
    field_type : Any
        Pydantic-compatible type to validate the data against.
    severity : str, optional
        Error severity — ``"minor"`` (default) or ``"major"``.

    Returns
    -------
    Callable
        A two-argument validator ``(data, info)`` ready for use with ``BeforeValidator``.
    """

    err_fn = err_major if severity == "major" else err_minor

    def _validator(data, info: ValidationInfo):
        """
        Validate ``data`` against ``field_type`` and raise on failure.

        Returns ``None`` unchanged.  On validation failure, packs all sub-errors into a single ``err_fn`` error whose message includes
        the parent object's name (read from ``info.data``) when available.
        """

        if data is None:
            return None
        try:
            TypeAdapter(field_type).validate_python(data)
        except ValidationError as ve:
            parent_name = info.data.get("name") if hasattr(info, "data") and info.data else None
            location = f"in {parent_name}" if parent_name else _LOCATION_SYSTEM
            sub_errors = _format_validation_error_sub_errors(ve)
            if severity == "major":
                raise err_fn(
                    f"Validation failed for {label} {location}.",
                    sub_errors=sub_errors,
                )
            raise err_fn(
                f"1 or more errors found while validating {label}. Removing {label} {location}.",
                sub_errors=sub_errors,
            )
        return data

    return _validator


def _format_validation_error_sub_errors(ve: ValidationError) -> str:
    """
    Flatten a :class:`ValidationError` into a "loc: msg" string, one line per sub-error.
    """

    return "\n".join(
        "{loc}: {msg}".format(
            loc=".".join(str(x) for x in e.get("loc", ())),
            msg=e.get("msg", ""),
        )
        for e in ve.errors()
    )


def _record_list_item_warning(label: str, location: str, field_name: str, idx: int, item: Any, sub_errors: str, severity: str) -> None:
    """
    Append a removed-list-item warning to the ``_validation_warnings`` context var, if one is active.
    """

    accumulated = _validation_warnings.get()
    if accumulated is None:
        return
    accumulated.append(
        {
            "type": severity,
            "msg": (f"1 or more errors found while validating {label}. Removing {label} {location}."),
            "loc": (field_name, idx),
            "input": item,
            "ctx": {"sub_errors": sub_errors},
            "url": "",
        }
    )


def validate_list_items_and_remove(label: str, item_type: Any, severity: str = "minor"):
    """
    Validate each item in a list individually, removing only invalid entries.

    Unlike :func:`validate_or_remove`, which discards the entire list when any item fails, this validator keeps valid items and removes only those
    that fail validation.  Per-item errors are forwarded to the ``_validation_warnings`` channel so they appear in the final error report even
    though the model continues building with the remaining valid items.

    Use inside ``Annotated`` as a ``BeforeValidator``.

    Parameters
    ----------
    label : str
        Human-readable field label used in error messages.
    item_type : Any
        Pydantic-compatible type for each individual list item.
    severity : str, optional
        Error severity for removed items — ``"minor"`` (default) or
        ``"major"``.

    Returns
    -------
    Callable
        A two-argument validator ``(data, info)`` ready for use with
        ``BeforeValidator``.
    """

    def _validator(data, info: ValidationInfo):
        """
        Validate each item in ``data`` individually, dropping invalid ones.

        Non-list values are returned unchanged.  For each item that fails validation an error is raised via ``err_fn``; valid items are
        collected and returned so the parent model loads with a partial list.  The parent object's name or VLAN ID is included in the message when
        available via ``info.data``.
        """

        if isinstance(data, dict):
            err_fn = err_major if severity == "major" else err_minor
            raise err_fn(
                f"'{label}' must be a list of items, but a single mapping was given. "
                f"Did you forget to add '- ' before each item to make it a list?"
            )
        if not isinstance(data, list):
            return data
        location = _resolve_location(info)
        field_name = getattr(info, "field_name", None) or label
        adapter = TypeAdapter(item_type)
        valid_items = []
        for idx, item in enumerate(data):
            try:
                adapter.validate_python(item)
                valid_items.append(item)
            except ValidationError as ve:
                sub_errors = _format_validation_error_sub_errors(ve)
                _record_list_item_warning(label, location, field_name, idx, item, sub_errors, severity)
        return valid_items

    return _validator


def validate_list_items_unique(input_list: list, list_label: Optional[str] = None) -> list:
    """
    Custom Validator for a list of items where every item should be unique.

    Args:
        input_list (list): List of items.

        list_label(str): Add an optional label to the error message.

    Raises:
        err_major: List contains duplicates.

    Returns:
        list: Input is handed over.
    """

    dupes = []
    if list_label:
        msg = f"Duplicates found in {list_label}:"
    else:
        msg = "Duplicates found:"

    if len(set(input_list)) != len(input_list):
        dupes = utils.get_duplicates_in_list(input_list)
        raise err_major(msg + str(dupes), category=Category.UNIQUENESS, error_number="009")
    return input_list


def validate_elements_in(subset: Iterable[Any], superset: Iterable[Any], msg: Optional[str] = None):
    """
    Custom Validator that checks if every element in `subset` appears at least once in `superset`.
    E.g. Validate if port_name is in switch_port_names.

    Args:
        subset (Iterable[Any]): Subset where elements are expected to be in superset.

        superset (Iterable[Any]): Reference set.

    Returns:
        Iterable[Any]: Return subset as received.
    """

    if msg:
        msg += " "
    if not all(elem in set(superset) for elem in subset):
        disallowed = set(subset) - set(superset)
        raise err_major(f"{msg}Invalid values: {sorted(disallowed)}.", category=Category.VALUE_RANGE, error_number="025")


def none_to_empty_list(v, info=None):
    """
    Make the field defined as optional [] if accidentally declared by the user as None.
    """

    if isinstance(v, dict):
        field = getattr(info, "field_name", None) or "list field"
        raise err_minor(
            f"'{field}' must be a list of items, but a single mapping was given. " "Did you forget to add '- ' before each item to make it a list?",
            error_number="185",
            category=Category.FORMAT,
        )
    return [] if v is None else v


def single_to_list(v):
    """
    Accept a single item where a list is expected and wrap it into a one-element list.

    Used for fields that started out as a single sub-model and later grew into a list, so
    that YAML written in the old single-mapping form keeps loading. ``None`` and existing
    list/tuple values are returned unchanged.
    """

    if v is None or isinstance(v, (list, tuple)):
        return v
    return [v]
