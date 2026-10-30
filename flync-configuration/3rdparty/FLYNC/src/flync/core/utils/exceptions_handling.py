"""Provides utilities for validating models and handling errors."""

from typing import Any, List, Optional, Set, Tuple, Type, get_args, get_origin

from pydantic import BaseModel, TypeAdapter, ValidationError
from pydantic_core import ErrorDetails, InitErrorDetails, PydanticCustomError

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions import _validation_warnings

FATAL_ERROR_TYPES = {"extra_forbid", "extra_forbidden", "fatal", "missing"}

# All error types that originate from FLYNC's own error factories or Pydantic's structural checks.
# Any error type NOT in this set is a native Pydantic constraint error (e.g. less_than_equal, int_type) that will be re-wrapped as
# a major error so it follows the standard FLYNC display format.
FLYNC_ERROR_TYPES = FATAL_ERROR_TYPES | {"minor", "major", "warning"}


def is_semantic_validation_error(err: ErrorDetails) -> bool:
    """
    Return ``True`` if ``err`` is a user-raised semantic validation error.

    User code raises semantic errors via :func:`err_major`, :func:`err_minor`,
    or :func:`err_fatal`; their ``type`` is exactly ``"major"`` / ``"minor"`` /
    ``"fatal"`` and ``ctx`` carries the formatter kwargs only.  A native
    Pydantic mismatch rewrapped by :func:`_wrap_native_error` shares the
    ``"major"`` type but always sets ``ctx["sub_errors"]`` — that key
    distinguishes structural mismatches from semantic failures.
    """

    if err.get("type") not in {"major", "minor", "fatal"}:
        return False
    ctx = err.get("ctx") or {}
    return "sub_errors" not in ctx


def resolve_alias(model: type[BaseModel], field_name: str) -> str:
    """
    Return the YAML key used for a Pydantic field, considering alias.
    """

    if not model or not hasattr(model, "model_fields"):
        return field_name
    field = model.model_fields.get(field_name)
    return field.alias or field_name if field else field_name


def get_name_by_alias(model: type[BaseModel], alias: str):
    """
    Return the Python field name that corresponds to the given alias.

    Parameters
    ----------
    model : type[BaseModel]
        The Pydantic model class to search.
    alias : str
        The alias to look up.

    Returns
    -------
    str
        The Python attribute name whose alias matches ``alias``.

    Raises
    ------
    KeyError
        If no field with the given alias is found.
    """

    for field_name, field in model.model_fields.items():
        if field.alias == alias:
            return field_name
    raise KeyError(alias)


def _child_at(container: Any, key: Any) -> Tuple[bool, Any]:
    """
    Return ``(True, child)`` for ``container[key]``, or ``(False, None)`` when the key/index is missing or the container is not subscriptable.
    """

    try:
        return True, container[key]
    except (KeyError, IndexError, TypeError):
        return False, None


def _descend_model(current_model: Any, part: str) -> Any:
    """
    Return the Pydantic model class describing field ``part`` of ``current_model``, or ``None`` when it cannot be resolved.

    ``current_model`` is either a model class (look ``part`` up in ``model_fields``) or an already-resolved container generic such as
    ``dict[str, Model]`` (use it directly as the annotation).
    """

    if hasattr(current_model, "model_fields"):
        field = current_model.model_fields.get(part)
        annotation = field.annotation if field else None
    else:
        # current_model is already a container generic (e.g. dict[str, Model])
        annotation = current_model

    if annotation is None:
        return None

    origin = get_origin(annotation)
    args: tuple[Any, ...] = get_args(annotation)
    if origin is dict:
        nested = args[1] if len(args) > 1 else None
    elif origin is None:
        nested = annotation
    else:
        nested = None

    return nested if hasattr(nested, "model_fields") else None


