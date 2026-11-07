"""
Docstring/field consistency gate for the FLYNC Pydantic models.
===============================================================

Purpose
-------
Every FLYNC model documents its fields in a NumPy-style ``Parameters`` section.
That section drives both the Sphinx documentation and the YAML schema reference,
so it must stay in sync with the actual Pydantic ``model_fields``.

This script imports every Pydantic model under the packages listed in
``DEFAULT_PACKAGES`` and compares each ``Parameters`` entry against the model:

============ ==========================================================================================
``missing``  a model field that no ``Parameters`` entry documents
``unknown``  a ``Parameters`` entry that is not a field of the class
``type``     a documented type that does not match the annotation
``optional`` an entry whose ``, optional`` suffix disagrees with the field having a default
``bounds``   a ``Field`` constraint (``ge``/``le``/``gt``/``lt``/lengths) the description never states
============ ==========================================================================================

Usage
-----
    uv run python scripts/ci/check_model_docstrings.py                   # all checks, all packages
    uv run python scripts/ci/check_model_docstrings.py --select missing,unknown
    uv run python scripts/ci/check_model_docstrings.py --package flync.model.flync_4_tsn
    uv run python scripts/ci/check_model_docstrings.py --strict          # exit non-zero on any finding


Required docstring format for FLYNC models
------------------------------------------
Every Pydantic model class must carry a docstring that follows the structure below.
Agents and contributors must use this as the single authoritative template.

Structure
~~~~~~~~~

(1) One-line or short summary of what the class represents.

    Optionally followed by a longer explanation paragraph separated by a blank line.

(2) ``Parameters`` section — documents every field the class itself declares.

    Inherited fields belong to the base class's ``Parameters`` section and must NOT
    be repeated here.

    The section heading must be exactly the word ``Parameters`` followed immediately
    on the next line by a run of three or more dashes (``---``).  This is the NumPy
    docstring convention that Sphinx and this script both expect.

    Each entry follows the sub-structure:

    (2.1) Header line::

            <name> : <type>[, optional]

          - ``<name>``  — exact Python field name.
          - ``<type>``  — the public-facing type, e.g. ``int``, ``str``,
            ``list of :class:`~Foo```.
            Use ``|`` or ``or`` to separate union members.
          - ``, optional`` — append this suffix when the field has a default value
            (i.e. it is not required).  Omit it for required fields.

    (2.2) Description — indented body text explaining what the field controls.

    (2.3) Default — when the field is optional, state the default explicitly::

            Defaults to ``<value>``.

          This may appear inside the description sentence or as its own sentence.
          The word ``default`` anywhere in the description also satisfies the check.

    (2.4) Constraints — when the field carries a ``Field(ge=…)`` / ``le`` / ``gt`` /
          ``lt`` / ``min_length`` / ``max_length`` constraint, the description must
          state the bound in one of the accepted phrasings listed in ``_CONSTRAINTS``
          (e.g. ``>= 0``, ``at least 0``, ``minimum 0``, ``non-negative``).
          Stating the numeric value directly also satisfies the check.

    Example::

        Parameters
        ----------
        name : str
            Human-readable identifier for this entry.
        count : int, optional
            Number of repetitions (>= 1). Defaults to ``1``.
        mode : Literal["tx", "rx"]
            Direction of data flow.


(3) ``Private Attributes`` section — documents ``PrivateAttr`` fields.

    Same heading convention (word + dashes), same per-entry sub-structure as
    Parameters.  Private attributes are never validated by this script but should
    be documented for human readers.

    Example::

        Private Attributes
        ------------------
        _ecu : :class:`~ECU`
            Back-reference to the owning ECU. Managed internally.


Skipped fields
~~~~~~~~~~~~~~
- ``root`` on a ``RootModel`` subclass — synthetic field, documented by the class prose.
- Any field named ``type`` whose annotation is ``Literal[…]`` — a discriminator tag
  whose only legal value is already spelled out by the type itself.
- Fields declared on a base class — documented once on the base, not repeated here.

Type aliases accepted by the ``type`` check
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The checker accepts common English synonyms for Python type names.
See ``TYPE_ALIASES`` at the top of this file for the full mapping
(e.g. ``"boolean"`` → ``bool``, ``"string"`` → ``str``).

Bounds phrasing accepted by the ``bounds`` check
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
See ``_CONSTRAINTS`` at the top of this file for the full list of accepted phrases
per constraint type (e.g. ``ge`` accepts ``">= 0"``, ``"at least 0"``, ``"minimum 0"``,
``"non-negative"``).
"""

