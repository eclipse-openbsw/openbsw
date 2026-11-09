"""Static scanner that reconstructs the FLYNC error catalog from source.

Walks the source tree for calls to the ``err_minor`` / ``err_major`` / ``err_fatal`` /
``warn`` factories, reads the ``category`` and ``error_number`` supplied at each call
site, and composes the ``FLYNC-...`` id exactly as the runtime does. This is what the
``flync errors`` commands use to hand out the next free number and to (re)generate the
documentation catalog.
"""

import ast
from collections.abc import Iterable
from dataclasses import dataclass
from pathlib import Path

import flync
from flync.core.utils.exceptions import Category, Severity, compose_error_id, module_code_for

SRC_ROOT = Path(flync.__file__).parents[1]

_SEVERITY_BY_FACTORY = {
    "err_minor": Severity.MIN,
    "err_major": Severity.MAJ,
    "err_fatal": Severity.FAT,
    "warn": Severity.WARN,
}


@dataclass(frozen=True)
class ErrorRecord(object):
    """A single catalog entry reconstructed from one factory call site."""

    error_id: str
    module: str  # module code segment of the id, e.g. "ECU"
    severity: Severity
    category: int  # 0 when uncategorised
    number: str | None  # None when the call site has not been numbered yet
    location: str  # last 2-3 dotted parts: [file.][Class.]function
    message: str  # source text of the first positional argument
    file: str  # source path relative to the repo
    lineno: int
    bad_category: str | None = None  # source text of an invalid category= value, if any
    number_pos: tuple[int, int, int] | None = None  # (lineno, col, end_col) of the error_number literal


class _CallCollector(ast.NodeVisitor):
    """Collects factory call sites within a single parsed module."""

    def __init__(self, dotted_module: str, rel_path: str, targets: dict[str, Severity]):
        self.dotted_module = dotted_module
        self.file_stem = dotted_module.rsplit(".", 1)[-1]
        self.rel_path = rel_path
        self.targets = targets  # local name -> severity
        self.scope: list[tuple[str, str]] = []  # (name, kind)
        self.records: list[ErrorRecord] = []

    def _visit_scoped(self, node, kind: str):
        self.scope.append((node.name, kind))
        self.generic_visit(node)
        self.scope.pop()

    def visit_FunctionDef(self, node):
        self._visit_scoped(node, "func")

    def visit_AsyncFunctionDef(self, node):
        self._visit_scoped(node, "func")

    def visit_ClassDef(self, node):
        self._visit_scoped(node, "class")

    def _location(self) -> str:
        classes = [name for name, kind in self.scope if kind == "class"]
        funcs = [name for name, kind in self.scope if kind == "func"]
        parts = [self.file_stem]
        if classes:
            parts.append(classes[-1])
        if funcs:
            parts.append(funcs[-1])
        return ".".join(parts)

    def visit_Call(self, node):
        severity = self.targets.get(node.func.id) if isinstance(node.func, ast.Name) else None
        if severity is not None:
            self.records.append(self._build_record(node, severity))
        self.generic_visit(node)

    def _build_record(self, node: ast.Call, severity: Severity) -> ErrorRecord:
        category, bad_category = _keyword_category(node)
        number = _keyword_number(node)
        number_pos = _keyword_number_pos(node)
        message = ast.unparse(node.args[0]) if node.args else ""
        module_code = module_code_for(self.dotted_module)
        return ErrorRecord(
            error_id=compose_error_id(severity, module_code, category, number),
            module=module_code,
            severity=severity,
            category=int(category) if category is not None else 0,
            number=number,
            location=self._location(),
            message=message,
            file=self.rel_path,
            lineno=node.lineno,
            bad_category=bad_category,
            number_pos=number_pos,
        )


def _target_names(tree: ast.Module) -> dict[str, Severity]:
    """Local names bound to a factory via ``from ....exceptions import ...``."""

    names: dict[str, Severity] = {}
    for node in ast.walk(tree):
        if isinstance(node, ast.ImportFrom) and node.module and node.module.endswith("exceptions"):
            for alias in node.names:
                if alias.name in _SEVERITY_BY_FACTORY:
                    names[alias.asname or alias.name] = _SEVERITY_BY_FACTORY[alias.name]
    return names