def safe_yaml_position(node: Any, loc: tuple, model: type[BaseModel] | None = None) -> Tuple[int | None, int | None]:
    """
    Given a ruamel.yaml node and a Pydantic `loc` tuple, return (line, column).
    Falls back gracefully if key/item is missing.
    """

    current = node
    current_model = model
    parent = None
    last_key = None

    for part in loc:
        parent = current
        last_key = part

        if isinstance(part, int):
            # Handle list indices — an index never carries model information.
            found, current = _child_at(current, part)
            current_model = None
        else:
            # Map field name to YAML key if alias exists
            yaml_key = resolve_alias(current_model, part) if current_model else part
            found, current = _child_at(current, yaml_key)
            if current_model:
                current_model = _descend_model(current_model, part)

        if not found:
            return _fallback_position(parent)

    # Get line/column for final key or index
    return _extract_position(parent, last_key)


def _extract_position(parent: Any, key: Any) -> Tuple[int | None, int | None]:
    """
    Safely extract line/col from ruamel.yaml node.  Returns (line, column) or (None, None)
    """

    try:
        line = parent.lc.line
        col = parent.lc.col
        if isinstance(key, int):
            line, col = getattr(parent.lc, "item", lambda k: None)(key)  # type: ignore[misc]
        else:
            line, col = getattr(parent.lc, "value", lambda k: None)(key)  # type: ignore[misc]
    except AttributeError:
        return None, None

    return (
        line + 1 if line is not None else None,
        col + 1 if col is not None else None,
    )


def _fallback_position(node: Any) -> Tuple[int | None, int | None]:
    """
    Return the best-effort parent position if key/item is missing.
    """

    try:
        line = getattr(node.lc, "line", None)
        col = getattr(node.lc, "col", None)
    except AttributeError:
        return None, None

    return (
        line + 1 if line is not None else None,
        col + 1 if col is not None else None,
    )


def _parse_first_sub_error_loc(sub_errors: str) -> tuple:
    """Extract the loc path from the first line of a sub_errors string.

    sub_errors lines have the format ``"dot.path: message"``.  The path is
    split on ``.`` and integer-looking parts are converted to ``int`` so the
    result can be appended to a Pydantic loc tuple for YAML navigation.
    """
    first_line = sub_errors.split("\n")[0]
    colon_idx = first_line.find(": ")
    if colon_idx <= 0:
        return ()
    parts: list[int | str] = []
    for part in first_line[:colon_idx].split("."):
        if part:
            try:
                parts.append(int(part))
            except ValueError:
                parts.append(part)
    return tuple(parts)


def _error_yaml_position(err: ErrorDetails, model: type[BaseModel], yaml_data: object, ctx: dict) -> Tuple[int | None, int | None]:
    """
    Return the YAML ``(line, column)`` for ``err``.

    Starts from the position of the error's own ``loc`` and, when the error carries ``sub_errors``, refines it with the position of the first
    sub-error so the report points at the actual offending line inside the sub-object.
    """

    line, col = safe_yaml_position(yaml_data, err["loc"], model=model)

    sub_errors = ctx.get("sub_errors", "")
    if not sub_errors:
        return line, col

    sub_loc = _parse_first_sub_error_loc(sub_errors)
    if not sub_loc:
        return line, col

    deep_line, deep_col = safe_yaml_position(yaml_data, err["loc"] + sub_loc)
    return (deep_line, deep_col) if deep_line is not None else (line, col)


def _enrich_error_ctx(
    err: ErrorDetails,
    model: Optional[type[BaseModel]],
    yaml_data: Optional[object],
    yaml_path: Optional[str],
) -> dict:
    """
    Return ``err``'s context enriched (in place) with ``yaml_path`` and the YAML ``line``/``col`` of the error, when they can be determined.
    """

    ctx = err.get("ctx", {})

    if yaml_path and "yaml_path" not in ctx:
        ctx["yaml_path"] = str(yaml_path)

    if model is not None and yaml_data and "yaml_location" not in ctx:
        line, col = _error_yaml_position(err, model, yaml_data, ctx)
        if line:
            ctx["line"] = line
        if col:
            ctx["col"] = col

    return ctx