import argparse
import ast
import importlib
import inspect
import pkgutil
import re
import sys
import types
import typing
from pathlib import Path

import annotated_types
from pydantic import BaseModel, RootModel
from pydantic_core import PydanticUndefined

# ---------------------------------------------------------------------------
# Configuration — edit these to adapt the script to a different project
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parents[2]

# Packages scanned when --package is not supplied on the command line.
DEFAULT_PACKAGES = ("flync.core", "flync.model")

# All available check names, in the order they appear in the summary.
CHECKS = ("missing", "unknown", "type", "optional", "bounds")

# Human-readable explanation shown in the per-check summary line.
CHECK_EXPLANATIONS = {
    "missing": "declared field the docstring never documents",
    "unknown": "documented parameter that is not a field",
    "type": "documented type the annotation cannot hold",
    "optional": "default or requiredness the docstring contradicts",
    "bounds": "Field constraint the description never states",
}

# Section headings that end the Parameters block.
# The parser stops when it encounters any of these as an unindented line.
SECTION_STOP_HEADINGS = {"Private Attributes", "Notes", "Examples", "See Also"}

# Minimum number of dashes required to recognise a NumPy section underline.
MIN_UNDERLINE_DASHES = 3

# Docstring spellings that mean the same annotation.
# Keys and values are compared lower-cased; only spellings that differ from the
# Python name by more than case need to appear here.
TYPE_ALIASES: dict[str, str] = {
    "boolean": "bool",
    "integer": "int",
    "string": "str",
    "text": "str",
    "sequence": "list",
    "tuple": "list",
    "mapping": "dict",
    "number": "float",
    "none": "none",
    "nonetype": "none",
    "any": "any",
    # IPvAnyAddress accepts both IPv4 and IPv6; docstrings often spell them out.
    "ipvanyaddress": "ipv4address|ipv6address",
    "ipaddress": "ipv4address|ipv6address",
    # Reverse direction: a docstring saying IPv4Address or IPv6Address matches IPvAnyAddress.
    "ipv4address": "ipvanyaddress",
    "ipv6address": "ipvanyaddress",
}

# Tokens that are always accepted without a matching annotated leaf.
# "any" and "none" carry structural meaning, not a concrete type.
STRUCTURAL_TOKENS: frozenset[str] = frozenset({"any", "none"})

# Per-constraint configuration: maps an annotated_types class to a tuple of
#   (constraint_name, report_wording, accepted_description_phrases)
# Add or extend entries here to teach the script new bound types.
_CONSTRAINTS: dict[type, tuple[str, str, tuple[str, ...]]] = {
    annotated_types.Ge: (
        "ge",
        "greater or equal",
        (
            "greater or equal",
            "greater than or equal",
            "at least",
            "minimum",
            "min.",
            "no less than",
            "not less than",
            "non-negative",
            "positive",
            ">=",
        ),
    ),
    annotated_types.Gt: (
        "gt",
        "greater than",
        ("greater than", "more than", "larger than", "above", "exclusive minimum", "positive", "non-empty", "at least one", ">"),
    ),
    annotated_types.Le: (
        "le",
        "less or equal",
        ("less or equal", "less than or equal", "at most", "maximum", "max.", "no more than", "not more than", "up to", "<="),
    ),
    annotated_types.Lt: (
        "lt",
        "less than",
        ("less than", "smaller than", "below", "under", "exclusive maximum", "<"),
    ),
    annotated_types.MinLen: (
        "min_length",
        "minimum length",
        ("minimum length", "at least", "no shorter than", "not shorter than", "non-empty", "at least one"),
    ),
    annotated_types.MaxLen: (
        "max_length",
        "maximum length",
        ("maximum length", "at most", "no longer than", "not longer than", "up to"),
    ),
    annotated_types.MultipleOf: (
        "multiple_of",
        "multiple of",
        ("multiple of", "step of", "increments of"),
    ),
}

# Compiled regex patterns used throughout the script
_RE_SECTION_UNDERLINE = re.compile(r"^-{" + str(MIN_UNDERLINE_DASHES) + r",}$")
_RE_PARAM_HEAD = re.compile(r"^(?P<names>[^:]+?)\s*:\s*(?P<type>.*)$")


# ---------------------------------------------------------------------------
# Model discovery
# ---------------------------------------------------------------------------


