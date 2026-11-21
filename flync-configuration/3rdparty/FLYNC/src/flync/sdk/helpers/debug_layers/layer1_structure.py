"""Layer 1: Folder and file structure validation.

Traverses the FLYNCModel's External annotations to build the expected filesystem tree,
then diffs it against the actual directory — reporting missing required entries,
files/folders found in the wrong place, and likely typos.
"""

from __future__ import annotations

import difflib
from dataclasses import dataclass
from pathlib import Path
from typing import Optional

from pydantic import BaseModel
from pydantic.fields import FieldInfo

from flync.core.annotations.external import External, OutputStrategy
from flync.sdk.helpers.debug import (
    FLYNC_EXT,
    _display_name,
    _has_external_fields,
    _is_optional,
    _is_pydantic_model,
    _unwrap_type,
)
from flync.sdk.utils.field_utils import get_metadata


@dataclass
class StructureIssue(object):
    """A single folder/file structure issue found in Layer 1 checks."""

    severity: str  # "error" | "warning"
    message: str
    hint: str = ""
    path: str = ""


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _get_ext_fields(model_cls: type[BaseModel]) -> list[tuple[str, FieldInfo, External]]:
    """Return only the External-annotated fields of model_cls."""
    result: list[tuple[str, FieldInfo, External]] = []
    for name, info in model_cls.model_fields.items():
        ext = get_metadata(info.metadata, External)
        if ext is not None:
            result.append((name, info, ext))
    return result


def _is_required(field_info: FieldInfo) -> bool:
    """True when the field has no default/factory and the type is not Optional."""
    return field_info.is_required() and not _is_optional(field_info.annotation)


def _rel(path: Path, root: Path) -> str:
    """Return path relative to root, or path unchanged if it isn't under root."""
    try:
        return str(path.relative_to(root))
    except ValueError:
        return str(path)


# ---------------------------------------------------------------------------
# Global filename map – used for "wrong location" and typo detection
# ---------------------------------------------------------------------------


def _build_known_filenames(
    model_cls: type[BaseModel],
) -> dict[str, list[str]]:
    """
    Walk the whole model tree once and return a map of every filename/dirname
    that the model expects anywhere, with a human-readable description of where
    it lives.  Used to detect misplaced files and suggest corrections.

    Returns: {filename → [location descriptions]}
    """
    result: dict[str, list[str]] = {}
    _collect_known_names(model_cls, "<root>", result, visited=set())
    return result


def _collect_known_names(
    model_cls: type[BaseModel],
    location_desc: str,
    result: dict[str, list[str]],
    visited: set,
) -> None:
    """Recursively walk model_cls, recording each expected name and where it lives."""
    if model_cls in visited:
        return
    visited.add(model_cls)

    for field_name, field_info, ext in _get_ext_fields(model_cls):
        is_file = OutputStrategy.SINGLE_FILE in ext.output_structure
        name = _display_name(field_name, ext, is_file)
        result.setdefault(name, []).append(location_desc)

        if not is_file:
            is_list, inner = _unwrap_type(field_info.annotation)
            if _is_pydantic_model(inner):
                child_desc = f"inside each <{field_name}_item>/" if is_list else f"inside {name}/"
                _collect_known_names(inner, child_desc, result, visited)


# ---------------------------------------------------------------------------
# Typo detection
# ---------------------------------------------------------------------------


def _typo_suggestion(name: str, candidates: list[str]) -> Optional[str]:
    """Return the closest candidate if within edit-distance threshold, else None."""
    bare = name.replace(FLYNC_EXT, "")
    bare_candidates = [c.replace(FLYNC_EXT, "") for c in candidates]
    matches = difflib.get_close_matches(bare, bare_candidates, n=1, cutoff=0.70)
    if matches:
        idx = bare_candidates.index(matches[0])
        return candidates[idx]
    return None


# ---------------------------------------------------------------------------
# Main check
# ---------------------------------------------------------------------------


def check_structure(
    model_cls: type[BaseModel],
    dir_path: Path,
    root_path: Path,
) -> list[StructureIssue]:
    """
    Validate the filesystem tree rooted at dir_path against the External
    annotations of model_cls.  Returns a list of StructureIssue objects.
    """
    issues: list[StructureIssue] = []
    known_names = _build_known_filenames(model_cls)
    _check_dir(model_cls, dir_path, root_path, known_names, issues)
    return issues


def _check_dir(
    model_cls: type[BaseModel],
    dir_path: Path,
    root_path: Path,
    known_names: dict[str, list[str]],
    issues: list[StructureIssue],
) -> None:
    """Recursively verify one directory against its expected model structure."""
    if not dir_path.exists():
        return

    expected = _build_expected_entries(model_cls)
    _check_expected_entries(expected, dir_path, root_path, known_names, issues)
    _check_unexpected_entries(dir_path, expected, known_names, root_path, issues)


def _build_expected_entries(
    model_cls: type[BaseModel],
) -> dict[str, tuple[bool, bool, FieldInfo, External]]:
    """Build the expected-names table: name -> (is_required, is_file, field_info, ext)."""
    expected: dict[str, tuple[bool, bool, FieldInfo, External]] = {}
    for field_name, field_info, ext in _get_ext_fields(model_cls):
        is_file = OutputStrategy.SINGLE_FILE in ext.output_structure
        name = _display_name(field_name, ext, is_file)
        expected[name] = (_is_required(field_info), is_file, field_info, ext)
    return expected