def _keyword_category(node: ast.Call) -> tuple[Category | None, str | None]:
    """Return (category, bad_category): the resolved member, or the source text of an invalid one."""
    for kw in node.keywords:
        if kw.arg != "category":
            continue
        name = kw.value.attr if isinstance(kw.value, ast.Attribute) else getattr(kw.value, "id", None)
        member = Category.__members__.get(name) if isinstance(name, str) else None
        if member is not None:
            return member, None
        return None, ast.unparse(kw.value)
    return None, None


def _keyword_number(node: ast.Call) -> str | None:
    """Resolve keyword arg for error number, return None otherwise"""

    for kw in node.keywords:
        if kw.arg == "error_number" and isinstance(kw.value, ast.Constant):
            return str(kw.value.value)
    return None


def _keyword_number_pos(node: ast.Call) -> tuple[int, int, int] | None:
    """Source span ``(lineno, col_offset, end_col_offset)`` of the ``error_number`` literal.

    Columns are UTF-8 byte offsets into the line, as produced by :mod:`ast`. ``None`` when the
    call site has no literal number, or when the literal spans lines (implicit concatenation) —
    the automatic renumbering refuses to touch those.
    """

    for kw in node.keywords:
        if kw.arg == "error_number" and isinstance(kw.value, ast.Constant):
            if kw.value.end_lineno != kw.value.lineno or kw.value.end_col_offset is None:
                return None
            return (kw.value.lineno, kw.value.col_offset, kw.value.end_col_offset)
    return None


def numbers_in_source(text: str) -> set[str]:
    """Error numbers used by factory call sites in one module's source text.

    Used to read a file as it looks on another git revision, where the module is not importable
    and only the raw numbers matter.
    """

    try:
        tree = ast.parse(text)
    except SyntaxError:
        return set()
    targets = _target_names(tree)
    if not targets:
        return set()
    numbers = set()
    for node in ast.walk(tree):
        if isinstance(node, ast.Call) and isinstance(node.func, ast.Name) and node.func.id in targets:
            number = _keyword_number(node)
            if number is not None:
                numbers.add(number)
    return numbers


def _dotted_module(path: Path) -> str:
    """Resolve relative path to the module with dot as a separator."""

    rel = path.relative_to(SRC_ROOT).with_suffix("")
    parts = [p for p in rel.parts if p != "__init__"]
    return ".".join(parts)


def scan_error_calls(src_root: Path = SRC_ROOT) -> list[ErrorRecord]:
    """Return every factory call site under ``src_root``, ordered by file then line."""

    records: list[ErrorRecord] = []
    for py_file in sorted(src_root.rglob("*.py")):
        tree = ast.parse(py_file.read_text(encoding="utf-8"), filename=str(py_file))
        targets = _target_names(tree)
        if not targets:
            continue
        collector = _CallCollector(_dotted_module(py_file), py_file.relative_to(SRC_ROOT.parent).as_posix(), targets)
        collector.visit(tree)
        records.extend(collector.records)
    records.sort(key=lambda r: (r.file, r.lineno))
    return records


def next_error_number(records: list[ErrorRecord], reserved: Iterable[str] = ()) -> str:
    """Return the next free number: highest ever assigned + 1, never reused.

    A number freed by a deleted error stays a gap on purpose. Reusing it would make one id mean
    two different errors depending on the FLYNC version, which breaks any tool that has to read
    several versions side by side.

    ``reserved`` adds numbers that are not (or no longer) in the scanned code but must still
    count as spent — the ids already in the committed catalog and the numbers on the base
    branch, so a number is never handed out twice across branches.
    """

    assigned = [int(r.number) for r in records if r.number is not None]
    assigned += [int(n) for n in reserved if n.isdigit()]
    return f"{max(assigned, default=0) + 1:03d}"


def catalog_numbers(catalog_text: str | None) -> set[str]:
    """Number segments of every id present in a catalog document."""

    if not catalog_text:
        return set()
    return {error_id.rsplit("-", 1)[-1] for error_id in parse_catalog_ids(catalog_text)}