def discover_models(package_names: tuple[str, ...]) -> tuple[list[type], list[tuple[str, Exception]]]:
    """
    Import *package_names* recursively and return every Pydantic model they define.

    Returns a 2-tuple of ``(models, import_failures)`` where ``import_failures`` is
    a list of ``(module_name, exception)`` pairs for modules that could not be loaded.
    A single import failure never aborts the scan of the remaining modules.
    """
    models: dict[str, type] = {}
    failed: list[tuple[str, Exception]] = []
    top_level_roots = {name.split(".")[0] for name in package_names}

    for package_name in package_names:
        package = importlib.import_module(package_name)
        module_names = [package_name]
        if hasattr(package, "__path__"):
            module_names += [info.name for info in pkgutil.walk_packages(package.__path__, f"{package_name}.")]

        for module_name in module_names:
            try:
                module = importlib.import_module(module_name)
            except Exception as error:  # noqa: BLE001
                failed.append((module_name, error))
                continue

            for obj in vars(module).values():
                if not inspect.isclass(obj) or not issubclass(obj, BaseModel):
                    continue
                if obj.__module__.split(".")[0] not in top_level_roots:
                    continue
                if not any(obj.__module__ == n or obj.__module__.startswith(f"{n}.") for n in package_names):
                    continue
                models[f"{obj.__module__}.{obj.__qualname__}"] = obj

    return [models[key] for key in sorted(models)], failed


# ---------------------------------------------------------------------------
# Docstring parsing
# ---------------------------------------------------------------------------


def _get_own_docstring_lines(cls: type) -> list[str]:
    """Return cleaned lines of the docstring written directly on *cls*, or ``[]``."""
    docstring = cls.__dict__.get("__doc__")
    if not docstring:
        return []
    return inspect.cleandoc(docstring).splitlines()


def _find_section_start(lines: list[str], heading: str) -> int | None:
    """Return the index of the first content line after *heading* + its underline, or ``None``."""
    for i, line in enumerate(lines[:-1]):
        if line.strip() == heading and _RE_SECTION_UNDERLINE.match(lines[i + 1]):
            return i + 2
    return None


def _find_section_end(lines: list[str], start: int, stop_headings: set[str]) -> int:
    """
    Return the index of the line that terminates the section beginning at *start*.

    The section ends at the next NumPy section heading (any word followed by a dashes
    underline) or at one of the *stop_headings* names appearing as an unindented line.
    """
    for i in range(start, len(lines) - 1):
        if lines[i].strip() and _RE_SECTION_UNDERLINE.match(lines[i + 1]):
            return i
        if lines[i].strip() in stop_headings:
            return i
    return len(lines)


def _parse_entries(lines: list[str]) -> dict[str, tuple[str, str]]:
    """
    Split a ``Parameters``-style body into ``{name: (type_string, description)}`` entries.

    An entry header sits at column 0 and matches ``<names> : <type>``.
    Everything indented below it (until the next header) is joined into the description.
    """
    entries: dict[str, tuple[str, str]] = {}
    head: str | None = None
    body: list[str] = []

    def _flush(h: str, b: list[str]) -> None:
        match = _RE_PARAM_HEAD.match(h)
        if not match:
            return
        type_string = (match.group("type") or "").strip()
        description = " ".join(part.strip() for part in b).strip()
        for raw_name in match.group("names").split(","):
            name = raw_name.strip().lstrip("*")
            if name:
                entries[name] = (type_string, description)

    for line in lines:
        if line and line[0] not in (" ", "\t"):
            # Unindented line: flush the previous entry and start a new one.
            if head is not None:
                _flush(head, body)
            head = line
            body = []
        else:
            # Indented or blank: belongs to the body of the current entry.
            body.append(line)

    if head is not None:
        _flush(head, body)
    return entries


def parse_parameters(cls: type) -> dict[str, tuple[str, str]]:
    """
    Return ``{field_name: (type_string, description)}`` from the class's own ``Parameters`` section.

    Only the docstring written directly on *cls* is considered.  The section must use
    the NumPy convention: the word ``Parameters`` followed on the very next line by
    three or more dashes.
    """
    lines = _get_own_docstring_lines(cls)
    start = _find_section_start(lines, "Parameters")
    if start is None:
        return {}
    end = _find_section_end(lines, start, SECTION_STOP_HEADINGS)
    return _parse_entries(lines[start:end])


# ---------------------------------------------------------------------------
# Field introspection
# ---------------------------------------------------------------------------


def documented_anywhere(cls: type) -> dict[str, tuple[str, str]]:
    """
    Merge the ``Parameters`` entries of *cls* and all its base classes.

    The nearest definition wins (MRO order, reversed so bases are overridden by
    subclasses).  Used to detect fields already documented on a parent so they are
    not flagged as ``missing`` on the subclass.
    """
    merged: dict[str, tuple[str, str]] = {}
    for klass in reversed(cls.__mro__):
        if isinstance(klass, type) and issubclass(klass, BaseModel):
            merged.update(parse_parameters(klass))
    return merged


