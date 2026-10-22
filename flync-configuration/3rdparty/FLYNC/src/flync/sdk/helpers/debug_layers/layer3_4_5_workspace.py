"""Layers 3, 4, and 5: Workspace-based validation.

Runs a single full workspace load and classifies the resulting errors into
three buckets based on their Pydantic error type:

  Layer 3 - Schema structure    : missing / extra / fatal fields
  Layer 4 - Field value errors   : minor / major field-value errors (incl. type mixups)
  Layer 5 - System-wide         : cross-model warnings (warn() calls)

The workspace already runs all validators; this module just classifies the
output so the runner can present it in layers.
"""

from __future__ import annotations

import difflib
import re
from dataclasses import dataclass
from dataclasses import field as dc_field
from pathlib import Path
from typing import Any, Optional, Tuple

# Sub-error patterns that indicate a structural (schema) problem
_EXTRA_RE = re.compile(r"^([^:\n]+):\s*Extra inputs are not permitted", re.MULTILINE)
_MISSING_RE = re.compile(r"^([^:\n]+):\s*Field required", re.MULTILINE)

# Sub-error pattern for list/dict type mixup raised by our validators
_LIST_TYPE_RE = re.compile(r"^([^:\n]+):\s*('.*?' must be a list of items[^\n]*)", re.MULTILINE)

# Native Pydantic error when a list field receives a non-list value (e.g. a dict).
# Two formats arise depending on context:
#   _NATIVE_LIST_TYPE_RE  — "list_type: Input should be a valid list"
#     Appears in _wrap_native_error sub_errors (workspace top-level, format: "type: msg").
#   _NATIVE_LIST_MSG_RE   — "<loc>: Input should be a valid list"
#     Appears in validate_or_remove sub_errors (format: "loc: msg", no type prefix).
_NATIVE_LIST_TYPE_RE = re.compile(r"list_type:\s*Input should be a valid list", re.IGNORECASE)
_NATIVE_LIST_MSG_RE = re.compile(r"^([^:\n]+):\s*(Input should be a valid list[^\n]*)", re.MULTILINE)

# Error types that indicate structural / schema problems (Layer 3)
_STRUCTURAL = {"missing", "extra_forbid", "extra_forbidden", "fatal"}

# Generic placeholder substituted with the real field name when info.field_name
# was unavailable (e.g. inside a BeforeValidator with no field context).
_LIST_FIELD_PLACEHOLDER = "'list field'"

# Field-level constraint errors raised by err_minor / err_major (Layer 4)
_CONSTRAINT = {"minor", "major"}

# Non-blocking system-wide warnings emitted via warn() (Layer 5)
_SYSTEM = {"warning"}


@dataclass
class WorkspaceIssue(object):
    """A single validation issue found in Layer 3, 4, or 5 workspace checks."""

    layer: int  # 3, 4, or 5
    severity: str  # "error" | "warning"
    message: str
    path: str = ""  # workspace-relative file path
    field: str = ""  # dot-separated field location within the file
    line: Optional[int] = None
    col: Optional[int] = None
    hint: str = ""  # optional "did you mean" suggestion
    err_type: str = ""  # original Pydantic error type
    loc_tuple: Tuple = dc_field(default_factory=tuple)  # raw Pydantic loc


def run_workspace_validation(
    dir_path: Path,
) -> tuple[Any, list[WorkspaceIssue]]:
    """
    Load the full workspace and return (DiagnosticsResult, [WorkspaceIssue]).

    Issues are classified into layers 3 / 4 / 5 by error type.
    Layer 3 errors are enriched with typo hints where possible.
    """
    from flync.sdk.helpers.validation_helpers import validate_workspace

    result = validate_workspace(dir_path)
    issues: list[WorkspaceIssue] = []

    for doc_uri, doc_errors in result.errors.items():
        rel_doc = _relativize(doc_uri, dir_path)
        for err in doc_errors:
            issues.extend(_classify_error(err, rel_doc, dir_path))

    # Enrich layer-3 errors with "did you mean" hints
    _enrich_typo_hints(issues, dir_path)
    return result, issues


