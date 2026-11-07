"""Visual directory structure debugger for FLYNC models."""

from __future__ import annotations

from pathlib import Path
from typing import Any, Union, get_args, get_origin

from pydantic import BaseModel
from pydantic.fields import FieldInfo

from flync.core.annotations.external import External, NamingStrategy, OutputStrategy
from flync.sdk.utils.field_utils import get_metadata

FLYNC_EXT = ".flync.yaml"

# Resolved at import time: src/flync/sdk/helpers/debug.py → 4 levels up = repo root, then /exports
_EXPORTS_DIR = Path(__file__).parents[4] / "exports"

# Box-drawing characters for the tree lines
_BRANCH = "├── "  # non-last sibling
_LAST = "└── "  # last sibling (closes the branch)
_PIPE = "│   "  # vertical continuation under a non-last sibling
_SPACE = "    "  # blank indent under the last sibling (no more pipe needed)


def _unwrap_type(tp: Any) -> tuple[bool, Any]:
    """
    Peel off type wrappers to reach the concrete model class.

    Handles three wrappers in any combination:
      - Annotated[X, ...]  →  recurse into X  (pydantic usually strips this already)
      - Optional[X]        →  recurse into X  (Optional is just Union[X, None])
      - list[X]            →  return (True, X) so callers know it's a collection

    Returns (is_list, core_type).
    """
    is_list = False
    while True:
        # Annotated[X, ...] carries extra metadata in __metadata__; the real type is arg 0
        if hasattr(tp, "__metadata__"):
            tp = get_args(tp)[0]
            continue
        origin = get_origin(tp)
        # Union[X, None] is how Optional[X] is stored at runtime
        if origin is Union:
            non_none = [a for a in get_args(tp) if a is not type(None)]
            if non_none:
                tp = non_none[0]
                continue
        # list[X] — signal to callers that we should render a placeholder item row.
        # Also unwrap X itself: items like Annotated[Union[StandardPDU, MultiplexedPDU], Field(...)]
        # need the same Annotated/Union stripping before _is_pydantic_model can recognise them.
        if origin is list:
            args = get_args(tp)
            if args:
                is_list = True
                tp = args[0]
                continue
        return is_list, tp


def _is_optional(tp: Any) -> bool:
    """True if the annotation allows None (i.e. the field can be absent on disk)."""
    origin = get_origin(tp)
    return origin is Union and type(None) in get_args(tp)


def _is_pydantic_model(tp: Any) -> bool:
    """True if tp is a concrete pydantic BaseModel subclass we can recurse into."""
    # TypeError guard: get_origin() can return non-type objects (e.g. generics)
    # that would blow up issubclass
    try:
        return isinstance(tp, type) and issubclass(tp, BaseModel)
    except TypeError:
        return False


def _has_external_fields(model_cls: type[BaseModel]) -> bool:
    """True if model_cls has at least one External-annotated field.

    Used to decide whether a list item placeholder is a folder (has sub-structure)
    or a plain file (all content inlined into one .yaml).
    """
    return any(get_metadata(info.metadata, External) is not None for info in model_cls.model_fields.values())


def _display_name(field_name: str, ext: External, is_file: bool) -> str:
    """
    Resolve the filesystem name for this field.

    NamingStrategy.FIXED_PATH + ext.path  →  use the hard-coded path (e.g. "someip")
    NamingStrategy.FIELD_NAME (default)   →  use the Python field name (e.g. "ecus")

    .flync.yaml is appended only for file nodes, not folder nodes.
    """
    base = ext.path if ext.naming_strategy == NamingStrategy.FIXED_PATH and ext.path else field_name
    return base + (FLYNC_EXT if is_file else "")


def _collect_ext_fields(model_cls: type[BaseModel]) -> list[tuple[str, FieldInfo, External]]:
    """Return the (name, info, ext) tuples for fields that carry an External annotation.

    Plain pydantic fields (no External) are inlined into their parent file and
    don't appear as separate filesystem entries, so they're excluded here.
    """
    ext_fields: list[tuple[str, FieldInfo, External]] = []
    for name, info in model_cls.model_fields.items():
        ext = get_metadata(info.metadata, External)
        if ext is not None:
            ext_fields.append((name, info, ext))
    return ext_fields