def own_fields(cls: type) -> dict[str, object]:
    """
    Return only the fields that *cls* itself declares (not inherited ones).

    Pydantic's ``model_fields`` includes inherited fields, so we intersect with
    ``__annotations__`` which only contains names declared on this class body.
    """
    declared = cls.__dict__.get("__annotations__", {})
    return {name: field for name, field in cls.model_fields.items() if name in declared}


def _is_skipped(cls: type, name: str, field: object) -> bool:
    """True when a field should not be checked."""
    # ``root`` on a ``RootModel``.
    if isinstance(cls, type) and issubclass(cls, RootModel) and name == "root":
        return True
    # A ``type`` field that is a discriminator Literal.
    if name == "type":
        origin = typing.get_origin(field.annotation)  # type: ignore[union-attr]
        if origin is typing.Literal:
            return True
    return False


# ---------------------------------------------------------------------------
# Type and annotation matching
# ---------------------------------------------------------------------------


def docstring_tokens(type_string: str) -> tuple[set[str], bool]:
    """
    Return the set of type name tokens extracted from a docstring type string,
    and whether the string ends with ", optional".

    Raises are not checked; a malformed string returns ``(set(), False)``.

    RST cross-reference syntax (``:role:`...``` ) is stripped so that tokens
    like ``"class"``, ``"meth"``, or full dotted module paths (``flync.model…``)
    do not pollute the token set.  The structural words ``"of"``, ``"or"``,
    ``"and"`` and ``"optional"`` are also excluded since they are conjunctions or
    handled separately.
    """
    documented_optional = ", optional" in type_string
    # Remove the ", optional" suffix before tokenizing.
    clean = type_string.replace(", optional", "")
    # Strip RST role prefixes: :class:, :meth:, :attr:, etc.
    clean = re.sub(r":[a-zA-Z_]+:", "", clean)
    # Strip tilde prefix (used for cross-references like ~flync.model.Foo).
    clean = re.sub(r"~", "", clean)
    # Collapse dotted paths (e.g. "flync.model.flync_4_ecu.Foo") to the last segment only.
    clean = re.sub(r"(?:[a-zA-Z_][\w]*\.)+([A-Za-z_]\w*)", r"\1", clean)
    # Tokenize on word boundaries.
    raw_tokens = re.findall(r"\b\w+\b", clean.lower())
    # Drop structural/conjunction words that are never type names.
    # "to" appears in "dict of str to str" and must not be treated as a token.
    _NOISE = frozenset({"of", "or", "and", "to", "optional", "a", "an", "the"})
    tokens = {t for t in raw_tokens if t not in _NOISE}
    return tokens, documented_optional


def annotation_tokens(annotation: object) -> set[str]:
    """
    Return the set of type name tokens that can appear in *annotation*.

    Recurses through unions, generics, Literal, and ForwardRef.

    Important: we must **not** overwrite ``annotation`` with ``get_origin()``
    before checking the branch conditions.  For ``Optional[str]`` the origin is
    ``Union``, but ``get_origin(Union)`` is ``None``, so the Union branch would
    never fire.  The same applies to ``Literal[…]``.  We therefore keep the
    original form and only call ``get_origin`` to *decide* which branch to take.
    """
    # Unwrap all layers of Annotated[T, ...] first, keeping the inner type intact.
    while typing.get_origin(annotation) is typing.Annotated:
        annotation = typing.get_args(annotation)[0]

    origin = typing.get_origin(annotation)

    # Union (covers Optional[X] = Union[X, None]).
    if origin is typing.Union:
        tokens: set[str] = set()
        for arg in typing.get_args(annotation):
            tokens.update(annotation_tokens(arg))
        return tokens

    # Literal: each literal value is a valid token.
    # We also add:
    #   - "literal" itself so docstrings that write ``Literal["x"]`` match.
    #   - the Python type name of each value (e.g. "int" for Literal[100, 1000]).
    #   - for string values, sub-tokens split on non-alphanumeric chars
    #     (e.g. "1.3" → {"1", "3"}) so version-string literals match.
    #   - for int values, the hex representation (e.g. 0x22F0 → "0x22f0")
    #     so docstrings that write the hex form are not rejected.
    if origin is typing.Literal:
        tokens = {"literal"}
        for arg in typing.get_args(annotation):
            s = str(arg).lower()
            tokens.add(s)
            tokens.add(type(arg).__name__.lower())
            if isinstance(arg, str):
                tokens.update(re.findall(r"[a-zA-Z0-9]+", s))
            if isinstance(arg, int):
                tokens.add(hex(arg))
        return tokens

    # Generic containers: List[X], Dict[K, V], etc.
    if origin is not None:
        tokens = set()
        if isinstance(origin, type):
            tokens.add(origin.__name__.lower())
        for arg in typing.get_args(annotation):
            tokens.update(annotation_tokens(arg))
        return tokens

    # Plain type or ForwardRef string.
    if isinstance(annotation, type):
        return {annotation.__name__.lower()}
    if isinstance(annotation, str):
        return {annotation.lower()}
    # typing.ForwardRef (unresolved forward reference).
    if isinstance(annotation, typing.ForwardRef):
        return {annotation.__forward_arg__.lower()}
    return set()