def errors_to_init_errors(
    errors: List[ErrorDetails],
    model: Optional[type[BaseModel]] = None,
    yaml_data: Optional[object] = None,
    yaml_path: Optional[str] = None,
) -> List[InitErrorDetails]:
    """
    Convert Pydantic validation errors into ``InitErrorDetails`` for re-raising.

    Optionally enriches each error with YAML source location information when ``model`` and ``yaml_data`` are provided, and with the file path when
    ``yaml_path`` is provided.

    Parameters
    ----------
    errors : List[ErrorDetails]
        The list of errors to convert.
    model : type[BaseModel], optional
        The Pydantic model class used to resolve field aliases for YAML position look-ups.
    yaml_data : object, optional
        The parsed ruamel.yaml AST of the document, used together with ``model`` to locate the error position within the file.
    yaml_path : str, optional
        The workspace-relative file path to embed in each error's context as ``yaml_path``.

    Returns
    -------
    List[InitErrorDetails]
        The converted errors, ready to be passed to ``ValidationError.from_exception_data``.
    """

    enriched = []
    for e in errors:
        ctx = _enrich_error_ctx(e, model, yaml_data, yaml_path)
        error_detail = InitErrorDetails(
            type=PydanticCustomError(e.get("type", ""), e.get("msg", ""), ctx),
            loc=e.get("loc", ()),
            input=e.get("input"),
            ctx=ctx,
        )
        error_detail["metadata"] = ctx  # type: ignore[typeddict-unknown-key]
        enriched.append(error_detail)
    return enriched


def delete_at_loc(data: Any, loc: Tuple):
    """
    Helper function to remove the key/item from original object by loc(path to an element within the object).

    Parameters
    ----------
    data : Any
        Data to remove the item from. **Will be mutated**.

    loc : Tuple
        Path to the location of item to remove.
    """

    if not loc:
        return

    cur = data

    for key in loc[:-1]:
        if isinstance(cur, dict) and key in cur:
            cur = cur[key]
        elif isinstance(cur, list) and isinstance(key, int) and 0 <= key < len(cur):
            cur = cur[key]
        else:
            return

    last = loc[-1]
    if isinstance(cur, dict) and last in cur:
        del cur[last]
    elif isinstance(cur, list) and isinstance(last, int) and 0 <= last < len(cur):
        cur.pop(last)


def _get_error_signature(error_details: ErrorDetails) -> Tuple:
    """
    A function to return hashable representation of ErrorDetails object or dict taken from ValidationError.errors() list to then use it for
    identification.

    Parameters
    ----------
    error_details: ErrorDetails
        The object with pydantic's error details

    Returns
    -------
    Tuple
    """

    loc: Tuple[int | str, ...] = error_details.get("loc", ())
    msg = error_details.get("msg")
    error_type = error_details.get("type")

    return loc, str(msg), str(error_type)


def get_unique_errors(
    errors: List[ErrorDetails],
) -> List[ErrorDetails]:
    """
    A function to get the list of unique errors.

    Parameters
    ----------
    errors: List[ErrorDetails]
        The list of pydantic's error details

    Returns
    -------
    List[ErrorDetails]
    """

    errors_seen: Set[Tuple[str, Tuple]] = set()
    unique_errors: List[ErrorDetails] = []

    for error in errors:
        error_signature = _get_error_signature(error)

        if error_signature not in errors_seen:
            errors_seen.add(error_signature)
            unique_errors.append(error)

    return unique_errors