def _relativize(doc_uri: str, dir_path: Path) -> str:
    """Return doc_uri relative to dir_path, or doc_uri unchanged if that's not possible."""
    try:
        return str(Path(doc_uri).relative_to(dir_path))
    except ValueError:
        return doc_uri


def _resolve_display_path(ctx: dict, rel_doc: str, dir_path: Path) -> str:
    """Prefer the yaml_path stamped in ctx (made relative), else fall back to rel_doc."""
    raw_yaml_path = ctx.get("yaml_path", "")
    if not raw_yaml_path:
        return rel_doc
    try:
        return str(Path(raw_yaml_path).relative_to(dir_path))
    except ValueError:
        return raw_yaml_path


def _classify_error(err: Any, rel_doc: str, dir_path: Path) -> list[WorkspaceIssue]:
    """Classify one Pydantic error dict into zero or more WorkspaceIssues (layer 3, 4, or 5)."""
    err_type = err.get("type", "")
    msg = err.get("msg", "") or ""
    ctx = err.get("ctx") or {}
    loc = err.get("loc", ())
    line = ctx.get("line")
    col = ctx.get("col")
    display_path = _resolve_display_path(ctx, rel_doc, dir_path)
    field_path = ".".join(str(p) for p in loc) if loc else ""

    if err_type in _STRUCTURAL:
        issues = [
            WorkspaceIssue(
                layer=3,
                severity="error",
                message=msg,
                path=display_path,
                field=field_path,
                line=line,
                col=col,
                err_type=err_type,
                loc_tuple=tuple(loc),
            )
        ]
    elif err_type in _CONSTRAINT:
        issues = _classify_constraint_error(err_type, msg, ctx, loc, display_path, field_path, line, col)
    elif err_type in _SYSTEM:
        issues = [
            WorkspaceIssue(
                layer=5,
                severity="warning",
                message=msg,
                path=display_path,
                field=field_path,
                line=line,
                col=col,
                err_type=err_type,
                loc_tuple=tuple(loc),
            )
        ]
    else:
        issues = []
    return issues


def _classify_constraint_error(
    err_type: str,
    msg: str,
    ctx: dict,
    loc: tuple,
    display_path: str,
    field_path: str,
    line: Optional[int],
    col: Optional[int],
) -> list[WorkspaceIssue]:
    """
    Classify a minor/major constraint error (Layer 4 by default).

    A BeforeValidator (e.g. validate_list_items_and_remove) may have already
    stripped a bad item and packed the original Pydantic errors into
    ctx["sub_errors"]. If those errors are structural (extra field / missing
    field) promote to Layer 3. List/dict type mixups are surfaced as specific
    Layer 4 issues.
    """
    sub_errors_str = ctx.get("sub_errors", "")
    loc_t = tuple(loc)

    promoted = _promote_constraint_to_layer3(sub_errors_str, loc_t, display_path, field_path, line, col)
    if promoted:
        issues = promoted
    else:
        list_type_issues = _extract_list_type_errors(sub_errors_str, loc_t, display_path, field_path, line, col)
        if list_type_issues:
            issues = list_type_issues
        elif _NATIVE_LIST_TYPE_RE.search(sub_errors_str):
            # Native Pydantic list_type error wrapped by _wrap_native_error:
            # field received a dict instead of a list, no BeforeValidator present.
            issues = [_native_list_type_issue(err_type, loc_t, display_path, field_path, line, col)]
        else:
            issues = [_generic_constraint_issue(err_type, msg, loc_t, display_path, field_path, line, col)]
    return issues


def _native_list_type_issue(
    err_type: str,
    loc: tuple,
    display_path: str,
    field_path: str,
    line: Optional[int],
    col: Optional[int],
) -> WorkspaceIssue:
    """Build the Layer 4 issue for a native Pydantic list_type error (a dict given instead of a list)."""
    field_name = str(loc[-1]) if loc else "field"
    message = f"'{field_name}' must be a list of items, but a mapping was given. Did you forget '- ' before each item to make it a list?"
    return WorkspaceIssue(
        layer=4,
        severity="error",
        message=message,
        path=display_path,
        field=field_path,
        line=line,
        col=col,
        err_type=err_type,
        loc_tuple=loc,
    )