def unmatched_types(documented: set[str], annotated: set[str]) -> set[str]:
    """Return the tokens from *documented* that do not appear in *annotated*.

    ``TYPE_ALIASES`` is applied to each documented token before comparison so that
    common synonyms (e.g. ``"ipv4address"`` → ``"ipvanyaddress"``) are accepted.
    """
    result: set[str] = set()
    for name in documented:
        if name in STRUCTURAL_TOKENS:
            continue
        # Direct match.
        if name in annotated:
            continue
        # Apply a static alias: the docstring may use a narrower or alternative spelling.
        alias = TYPE_ALIASES.get(name)
        if alias is not None and any(part in annotated for part in alias.split("|")):
            continue
        result.add(name)
    return result


def expand_aliases(tokens: set[str], module: types.ModuleType) -> set[str]:
    """
    Expand type aliases in *tokens* by looking them up in *module*.

    A token that names a module-level assignment is replaced by the tokens in
    the value it assigns. Non-existent names and circular references are skipped.
    """
    expanded: set[str] = set()
    seen: set[str] = set()

    def _expand_one(name: str) -> set[str]:
        if name in seen or name in STRUCTURAL_TOKENS or name in TYPE_ALIASES:
            return set()
        seen.add(name)

        # Try as a module-level name.  The docstring token is lower-cased so we
        # must do a case-insensitive lookup among module attributes.
        obj = None
        name_lower = name.lower()
        for attr_name in dir(module):
            if attr_name.lower() == name_lower:
                try:
                    obj = getattr(module, attr_name)
                except AttributeError:
                    pass
                break
        if obj is None:
            return set()

        # Expand as a type alias or class reference.
        # First try get_type_hints (catches module-level annotated assignments).
        annotation = typing.get_type_hints(module, include_extras=True).get(name[0].upper() + name[1:])
        if annotation is not None:
            return annotation_tokens(annotation)
        # Fall back to the object itself: type aliases (TypeAlias, Annotated unions, etc.)
        # are not captured by get_type_hints but their tokens can be read directly.
        if typing.get_origin(obj) is not None or isinstance(obj, type):
            return annotation_tokens(obj)
        return set()

    for token in tokens:
        expanded.update(annotation_tokens(_expand_one(token)) if _expand_one(token) else {token})
    return expanded


# ---------------------------------------------------------------------------
# Constraint checking
# ---------------------------------------------------------------------------


def field_constraints(field: object) -> list[tuple[str, str, tuple[str, ...], str | int | float]]:
    """
    Yield each constraint on *field* as a 4-tuple of
    (constraint_name, wording, accepted_phrases, value).
    """
    metadata = typing.get_args(field.annotation)[1:] if typing.get_origin(field.annotation) is typing.Annotated else ()  # type: ignore[union-attr]

    for constraint_type, (name, wording, phrases) in typing.cast(
        dict[type, tuple[str, str, tuple[str, ...]]],
        {k: v for k, v in _CONSTRAINTS.items()},
    ).items():
        for annotation in metadata:
            if isinstance(annotation, constraint_type):
                yield (name, wording, phrases, annotation.value)  # type: ignore[union-attr]


def describes_bound(combined_text: str, phrases: tuple[str, ...], value: str | int | float) -> bool:
    """
    True when *combined_text* mentions the bound specified by *phrases* and *value*.

    The text is searched for any of the *phrases*, or for the numeric value itself.
    """
    combined_lower = combined_text.lower()
    for phrase in phrases:
        if phrase in combined_lower:
            return True
    return str(value) in combined_text


# ---------------------------------------------------------------------------
# Validator introspection
# ---------------------------------------------------------------------------


def _cls_source_text(cls: type) -> str:
    """Return the source text of *cls*, or ``""`` on failure."""
    try:
        return inspect.getsource(cls)
    except (OSError, TypeError):
        return ""