def _build_lines(model_cls: type[BaseModel], prefix: str, seen: frozenset[type[BaseModel]] = frozenset()) -> list[str]:
    """
    Recursively build tree lines for all External-annotated fields of model_cls.

    prefix is the indentation string accumulated from parent calls — it grows by
    _PIPE or _SPACE each level depending on whether the parent was the last sibling.

    seen tracks the model classes already visited on the current path, so a
    self- or mutually-referencing model (e.g. a model that can nest itself)
    terminates with a "(recursive)" marker instead of recursing forever.
    """
    seen = seen | {model_cls}
    lines: list[str] = []
    ext_fields = _collect_ext_fields(model_cls)

    for i, (field_name, field_info, ext) in enumerate(ext_fields):
        is_last = i == len(ext_fields) - 1

        # ├── for all siblings except the last one, which gets └──
        connector = _LAST if is_last else _BRANCH
        # The prefix for children: blank space under the last sibling, pipe under others
        child_prefix = prefix + (_SPACE if is_last else _PIPE)

        ann = field_info.annotation
        # A field is optional if its type allows None OR it has a default value
        is_opt = _is_optional(ann) or not field_info.is_required()
        req_mark = "   " if is_opt else "!! "

        # OutputStrategy.SINGLE_FILE means this field serialises to one .yaml file;
        # anything else (FOLDER / AUTO) means it gets its own subdirectory
        is_file = OutputStrategy.SINGLE_FILE in ext.output_structure
        display = _display_name(field_name, ext, is_file)
        # Trailing "/" marks a folder node; file nodes already carry the .flync.yaml suffix
        display = display if is_file else display + "/"

        lines.append(f"{prefix}{connector}{req_mark}{display}")

        if not is_file:
            lines.extend(_build_folder_children_lines(field_name, ann, child_prefix, seen))

    return lines


def _build_folder_children_lines(field_name: str, ann: Any, child_prefix: str, seen: frozenset[type[BaseModel]]) -> list[str]:
    """Build the child lines for a folder-node field: list-item placeholder or nested model."""
    is_list, inner = _unwrap_type(ann)
    if not _is_pydantic_model(inner):
        return []
    if is_list:
        return _build_list_item_lines(field_name, inner, child_prefix, seen)
    return [f"{child_prefix}{_LAST}(recursive)"] if inner in seen else _build_lines(inner, child_prefix, seen)


def _build_list_item_lines(field_name: str, inner: type[BaseModel], child_prefix: str, seen: frozenset[type[BaseModel]]) -> list[str]:
    """
    Build placeholder lines for a List[Model] field.

    e.g. List[ECU]: each item lives in its own named subfolder. We don't have
    real data here, so show one placeholder row followed by "..." to indicate
    N items exist at runtime. If the item type has no External fields every
    item serialises to a single file (not a sub-directory), so a trailing "/"
    is omitted in that case.

    If inner was already visited on this path (a model that can nest itself,
    directly or through another model), the nested breakdown is replaced with
    a "(recursive)" marker instead of recursing forever.
    """
    item_has_substructure = _has_external_fields(inner)
    item_placeholder = f"<{field_name}_item>" + ("/" if item_has_substructure else FLYNC_EXT)

    lines = [f"{child_prefix}│", f"{child_prefix}{_BRANCH}{item_placeholder}"]
    if item_has_substructure:
        if inner in seen:
            lines.append(f"{child_prefix}{_PIPE}{_LAST}(recursive)")
        else:
            lines.extend(_build_lines(inner, child_prefix + _PIPE, seen))
    lines.append(f"{child_prefix}│")
    lines.append(f"{child_prefix}{_LAST}...")
    return lines


def print_field_subtree(parent_cls: type[BaseModel], field_name: str) -> Path:
    """
    Write the structure for a single External-annotated field of parent_cls.

    Used for fields whose item type has no External sub-fields (leaf lists like
    can_buses, pdus, sockets …). Calling print_flync_structure on the item class
    itself would show an empty tree; this function instead renders the folder
    node and its file placeholders, giving the user the useful view.

    Output is written to exports/<display_name>_structure.txt.
    """
    field_info = parent_cls.model_fields[field_name]
    ext = get_metadata(field_info.metadata, External)
    if ext is None:
        raise ValueError(f"{parent_cls.__name__}.{field_name} has no External annotation")

    # Folder display name comes from the External annotation (same rule as _display_name)
    folder_name = ext.path if ext.naming_strategy == NamingStrategy.FIXED_PATH and ext.path else field_name

    is_list, inner = _unwrap_type(field_info.annotation)
    lines = ["!! required", "", f"{folder_name}/"]

    if _is_pydantic_model(inner):
        if is_list:
            lines.extend(_build_list_item_lines(field_name, inner, "", frozenset()))
        else:
            # Single non-list folder — recurse directly
            lines.extend(_build_lines(inner, ""))

    content = "\n".join(lines) + "\n"
    _EXPORTS_DIR.mkdir(exist_ok=True)
    out = _EXPORTS_DIR / f"{folder_name}_structure.txt"
    out.write_text(content, encoding="utf-8")
    return out


def print_flync_structure(model_cls: type[BaseModel] | None = None) -> Path:
    """
    Write the FLYNC directory structure to exports/<ModelName>_structure.txt.

    Starts traversal from FLYNCModel by default. Pass any other pydantic model
    class to visualise a subtree instead.

    Returns the Path of the file that was written.
    """
    if model_cls is None:
        from flync.model.flync_model import FLYNCModel

        model_cls = FLYNCModel

    lines = ["!! required", "", f"{model_cls.__name__}/"] + _build_lines(model_cls, "")
    content = "\n".join(lines) + "\n"

    _EXPORTS_DIR.mkdir(exist_ok=True)
    out = _EXPORTS_DIR / f"{model_cls.__name__}_structure.txt"
    out.write_text(content, encoding="utf-8")
    return out