def _check_expected_entries(
    expected: dict[str, tuple[bool, bool, FieldInfo, External]],
    dir_path: Path,
    root_path: Path,
    known_names: dict[str, list[str]],
    issues: list[StructureIssue],
) -> None:
    """Report missing required entries and recurse into every folder that is present."""
    for name, (is_req, is_file, field_info, ext) in expected.items():
        target = dir_path / name
        if not target.exists():
            if is_req:
                issues.append(_missing_entry_issue(name, is_file, dir_path, root_path))
            continue

        if not is_file:
            _check_folder_field(name, field_info, target, root_path, known_names, issues)


def _missing_entry_issue(name: str, is_file: bool, dir_path: Path, root_path: Path) -> StructureIssue:
    """Build the StructureIssue for a required entry that doesn't exist on disk."""
    kind = "file" if is_file else "folder"
    return StructureIssue(
        severity="error",
        message=f"Required {kind} missing: '{name}'",
        path=_rel(dir_path, root_path),
        hint=f"Expected at: {_rel(dir_path / name, root_path)}",
    )


def _check_folder_field(
    name: str,
    field_info: FieldInfo,
    target: Path,
    root_path: Path,
    known_names: dict[str, list[str]],
    issues: list[StructureIssue],
) -> None:
    """Recurse into a folder-node field: per-list-item validation or a single nested model."""
    is_list, inner = _unwrap_type(field_info.annotation)
    if not _is_pydantic_model(inner):
        return
    if is_list:
        _check_list_field_items(name, inner, target, root_path, known_names, issues)
    else:
        _check_dir(inner, target, root_path, known_names, issues)


def _check_list_field_items(
    name: str,
    inner: type[BaseModel],
    target: Path,
    root_path: Path,
    known_names: dict[str, list[str]],
    issues: list[StructureIssue],
) -> None:
    """Validate each item found under a List[Model] field's folder."""
    item_has_substructure = _has_external_fields(inner)
    for child in sorted(target.iterdir()):
        if _should_skip(child):
            continue
        _check_list_item(name, inner, child, item_has_substructure, target, root_path, known_names, issues)


def _check_list_item(
    name: str,
    inner: type[BaseModel],
    child: Path,
    item_has_substructure: bool,
    target: Path,
    root_path: Path,
    known_names: dict[str, list[str]],
    issues: list[StructureIssue],
) -> None:
    """Validate a single item inside a List[Model] field's folder."""
    if item_has_substructure:
        # Each item is its own sub-folder (e.g. ecus/, controllers/)
        if child.is_dir():
            _check_dir(inner, child, root_path, known_names, issues)
        else:
            issues.append(
                StructureIssue(
                    severity="warning",
                    message=f"Unexpected file '{child.name}' directly inside '{_rel(target, root_path)}/'",
                    hint=f"'{name}/' expects one sub-folder per item, not individual files at this level.",
                    path=_rel(child, root_path),
                )
            )
    elif child.is_dir():
        # Each item is a flat .flync.yaml file (e.g. pdus/, sockets/); a sub-folder is unexpected
        issues.append(
            StructureIssue(
                severity="warning",
                message=f"Unexpected folder '{child.name}' inside '{_rel(target, root_path)}/'",
                hint=f"'{name}/' expects one .flync.yaml file per item, not sub-folders.",
                path=_rel(child, root_path),
            )
        )
    # individual .flync.yaml files here are correct — no issue


def _check_unexpected_entries(
    dir_path: Path,
    expected: dict[str, tuple[bool, bool, FieldInfo, External]],
    known_names: dict[str, list[str]],
    root_path: Path,
    issues: list[StructureIssue],
) -> None:
    """Report every entry in dir_path that isn't one of the expected names."""
    for actual in sorted(dir_path.iterdir()):
        if _should_skip(actual):
            continue
        if actual.name in expected:
            continue
        _report_unexpected(actual, expected, known_names, dir_path, root_path, issues)


def _should_skip(path: Path) -> bool:
    """Ignore hidden files, markdown docs, and non-FLYNC files."""
    name = path.name
    # Only care about .flync.yaml files and directories
    return name.startswith(".") or name.endswith(".md") or (path.is_file() and not name.endswith(FLYNC_EXT))


def _report_unexpected(
    actual: Path,
    expected: dict,
    known_names: dict[str, list[str]],
    dir_path: Path,
    root_path: Path,
    issues: list[StructureIssue],
) -> None:
    """Classify one unexpected filesystem entry: wrong location, typo, or truly unknown."""
    name = actual.name
    all_expected_names = list(expected.keys())

    # ── Known elsewhere: wrong location ─────────────────────────────────────
    if name in known_names:
        locations = known_names[name]
        issues.append(
            StructureIssue(
                severity="error",
                message=f"'{name}' is in the wrong location: '{_rel(dir_path, root_path)}'",
                hint=f"This belongs {', '.join(locations)}.",
                path=_rel(actual, root_path),
            )
        )
        return

    # ── Typo against expected names at this level ────────────────────────────
    suggestion = _typo_suggestion(name, all_expected_names)
    if suggestion:
        issues.append(
            StructureIssue(
                severity="warning",
                message=f"Unrecognised {'folder' if actual.is_dir() else 'file'}: '{name}'",
                hint=f"Did you mean '{suggestion}'? Also verify this is the right folder for it.",
                path=_rel(actual, root_path),
            )
        )
        return

    # ── Truly unknown ────────────────────────────────────────────────────────
    if actual.is_dir() or name.endswith(FLYNC_EXT):
        expected_list = ", ".join(f"'{n}'" for n in sorted(expected.keys())) or "none"
        issues.append(
            StructureIssue(
                severity="warning",
                message=f"Unrecognised {'folder' if actual.is_dir() else 'file'}: '{name}'",
                hint=f"Expected entries here: {expected_list}.",
                path=_rel(actual, root_path),
            )
        )