def _own_validator_methods(cls: type) -> list[ast.FunctionDef]:
    """
    Return AST nodes for validator methods declared directly on *cls*.

    Parses only the source of the class body — inherited validators are not included.
    Returns ``[]`` when the source cannot be parsed.
    """
    source = _cls_source_text(cls)
    if not source:
        return []
    try:
        tree = ast.parse(source)
    except SyntaxError:
        return []

    # The parsed source starts at the class definition itself.
    class_nodes = [n for n in ast.walk(tree) if isinstance(n, ast.ClassDef)]
    if not class_nodes:
        return []
    # Take the first class node that matches the class name.
    target = next((n for n in class_nodes if n.name == cls.__name__), class_nodes[0])

    validators = []
    for node in target.body:
        if not isinstance(node, ast.FunctionDef):
            continue
        dec_names = {_decorator_name(d) for d in node.decorator_list}
        if dec_names & VALIDATOR_DECORATOR_NAMES:
            validators.append(node)
    return validators


def _decorator_name(node: ast.expr) -> str:
    """Extract the bare name from a decorator node (handles ``@name`` and ``@name(…)``)."""
    if isinstance(node, ast.Name):
        return node.id
    if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
        return node.func.id
    if isinstance(node, ast.Attribute):
        return node.attr
    return ""


def _validator_calls_error_factory(func_node: ast.FunctionDef) -> set[str]:
    """Return the set of error-factory names called anywhere inside *func_node*."""
    called: set[str] = set()
    for node in ast.walk(func_node):
        if isinstance(node, ast.Call):
            name = ""
            if isinstance(node.func, ast.Name):
                name = node.func.id
            elif isinstance(node.func, ast.Attribute):
                name = node.func.attr
            if name in ERROR_FACTORY_NAMES:
                called.add(name)
    return called


def class_uses_error_factories(cls: type) -> set[str]:
    """
    Return the set of error-factory names (e.g. ``err_major``) called by any validator on *cls*.

    Used by the ``raises`` check to detect when a class has validators that raise errors.
    """
    factories: set[str] = set()
    for func_node in _own_validator_methods(cls):
        factories.update(_validator_calls_error_factory(func_node))
    return factories


# ---------------------------------------------------------------------------
# Finding and reporting
# ---------------------------------------------------------------------------


class Finding:
    """One inconsistency between a documented parameter and the field it documents."""

    def __init__(self, check: str, cls: type, field: str, message: str):
        self.check = check
        self.cls = cls
        self.location = f"{cls.__module__}.{cls.__qualname__}"
        self.field = field
        self.message = message

    def __str__(self) -> str:
        if self.field:
            return f"  [{self.check}] {self.field}: {self.message}"
        return f"  [{self.check}] {self.message}"


def _module_to_filepath(module_name: str) -> str | None:
    """
    Return the file path for a module name, or None if not found.

    Converts ``flync.model.flync_4_ecu.controller`` to ``src/flync/model/flync_4_ecu/controller.py``.
    """
    try:
        module = sys.modules.get(module_name)
        if module is None:
            return None
        filepath = getattr(module, "__file__", None)
        if filepath:
            # Convert absolute path to relative path from repo root if possible.
            try:
                rel = Path(filepath).relative_to(REPO_ROOT)
                return str(rel)
            except ValueError:
                # Not under repo root, return absolute path
                return filepath
    except (AttributeError, TypeError):
        pass
    return None


def _class_to_line_number(cls: type) -> int | None:
    """
    Return the line number where *cls* is defined, or None if not found.

    Uses inspect.getsourcelines() to locate the class definition in its source file.
    """
    try:
        _, lineno = inspect.getsourcelines(cls)
        return lineno
    except (OSError, TypeError):
        return None


