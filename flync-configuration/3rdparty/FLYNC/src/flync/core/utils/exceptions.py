"""Defines custom pydantic errors and the FLYNC error-id scheme."""

import importlib
import sys
from contextvars import ContextVar
from enum import IntEnum, StrEnum
from typing import List, Optional

from pydantic_core import ErrorDetails, PydanticCustomError

PROJECT_KEY = "FLYNC"

# Accumulates informational warnings during a validate_with_policy call.
# Validators append to this list via warn() instead of raising, so the
# validated field is kept while the message still surfaces as a warning.
_validation_warnings: ContextVar[Optional[List[ErrorDetails]]] = ContextVar("_validation_warnings", default=None)


class Severity(StrEnum):
    "String enum that represents errors severity levels"

    WARN = "WARN"
    MIN = "MIN"
    MAJ = "MAJ"
    FAT = "FAT"


class Category(IntEnum):
    "Integer enum to represent categories for errors"

    VALUE_RANGE = 1
    REQUIRED = 2
    CONSISTENCY = 3
    UNIQUENESS = 4
    REFERENCE = 5
    FORMAT = 6
    COMPATIBILITY = 7
    STRUCTURAL = 8
    LIFECYCLE = 9


# Short category codes used in the error ID; the catalog still shows the full name.
CATEGORY_CODE: dict[Category, str] = {
    Category.VALUE_RANGE: "VAL",
    Category.REQUIRED: "REQ",
    Category.CONSISTENCY: "CONS",
    Category.UNIQUENESS: "UNIQ",
    Category.REFERENCE: "REF",
    Category.FORMAT: "FMT",
    Category.COMPATIBILITY: "COMP",
    Category.STRUCTURAL: "STRUCT",
    Category.LIFECYCLE: "LIFE",
}


def module_code_for(dotted_module: str) -> str:
    """Resolve a module code from the ``KEY`` declared in the nearest enclosing package.

    Walks the package chain (``flync.model.flync_4_ecu.port`` -> ``flync_4_ecu`` -> ``ECU``).
    Falls back to ``GEN`` for the top-level model and version migrators, ``CMN`` otherwise.
    """

    parts = dotted_module.split(".")
    for end in range(len(parts), 0, -1):
        name = ".".join(parts[:end])
        module = sys.modules.get(name)
        if module is None:
            try:
                module = importlib.import_module(name)
            except Exception:
                continue
        key = getattr(module, "KEY", None)
        if isinstance(key, str):
            return key
    if dotted_module.endswith("flync.model.flync_model") or "version_migrators" in parts:
        return "GEN"
    return "CMN"


def compose_error_id(severity: Severity, module_code: str, category: Category | None, error_number: str | None) -> str:
    """Assemble ``FLYNC-<MODULE>-<SEVERITY>-<CATEGORY>-<NUMBER>``; unset category/number render as ``UNC``/``000``."""

    category_code = CATEGORY_CODE[category] if category is not None else "UNC"
    number = error_number if error_number is not None else "000"
    return f"{PROJECT_KEY}-{module_code}-{severity.value}-{category_code}-{number}"


def _caller_error_id(severity: Severity, category: Category | None, error_number: str | None) -> str:
    """Wrapper function to resolve module from the callstack and return error ID."""

    # Frame 0 is this helper, frame 1 the factory, frame 2 the calling validator.
    caller_module = sys._getframe(2).f_globals.get("__name__", "")
    return compose_error_id(severity, module_code_for(caller_module), category, error_number)


def warn(msg: str, *, category: Category | None = None, error_number: str | None = None, **ctx) -> None:
    """
    Record a validation warning without raising a validation error.

    The message is appended to the active warning list (set up by ``validate_with_policy``) and will be returned alongside
    ``load_errors`` so that it appears in the warnings table.  If called outside a ``validate_with_policy`` context the call is silently ignored.

    Parameters
    ----------
    msg : str
        Human-readable warning message.

    category : Category, optional
        Semantic category used to compose the error id.

    error_number : str, optional
        Globally unique zero-padded number identifying this call site.

    ctx : dict
        Context arguments that define key-value pairs to fill the placeholders in `msg`.
    """

    warnings_list = _validation_warnings.get()
    if warnings_list is None:
        return
    ctx["error_id"] = _caller_error_id(Severity.WARN, category, error_number)
    warnings_list.append(
        {
            "type": "warning",
            "msg": msg,
            "loc": (),
            "ctx": ctx,
            "input": None,
            "url": "",
        }
    )


def warn_from_error(error: PydanticCustomError) -> None:
    """
    Record a validation warning from an already-raised ``PydanticCustomError``, preserving its original
    error id and message instead of composing a new one at the catch site.

    Use this when downgrading a caught error to a warning (e.g. a best-effort ``bind()`` call whose
    failures should not be fatal) so the warning still reports the id of the validator that actually
    raised it, rather than the id of the catch site.

    Parameters
    ----------
    error : PydanticCustomError
        The error caught from a factory call (``err_minor``/``err_major``/``err_fatal``).
    """

    warnings_list = _validation_warnings.get()
    if warnings_list is None:
        return
    warnings_list.append(
        {
            "type": "warning",
            "msg": error.message(),
            "loc": (),
            "ctx": dict(error.context or {}),
            "input": None,
            "url": "",
        }
    )


def err_minor(msg: str, *, category: Category | None = None, error_number: str | None = None, **ctx) -> PydanticCustomError:
    """
    Factory that returns PydanticCustomError with type **minor**.

    Parameters
    ----------
    msg : str
        Error message that may contain placeholders

    category : Category, optional
        Semantic category used to compose the error id.

    error_number : str, optional
        Globally unique zero-padded number identifying this call site.

    ctx : dict
        Context arguments that define key-value pairs to fill the placeholders in `msg`

    Returns
    -------
    PydanticCustomError
    """

    ctx["error_id"] = _caller_error_id(Severity.MIN, category, error_number)
    return PydanticCustomError("minor", msg, ctx)


def err_major(msg: str, *, category: Category | None = None, error_number: str | None = None, **ctx) -> PydanticCustomError:
    """
    Factory that returns PydanticCustomError with type **major**.

    Parameters
    ----------
    msg : str
        Error message that may contain placeholders

    category : Category, optional
        Semantic category used to compose the error id.

    error_number : str, optional
        Globally unique zero-padded number identifying this call site.

    ctx : dict
        Context arguments that define key-value pairs to fill the placeholders in `msg`

    Returns
    -------
    PydanticCustomError
    """

    ctx["error_id"] = _caller_error_id(Severity.MAJ, category, error_number)
    return PydanticCustomError("major", msg, ctx)


def err_fatal(msg: str, *, category: Category | None = None, error_number: str | None = None, **ctx) -> PydanticCustomError:
    """
    Factory that returns PydanticCustomError with type **fatal**.

    Parameters
    ----------
    msg : str
        Error message that may contain placeholders

    category : Category, optional
        Semantic category used to compose the error id.

    error_number : str, optional
        Globally unique zero-padded number identifying this call site.

    Returns
    -------
    PydanticCustomError
    """

    ctx["error_id"] = _caller_error_id(Severity.FAT, category, error_number)
    return PydanticCustomError("fatal", msg, ctx)