def _generic_constraint_issue(
    err_type: str,
    msg: str,
    loc: tuple,
    display_path: str,
    field_path: str,
    line: Optional[int],
    col: Optional[int],
) -> WorkspaceIssue:
    """
    Build the Layer 4 issue for a generic minor/major constraint error.

    If the message still carries the 'list field' placeholder (info.field_name
    was None in the BeforeValidator context), substitute the real field name
    from the error loc so the user sees a precise field reference.
    """
    display_msg = msg
    if _LIST_FIELD_PLACEHOLDER in msg and loc:
        real_field = next((str(s) for s in reversed(loc) if not str(s).isdigit()), None)
        if real_field:
            display_msg = msg.replace(_LIST_FIELD_PLACEHOLDER, f"'{real_field}'")
    return WorkspaceIssue(
        layer=4,
        severity="error",
        message=display_msg,
        path=display_path,
        field=field_path,
        line=line,
        col=col,
        err_type=err_type,
        loc_tuple=loc,
    )


def _promote_constraint_to_layer3(
    sub_errors: str,
    loc: tuple,
    path: str,
    field: str,
    line: Optional[int],
    col: Optional[int],
) -> list[WorkspaceIssue]:
    """
    If sub_errors contain structural patterns (extra field / missing field),
    return Layer 3 WorkspaceIssues built from those patterns.

    The loc from the minor error points to the list field + item index
    (e.g. ("virtual_interfaces", 0)).  We append the bad field name as the
    last loc element so _enrich_typo_hints can navigate the model tree to
    generate a "did you mean" suggestion.
    """
    issues: list[WorkspaceIssue] = []
    extra_keys = [m.strip() for m in _EXTRA_RE.findall(sub_errors)]
    missing_keys = [m.strip() for m in _MISSING_RE.findall(sub_errors)]

    for key in extra_keys:
        field_name = key.split(".")[-1]  # strip any dotted prefix
        issues.append(
            WorkspaceIssue(
                layer=3,
                severity="error",
                message=f"Unexpected field '{field_name}'",
                path=path,
                field=f"{field}.{field_name}" if field else field_name,
                line=line,
                col=col,
                err_type="extra_forbid",
                # Append the bad key so _find_typo_in_model can use it
                loc_tuple=loc + (field_name,),
            )
        )

    for key in missing_keys:
        field_name = key.split(".")[-1]
        # If a paired extra key is a typo of this missing key we'll generate
        # the hint in _enrich_typo_hints; otherwise no hint is needed here.
        issues.append(
            WorkspaceIssue(
                layer=3,
                severity="error",
                message=f"Required field missing: '{field_name}'",
                path=path,
                field=f"{field}.{field_name}" if field else field_name,
                line=line,
                col=col,
                err_type="missing",
                loc_tuple=loc + (field_name,),
            )
        )

    return issues