def render(findings: list[Finding]) -> list[str]:
    """
    Render findings grouped by Python module with file locations and class links.

    Format:
        src/path/to/module.py (module.name):
            ClassName (src/path/to/module.py:123):
              [check] field: message

    The ClassName line includes a file:line link (e.g. ``ClassName (file.py:123)``)
    that can be clicked in IDEs supporting the standard ``file:line`` format.
    """
    lines: list[str] = []

    # Group findings by module (extracted from location)
    by_module: dict[str, list[tuple[str, Finding]]] = {}
    for finding in findings:
        # location is "module.path.ClassName"
        parts = finding.location.rsplit(".", 1)
        if len(parts) == 2:
            module_name, class_name = parts
        else:
            module_name = finding.location
            class_name = ""

        if module_name not in by_module:
            by_module[module_name] = []
        by_module[module_name].append((class_name, finding))

    # Output grouped by module
    for module_name in sorted(by_module.keys()):
        filepath = _module_to_filepath(module_name)
        lines.append("")
        if filepath:
            lines.append(f"{filepath}:")
        else:
            lines.append(f"{module_name}:")

        # Group class findings within each module
        by_class: dict[str, list[Finding]] = {}
        for class_name, finding in by_module[module_name]:
            if class_name not in by_class:
                by_class[class_name] = []
            by_class[class_name].append(finding)

        for class_name in sorted(by_class.keys()):
            if class_name:
                # Get line number for the class and format as clickable link.
                # We need to get the actual class object to find its line number.
                lineno = None
                if by_class[class_name]:
                    # All findings for this class have the same cls, use the first one.
                    cls_obj = by_class[class_name][0].cls
                    lineno = _class_to_line_number(cls_obj)

                if lineno is not None and filepath:
                    lines.append(f"  {class_name} ({filepath}:{lineno}):")
                else:
                    lines.append(f"  {class_name}:")

                for finding in by_class[class_name]:
                    lines.append(f"    {finding}")
            else:
                # Module-level findings (rare)
                for finding in by_class[class_name]:
                    lines.append(f"  {finding}")

    return lines


# ---------------------------------------------------------------------------
# Checking logic
# ---------------------------------------------------------------------------


def _check_missing(cls: type, own: dict[str, tuple[str, str]], fields: dict[str, object], findings: list[Finding]) -> None:
    """Append ``missing`` findings for fields not documented."""
    inherited = documented_anywhere(cls)
    for name in fields:
        if name not in own and name not in inherited:
            findings.append(Finding("missing", cls, name, "field is not documented in the Parameters section"))


def _check_unknown(cls: type, own: dict[str, tuple[str, str]], findings: list[Finding]) -> None:
    """Append ``unknown`` findings for documented entries that are not fields."""
    for name in own:
        if name not in cls.model_fields:
            findings.append(Finding("unknown", cls, name, "documented parameter is not a field of this model"))


def _check_type(cls: type, name: str, field: object, type_string: str, findings: list[Finding]) -> None:
    """
    Append a ``type`` finding when no annotated token matches the documented type string.

    Both the raw documented tokens and the alias-expanded tokens are tried, so a docstring
    that names a module-level type alias is not falsely flagged.
    """
    if not type_string:
        return
    module = sys.modules.get(cls.__module__)
    documented, _ = docstring_tokens(type_string)
    annotated = annotation_tokens(field.annotation)  # type: ignore[union-attr]
    unmatched = unmatched_types(documented, annotated)
    if unmatched and module is not None:
        unmatched &= unmatched_types(expand_aliases(documented, module), annotated)
    if unmatched:
        findings.append(
            Finding(
                "type",
                cls,
                name,
                f"documented as '{type_string}' but annotated {_render_annotation(field.annotation)}"  # type: ignore[union-attr]
                f" - nothing in the annotation matches {', '.join(sorted(unmatched))}",
            )
        )


def _check_optional(cls: type, name: str, field: object, type_string: str, description: str, findings: list[Finding]) -> None:
    """
    Append an ``optional`` finding when the ``, optional`` suffix and the field default disagree.

    The word ``"default"`` anywhere in the combined type+description string is accepted as
    an alternative to the ``, optional`` suffix (e.g. ``"Defaults to 0."`` in the body).
    """
    if not type_string:
        return
    _, documented_optional = docstring_tokens(type_string)
    says_default = documented_optional or "default" in f"{type_string} {description}".lower()
    if field.is_required() and documented_optional:  # type: ignore[union-attr]
        findings.append(Finding("optional", cls, name, "documented as optional but is a required field"))
    elif _has_stated_default(field) and not says_default:
        findings.append(
            Finding(
                "optional",
                cls,
                name,
                f"defaults to {field.default!r} but is documented as neither optional nor defaulted",  # type: ignore[union-attr]
            )
        )


def _check_bounds(cls: type, name: str, field: object, type_string: str, description: str, findings: list[Finding]) -> None:
    """Append ``bounds`` findings for Field constraints not mentioned in the description."""
    combined = f"{type_string} {description}"
    for constraint, wording, phrases, value in field_constraints(field):
        if not describes_bound(combined, phrases, value):
            findings.append(Finding("bounds", cls, name, f"{constraint}={value} ('{wording} {value}') is not stated in the description"))


def _has_stated_default(field: object) -> bool:
    """
    True for a default a reader has to be told about.

    A ``default_factory`` is skipped: it is what builds the empty list or dict a collection field starts as,
    and ``list of X`` already says the field may be left out. A ``Literal`` default is skipped too - a
    discriminator or a fixed constant has exactly one legal value, which the documented type spells out.
    """
    if field.default is PydanticUndefined or field.default_factory is not None:  # type: ignore[union-attr]
        return False
    return typing.get_origin(_unwrap_annotated(field.annotation)) is not typing.Literal  # type: ignore[union-attr]