def _wrap_native_error(err: ErrorDetails) -> ErrorDetails:
    """
    Re-wrap a native Pydantic constraint error as a FLYNC major error.

    Native Pydantic errors (e.g. ``less_than_equal``, ``int_type``) do not carry ``sub_errors`` and appear with their raw Pydantic type in the error
    table.  This wraps them as ``major`` so they follow the standard format:
    the original type and message appear in the Details column and the message mirrors the ``validate_or_remove`` pattern.

    Cascade behavior is unchanged — the original error type is still used to decide whether to populate ``major_removed_locs``.

    Parameters
    ----------
    err : ErrorDetails
        A Pydantic error dict whose ``type`` is not in :data:`FLYNC_ERROR_TYPES`.

    Returns
    -------
    ErrorDetails
        A new error dict with ``type="major"``, a human-readable message, and the original error packed into ``ctx["sub_errors"]``.
    """

    original_type = err.get("type", "")
    original_msg = err.get("msg", "")
    loc = err.get("loc", ())
    field_name = loc[-1] if loc else "field"
    sub_errors = f"{original_type}: {original_msg}"
    original_ctx = err.get("ctx") or {}
    ctx: dict = {"sub_errors": sub_errors}
    for key in ("yaml_path", "line", "col"):
        if key in original_ctx:
            ctx[key] = original_ctx[key]
    return {
        "type": "major",
        "msg": (f"1 or more errors found while validating {field_name}. Removing {field_name}."),
        "loc": loc,
        "input": err.get("input"),
        "ctx": ctx,
        "url": "",
    }  # type: ignore[return-value]


def _tag_warnings_with_path(warnings: list, path) -> None:
    """
    Stamp each warning that lacks a yaml_path with the given path.
    """

    if not path:
        return
    for w in warnings:
        w_ctx = w.get("ctx") or {}
        if "yaml_path" not in w_ctx:
            w_ctx["yaml_path"] = str(path)
            w["ctx"] = w_ctx


def _enrich_validation_error(
    ve: ValidationError,
    model: type,
    working: Any,
    path,
) -> ValidationError:
    """
    Return ``ve`` re-raised with YAML source locations injected.
    """

    try:
        enriched = errors_to_init_errors(
            get_unique_errors(ve.errors()),
            model=model,
            yaml_data=working,
            yaml_path=path,
        )
        raise ValidationError.from_exception_data(
            title=ve.title,
            line_errors=enriched,  # type: ignore[arg-type]
        )
    except ValidationError as ve_enriched:
        return ve_enriched


def _has_top_level_fatal(errs: List[ErrorDetails], removed_locs: Set[Tuple]) -> bool:
    """
    Return True when an unrecovered fatal error sits at depth ≤ 1.
    """

    return any(e.get("type") in FATAL_ERROR_TYPES and len(e.get("loc", ())) <= 1 and e.get("loc", ()) not in removed_locs for e in errs)


def _collect_original_error(
    err: ErrorDetails,
    remove_loc: Tuple,
    collected_errors: List[ErrorDetails],
    working: Any,
    removed_locs: Set[Tuple],
    major_removed_locs: Set[Tuple],
) -> None:
    """
    Record ``err`` and excise its location from ``working``.
    """

    err_to_collect = err if err.get("type") in FLYNC_ERROR_TYPES else _wrap_native_error(err)
    collected_errors.append(err_to_collect)
    delete_at_loc(working, remove_loc)
    removed_locs.add(remove_loc)
    if err.get("type") == "major":
        major_removed_locs.add(remove_loc)