def _extract_list_type_errors(
    sub_errors: str,
    loc: tuple,
    path: str,
    field: str,
    line: Optional[int],
    col: Optional[int],
) -> list[WorkspaceIssue]:
    """
    If sub_errors indicate that a list field received a dict, return specific Layer 4
    WorkspaceIssues with a clear user-facing message.

    Two sub_error formats are handled:
      • Custom validator format (_LIST_TYPE_RE):
          "<loc>: '...' must be a list of items ..."
          Raised by none_to_empty_list / validate_list_items_and_remove when they
          detect a dict.  The field name is extracted from the loc segment.

      • Native Pydantic format (_NATIVE_LIST_MSG_RE):
          "<loc>: Input should be a valid list"
          Emitted by validate_or_remove when TypeAdapter fails on a plain list
          field that has no custom BeforeValidator.  Converted to the same
          user-friendly message.

    The sub_error loc prefix is appended to the parent field path.
    """
    issues: list[WorkspaceIssue] = []

    def _make_issue(sub_loc_str: str, message: str) -> WorkspaceIssue:
        """Build a Layer 4 WorkspaceIssue for one matched sub_errors entry."""
        sub_field = f"{field}.{sub_loc_str}" if field and sub_loc_str else (field or sub_loc_str)
        return WorkspaceIssue(
            layer=4,
            severity="error",
            message=message,
            path=path,
            field=sub_field,
            line=line,
            col=col,
            err_type="minor",
            loc_tuple=loc,
        )

    def _field_from_loc(sub_loc_str: str) -> Optional[str]:
        """Return the last non-numeric segment of a dotted loc string, e.g. the field name."""
        return next(
            (seg for seg in reversed(sub_loc_str.split(".")) if not seg.isdigit()),
            None,
        )

    # Pattern A: our custom "must be a list of items" message
    for match in _LIST_TYPE_RE.finditer(sub_errors):
        sub_loc_str = match.group(1).strip()
        message = match.group(2).strip()
        actual_field = _field_from_loc(sub_loc_str)
        if actual_field and _LIST_FIELD_PLACEHOLDER in message:
            message = message.replace(_LIST_FIELD_PLACEHOLDER, f"'{actual_field}'")
        issues.append(_make_issue(sub_loc_str, message))

    # Pattern B: native Pydantic "Input should be a valid list" in validate_or_remove sub_errors.
    # Skip matches where the key is a Pydantic error type (e.g. "list_type") — those are handled
    # by the _NATIVE_LIST_TYPE_RE branch in the caller so the field name comes from loc[-1].
    if not issues:
        for match in _NATIVE_LIST_MSG_RE.finditer(sub_errors):
            sub_loc_str = match.group(1).strip()
            if sub_loc_str == "list_type":
                continue
            actual_field = _field_from_loc(sub_loc_str) or sub_loc_str
            message = (
                f"'{actual_field}' must be a list of items, but a single mapping was given. "
                f"Did you forget to add '- ' before each item to make it a list?"
            )
            issues.append(_make_issue(sub_loc_str, message))

    return issues


def _enrich_typo_hints(
    issues: list[WorkspaceIssue],
    dir_path: Path,
) -> None:
    """
    Add "did you mean X?" hints to Layer 3 errors caused by renamed fields.

    Two strategies:
    - missing      : open the YAML file and look for a similar key at the
                     parent level in the actual data (the mistyped key is
                     still present in the file as an "extra" field).
    - extra_forbid : traverse the Pydantic model tree to find a real field
                     name similar to the rejected key (the correct name is
                     absent from the file but exists in the schema).
    """
    yaml_cache: dict[str, Any] = {}
    model_cache: dict[str, list[type]] = {}

    for issue in issues:
        if issue.layer != 3 or not issue.loc_tuple:
            continue
        if issue.err_type == "missing":
            _enrich_missing_field_hint(issue, dir_path, yaml_cache)
        elif issue.err_type in ("extra_forbid", "extra_forbidden"):
            _enrich_extra_field_hint(issue, model_cache)


def _enrich_missing_field_hint(
    issue: WorkspaceIssue,
    dir_path: Path,
    yaml_cache: dict[str, Any],
) -> None:
    """Set issue.hint by looking for a similarly-named key in the actual YAML file."""
    if not issue.path:
        return
    abs_path = _resolve_yaml_path(dir_path, issue.path, issue.loc_tuple)
    if abs_path is None:
        return

    data = _load_yaml_cached(abs_path, yaml_cache)
    if data is None:
        return

    missing_field = str(issue.loc_tuple[-1])
    suggestion = _find_typo_in_yaml(data, issue.loc_tuple, missing_field)
    if suggestion:
        issue.hint = f"Found '{suggestion}' at this level -- did you mean '{missing_field}'?"


def _load_yaml_cached(abs_path: Path, cache: dict[str, Any]) -> Any:
    """Load and cache the YAML contents of abs_path, returning None on failure."""
    import yaml

    cache_key = str(abs_path)
    if cache_key not in cache:
        try:
            with open(abs_path, encoding="utf-8") as fh:
                cache[cache_key] = yaml.safe_load(fh)
        except Exception:
            cache[cache_key] = None
    return cache[cache_key]