def _unwrap_annotated(annotation: object) -> object:
    """Remove all layers of Annotated wrapping."""
    while typing.get_origin(annotation) is typing.Annotated:
        annotation = typing.get_args(annotation)[0]
    return annotation


def _render_annotation(annotation: object) -> str:
    """Spell an annotation the way the docstring would: no module paths, no validator reprs, no memory addresses."""
    if isinstance(annotation, type):
        return annotation.__name__

    text = str(annotation)
    text = re.sub(r"<function [^>]*>", "...", text)
    text = re.sub(
        r"\b(BeforeValidator|AfterValidator|PlainValidator|WrapValidator|PlainSerializer|FieldInfo|BeforeValidator)\([^()]*\)",
        r"\1(...)",
        text,
    )
    text = re.sub(r"\b(?:[a-zA-Z_][\w]*\.)+([A-Za-z_]\w*)", r"\1", text)
    return text


def check_model(cls: type, selected: set[str]) -> list[Finding]:
    """
    Compare one model's own ``Parameters`` section with the fields it declares.

    Both halves are restricted to fields and entries declared on *cls* itself.
    Inherited fields are the base class's responsibility to document; inherited
    docstring entries are the base class's responsibility to keep correct.  Checking
    them again here would report the same drift once per subclass.
    """
    own = parse_parameters(cls)
    fields = {n: f for n, f in own_fields(cls).items() if not _is_skipped(cls, n, f)}
    findings: list[Finding] = []

    if "missing" in selected:
        _check_missing(cls, own, fields, findings)
    if "unknown" in selected:
        _check_unknown(cls, own, findings)

    for name, field in fields.items():
        entry = own.get(name)
        if entry is None:
            continue
        type_string, description = entry
        if "type" in selected:
            _check_type(cls, name, field, type_string, findings)
        if "optional" in selected:
            _check_optional(cls, name, field, type_string, description, findings)
        if "bounds" in selected:
            _check_bounds(cls, name, field, type_string, description, findings)

    return findings


# ---------------------------------------------------------------------------
# CLI entry point
# ---------------------------------------------------------------------------


def _parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__.split("Usage")[0].strip(),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--package",
        action="append",
        dest="packages",
        metavar="NAME",
        help=f"package to scan (repeatable; default: {', '.join(DEFAULT_PACKAGES)})",
    )
    parser.add_argument(
        "--select",
        default=",".join(CHECKS),
        help=f"comma-separated checks to run (default: all of {','.join(CHECKS)})",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="exit non-zero when anything is reported; without it findings are warnings only",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="print only the summary and findings, not the scanned model count",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = _parse_args(argv if argv is not None else sys.argv[1:])

    selected = {c.strip() for c in args.select.split(",") if c.strip()}
    unknown_checks = selected - set(CHECKS)
    if unknown_checks:
        print(f"Unknown check(s): {', '.join(sorted(unknown_checks))}. Available: {', '.join(CHECKS)}", file=sys.stderr)
        return 2

    packages = tuple(args.packages or DEFAULT_PACKAGES)
    print(f"===== Checking model docstrings in {', '.join(packages)} =====")

    models, import_failures = discover_models(packages)
    for module_name, error in import_failures:
        print(f"  (not checked: {module_name} failed to import: {type(error).__name__}: {error})", file=sys.stderr)

    if not models:
        print(f"No Pydantic models found in {', '.join(packages)}", file=sys.stderr)
        return 1

    findings: list[Finding] = []
    for model in models:
        findings.extend(check_model(model, selected))

    if not args.quiet:
        classes = len({f.location for f in findings})
        print(f"{len(models)} model(s) checked, {len(findings)} warning(s) in {classes} model(s)")

    if findings:
        for line in render(findings):
            print(line)
        print("\nwarnings by check:")
        for check in CHECKS:
            count = sum(1 for f in findings if f.check == check)
            if count:
                print(f"  {check:<8} {count:>4}  {CHECK_EXPLANATIONS[check]}")

    if not findings:
        print("\n===== OK: every documented parameter matches its field =====")
        return 0

    if args.strict:
        print(f"\n===== FAILED: {len(findings)} finding(s) =====", file=sys.stderr)
        return 1

    print(f"\n===== {len(findings)} warning(s), not failing the build (use --strict to fail) =====")
    return 0


if __name__ == "__main__":
    sys.exit(main())