def _process_error_list(
    errs: List[ErrorDetails],
    removed_locs: Set[Tuple],
    major_removed_locs: Set[Tuple],
    collected_errors: List[ErrorDetails],
    working: Any,
) -> bool:
    """
    Remove offending locations from ``working`` and collect errors.

    For each error:
    - fatal nested (depth > 1): remove the parent so the whole sub-object is dropped cleanly.
    - minor/major: remove the exact offending field.
    - cascade from an earlier removal: escalate to parent silently.
    - cascade from a major-removed field: stop the chain, do not escalate.

    Returns True if at least one location was removed (progress made).
    """

    made_progress = False
    for err in errs:
        loc = err.get("loc", ())
        is_fatal = err.get("type") in FATAL_ERROR_TYPES
        remove_loc = loc[:-1] if is_fatal and len(loc) > 1 else loc
        if remove_loc in removed_locs:
            continue
        is_cascade = loc in removed_locs
        if is_cascade:
            # Cascade from an earlier removal → escalate to parent silently,
            # unless it originates from a major-removed field.
            if not (is_fatal and loc in major_removed_locs):
                delete_at_loc(working, remove_loc)
                removed_locs.add(remove_loc)
                made_progress = True
        else:
            _collect_original_error(
                err,
                remove_loc,
                collected_errors,
                working,
                removed_locs,
                major_removed_locs,
            )
            made_progress = True
    return made_progress


def validate_with_policy(model: Type[FLYNCBaseModel], data: Any, path) -> Tuple[Optional[FLYNCBaseModel], List[ErrorDetails]]:
    """
    Helper function to perform model validation from the given data, collect errors with different severity and perform action based on severity.

    For minor/major errors the offending field is removed from the working data via :func:`delete_at_loc` and validation is retried, so that the
    model can still be constructed without the invalid field. The loop continues until either validation succeeds, a fatal error is encountered,
    or no further progress can be made (all error locations already removed).

    Parameters
    ----------
    model : Type[FLYNCBaseModel]
        Flync model class.

    data : Any
        Data to validate and instantiate the model with.

    Returns
    -------
    Tuple[Optional[FLYNCBaseModel], List]
        Tuple with optional model instance and list of errors.

    Raises
    ------
    ValidationError
    """

    working = data
    collected_errors: List[ErrorDetails] = []
    removed_locs: Set[Tuple] = set()
    major_removed_locs: Set[Tuple] = set()
    warnings_token = _validation_warnings.set([])
    try:
        while True:
            try:
                result = TypeAdapter(model).validate_python(working)
                accumulated = _validation_warnings.get() or []
                _tag_warnings_with_path(accumulated, path)
                return result, get_unique_errors(collected_errors + accumulated)
            except ValidationError as ve:
                ve_enriched = _enrich_validation_error(ve, model, working, path)
                errs = ve_enriched.errors()
                if _has_top_level_fatal(errs, removed_locs):
                    raise ve_enriched
                if not _process_error_list(
                    errs,
                    removed_locs,
                    major_removed_locs,
                    collected_errors,
                    working,
                ):
                    break
            except Exception as e:
                fatal_ctx = {"ex": e.with_traceback(None)}
                raise ValidationError.from_exception_data(
                    title="Unhandled exception",
                    line_errors=errors_to_init_errors(
                        get_unique_errors(collected_errors),
                        model=model,
                        yaml_data=working,
                        yaml_path=path,
                    )
                    + [
                        InitErrorDetails(
                            type=PydanticCustomError(
                                "fatal",
                                "unhandled exception caught: {ex}",
                                fatal_ctx,
                            ),
                            ctx=fatal_ctx,
                            input=model,
                        )
                    ],
                )
        accumulated = _validation_warnings.get() or []
        _tag_warnings_with_path(accumulated, path)
        return None, get_unique_errors(collected_errors + accumulated)
    finally:
        _validation_warnings.reset(warnings_token)


def has_validators(model_type: type[FLYNCBaseModel]) -> bool:
    """
    Check if a model class has @model_validator or @field_validator decorators.

    Args:
        model_type (type[FLYNCBaseModel]): The model class to inspect.

    Returns:
        bool: True if the model has any validators.
    """
    if hasattr(model_type, "__pydantic_decorators__"):
        decorators = model_type.__pydantic_decorators__
        if (hasattr(decorators, "field_validators") and decorators.field_validators) or (
            hasattr(decorators, "model_validators") and decorators.model_validators
        ):
            return True

    return False