def _enrich_extra_field_hint(issue: WorkspaceIssue, model_cache: dict[str, list[type]]) -> None:
    """Set issue.hint by finding a real model field name similar to the rejected key."""
    if "models" not in model_cache:
        from flync.model.flync_model import FLYNCModel

        model_cache["models"] = _collect_models(FLYNCModel)

    extra_key = str(issue.loc_tuple[-1])
    suggestion = _find_typo_in_model(model_cache["models"], issue.loc_tuple, extra_key)
    if suggestion:
        issue.hint = f"Did you mean '{suggestion}'?"


def _resolve_yaml_path(dir_path: Path, issue_path: str, loc_tuple: tuple) -> Optional[Path]:
    """
    Turn an issue's path into an absolute path to a readable YAML file.

    The workspace stamps the *directory* path as yaml_path for folder models.
    For OMMIT_ROOT sub-files the first loc element is the field/file name.
    """
    abs_path = dir_path / issue_path
    if abs_path.is_dir() and loc_tuple:
        candidate = abs_path / f"{loc_tuple[0]}.flync.yaml"
        return candidate if candidate.is_file() else None
    return abs_path if abs_path.is_file() else None


def _navigate(data: Any, path: tuple) -> Any:
    """Walk data using path, returning the object at the end or None on failure."""
    current = data
    for key in path:
        if isinstance(current, dict):
            current = current.get(key)
        elif isinstance(current, list) and isinstance(key, int):
            current = current[key] if 0 <= key < len(current) else None
        else:
            return None
        if current is None:
            return None
    return current


def _find_typo_in_yaml(data: Any, loc_tuple: tuple, missing_field: str) -> Optional[str]:
    """
    Look for a key in the YAML similar to missing_field.

    Tries progressively shorter prefixes of loc_tuple so that OMMIT_ROOT
    models (where the file content is the sub-model directly) are handled
    even when the first loc elements don't exist as YAML keys.
    """
    for skip in range(len(loc_tuple)):
        nav_path = loc_tuple[skip:-1]
        parent = _navigate(data, nav_path)
        if not isinstance(parent, dict):
            continue
        if missing_field in parent:
            continue
        candidates = [k for k in parent.keys() if k != missing_field]
        matches = difflib.get_close_matches(missing_field, candidates, n=1, cutoff=0.65)
        if matches:
            return matches[0]
    return None


def _collect_models(root_model: type) -> list[type]:
    """Collect all distinct Pydantic model types reachable from root_model."""
    from flync.sdk.helpers.debug import _is_pydantic_model, _unwrap_type

    result: list[type] = []
    visited: set[type] = set()

    def walk(cls: type) -> None:
        """Depth-first visit cls and every pydantic model reachable through its fields."""
        if cls in visited or not hasattr(cls, "model_fields"):
            return
        visited.add(cls)
        result.append(cls)
        for _fname, finfo in cls.model_fields.items():
            _, inner = _unwrap_type(finfo.annotation)
            if _is_pydantic_model(inner):
                walk(inner)

    walk(root_model)
    return result


def _find_typo_in_model(all_models: list[type], loc_tuple: tuple, extra_key: str) -> Optional[str]:
    """
    For an extra_forbid error, find a real model field name similar to extra_key.

    Navigates each candidate model in the tree using the non-integer path
    segments of loc_tuple[:-1].  The first model where navigation succeeds
    gives us the set of valid field names to compare against.
    """
    nav_keys = [k for k in loc_tuple[:-1] if not isinstance(k, int)]

    for start_model in all_models:
        current = _navigate_model_fields(start_model, nav_keys)
        if current is None or current is start_model or not hasattr(current, "model_fields"):
            continue
        field_names = list(current.model_fields.keys())
        matches = difflib.get_close_matches(extra_key, field_names, n=1, cutoff=0.65)
        if matches:
            return matches[0]
    return None


def _navigate_model_fields(start_model: type, nav_keys: list) -> Optional[type]:
    """Walk start_model's fields through nav_keys, returning the model class reached or None."""
    from flync.sdk.helpers.debug import _is_pydantic_model, _unwrap_type

    current: Optional[type] = start_model
    for key in nav_keys:
        if current is None or not hasattr(current, "model_fields") or key not in current.model_fields:
            return None
        finfo = current.model_fields[key]
        _, inner = _unwrap_type(finfo.annotation)
        current = inner if _is_pydantic_model(inner) else None
    return current