CATALOG_PATH = Path(flync.__file__).parents[2] / "docs" / "source" / "error_catalog.rst"

_RST_HEADER = """.. Generated by ``flync errors generate-catalog`` — do not edit by hand.

Error Catalog
=============

Every error and warning the FLYNC validators can raise, identified as
``FLYNC-<MODULE>-<SEVERITY>-<CATEGORY>-<NUMBER>``.

.. needtable::
   :types: err
   :columns: id;title;module;severity;category;location
   :style: ssp-tiny

"""

_ID_OPTION = ":id:"


def _title(message: str) -> str:
    """Sanitize error message."""

    text = message.strip().lstrip("frbFRB").strip("'\"").replace("`", "'").replace("\n", " ")
    return (text[:77] + "...") if len(text) > 80 else text or "(no message)"


def _category_name(category: int) -> str:
    """Get category name from it's number or default category else"""

    return Category(category).name if category else "UNCATEGORISED"


def _catalog_sort_key(record: ErrorRecord) -> tuple[int, int, str, int]:
    """Order entries by error number ascending; unnumbered call sites last, by file then line.

    Numbers are globally unique and never reused, so this is a total order that keeps the
    generated document diff-stable: a newly added error appends at the end instead of being
    inserted in the middle.
    """

    if record.number is None:
        return (1, 0, record.file, record.lineno)
    return (0, int(record.number), record.file, record.lineno)


def render_catalog(records: list[ErrorRecord]) -> str:
    """Render the full ``error_catalog.rst`` document for the given records."""

    blocks = [_RST_HEADER]
    for r in sorted(records, key=_catalog_sort_key):
        blocks.append(
            f".. err:: {_title(r.message)}\n"
            f"   :id: {r.error_id}\n"
            f"   :module: {r.module}\n"
            f"   :severity: {r.severity.value}\n"
            f"   :category: {_category_name(r.category)}\n"
            f"   :number: {r.number or '000'}\n"
            f"   :location: {r.location}\n\n"
            f"   {r.message}\n"
        )
    return "\n".join(blocks)


def parse_catalog_ids(text: str) -> list[str]:
    """Extract the ``:id:`` values already present in a catalog document."""

    return [line.split(_ID_OPTION, 1)[1].strip() for line in text.splitlines() if _ID_OPTION in line]


@dataclass
class CatalogReport(object):
    """Outcome of comparing the source of truth (code) against the catalog."""

    unnumbered: list[ErrorRecord]
    uncategorised: list[ErrorRecord]
    invalid_category: list[ErrorRecord]  # category= names that are not Category members
    duplicate_numbers: dict[str, list[ErrorRecord]]
    missing_from_catalog: list[str]  # ids in code, absent from the .rst
    orphaned_in_catalog: list[str]  # ids in the .rst, absent from code

    @property
    def ok(self) -> bool:
        return not any(
            (
                self.unnumbered,
                self.uncategorised,
                self.invalid_category,
                self.duplicate_numbers,
                self.missing_from_catalog,
                self.orphaned_in_catalog,
            )
        )


def validate_catalog(records: list[ErrorRecord], catalog_text: str | None) -> CatalogReport:
    """Diff the reconstructed catalog against the committed ``.rst`` and code hygiene."""

    numbers: dict[str, list[ErrorRecord]] = {}
    for r in records:
        if r.number is not None:
            numbers.setdefault(r.number, []).append(r)

    catalog_ids = set(parse_catalog_ids(catalog_text)) if catalog_text is not None else set()
    code_ids = {r.error_id for r in records if r.number is not None}
    return CatalogReport(
        unnumbered=[r for r in records if r.number is None],
        uncategorised=[r for r in records if r.category == 0 and r.bad_category is None],
        invalid_category=[r for r in records if r.bad_category is not None],
        duplicate_numbers={num: rs for num, rs in numbers.items() if len(rs) > 1},
        missing_from_catalog=sorted(code_ids - catalog_ids),
        orphaned_in_catalog=sorted(catalog_ids - code_ids),
    )
