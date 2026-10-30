"""Automatic repair of the FLYNC error-number space.

Error numbers are globally unique and never reused, which two branches cannot honor on their
own: both pick the same "next free" number, and the collision only shows up once they meet.
This module resolves that mechanically. It asks git which of the colliding call sites already
existed on the base branch, leaves that one alone, and rewrites the newcomers -- the ones added
by the current branch -- to fresh numbers above everything ever assigned.

Only the ``error_number="NNN"`` literal is touched; the rewrite is a byte splice at the exact
source span reported by :mod:`ast`, so nothing else in the file can move.
"""

import os
import subprocess
from dataclasses import dataclass
from pathlib import Path

from flync_cli.utils.errors import (
    CATALOG_PATH,
    SRC_ROOT,
    ErrorRecord,
    catalog_numbers,
    next_error_number,
    numbers_in_source,
    render_catalog,
    scan_error_calls,
    validate_catalog,
)

REPO_ROOT = SRC_ROOT.parent

# Base-branch candidates, most specific first: an explicit --base wins, then the branch the CI
# system says we are merging into, then the conventional default branch.
_BASE_ENV_VARS = ("CI_MERGE_REQUEST_TARGET_BRANCH_NAME", "GITHUB_BASE_REF", "CHANGE_TARGET")
_BASE_FALLBACKS = ("origin/main", "main", "origin/master", "master")


@dataclass(frozen=True)
class Renumbering(object):
    """One planned rewrite of a single call site's error number."""

    record: ErrorRecord
    new_number: str

    @property
    def old_number(self) -> str:
        return self.record.number or ""

    @property
    def old_error_id(self) -> str:
        return self.record.error_id

    @property
    def new_error_id(self) -> str:
        """The composed id after the rewrite -- only the number segment moves."""

        return f"{self.record.error_id.rsplit('-', 1)[0]}-{self.new_number}"

    def describe(self) -> str:
        """One-line summary for the CLI table."""

        return f"{self.record.file}:{self.record.lineno} ({self.record.location}) {self.old_number} -> {self.new_number}"


@dataclass(frozen=True)
class UnfixableDuplicate(object):
    """A duplicate group the fixer must not resolve on its own."""

    number: str
    records: tuple[ErrorRecord, ...]
    reason: str


@dataclass
class RenumberPlan(object):
    """What the fixer intends to do, and what it had to leave alone."""

    renumberings: list[Renumbering]
    unfixable: list[UnfixableDuplicate]
    base_ref: str | None

    @property
    def empty(self) -> bool:
        return not self.renumberings


def _git(*args: str) -> str | None:
    """Run a read-only git command in the repo, returning stdout or ``None`` if it failed."""

    try:
        result = subprocess.run(("git", *args), cwd=REPO_ROOT, capture_output=True, text=True, check=False)
    except OSError:
        return None
    return result.stdout if result.returncode == 0 else None


def resolve_base_ref(explicit: str | None = None) -> str | None:
    """First base-branch candidate that git can actually resolve, or ``None``."""

    named = [explicit] if explicit else [os.environ.get(var) for var in _BASE_ENV_VARS]
    # A CI variable names a branch, not a ref we necessarily have locally: try the remote copy too.
    candidates = [ref for name in named if name for ref in (name, f"origin/{name}")]
    for ref in [*candidates, *_BASE_FALLBACKS]:
        if _git("rev-parse", "--verify", "--quiet", f"{ref}^{{commit}}"):
            return ref
    return None


def _merge_base(base_ref: str) -> str:
    """Commit where the current branch left the base branch (the base tip itself if unrelated)."""

    out = _git("merge-base", base_ref, "HEAD")
    return out.strip() if out and out.strip() else base_ref


def base_numbers(base_ref: str | None, files: set[str]) -> dict[str, set[str]]:
    """Error numbers per source file as they look on the base branch's merge base.

    Files absent there (added by this branch) map to an empty set, which is exactly what marks
    every number in them as new.
    """

    if base_ref is None:
        return {}
    commit = _merge_base(base_ref)
    return {path: numbers_in_source(_git("show", f"{commit}:{path}") or "") for path in sorted(files)}


def _reserved_numbers(records: list[ErrorRecord], base_ref: str | None, catalog_text: str | None) -> set[str]:
    """Every number that counts as spent but is invisible to the current scan.

    That is the catalog's ids -- retired call sites still documented -- plus the base branch's
    numbers, so a fresh number never collides with one this branch happened to delete or move.
    """

    reserved = catalog_numbers(catalog_text)
    for numbers in base_numbers(base_ref, {r.file for r in records}).values():
        reserved |= numbers
    return reserved


def _keeper(records: list[ErrorRecord], existed_on_base: dict[str, set[str]]) -> ErrorRecord:
    """Which call site of a duplicate group keeps the number.

    The one that already carried it on the base branch -- that is the error the catalog and any
    already-merged test refer to. With no git information, or when the collision predates this
    branch, fall back to file/line order so the outcome stays deterministic.
    """

    on_base = [r for r in records if r.number in existed_on_base.get(r.file, set())]
    return min(on_base or records, key=lambda r: (r.file, r.lineno))


def plan_renumbering(
    records: list[ErrorRecord],
    base_ref: str | None = None,
    catalog_text: str | None = None,
) -> RenumberPlan:
    """Decide, without touching any file, how to resolve every duplicated error number."""

    groups: dict[str, list[ErrorRecord]] = {}
    for record in records:
        if record.number is not None:
            groups.setdefault(record.number, []).append(record)
    duplicates = {number: rs for number, rs in sorted(groups.items()) if len(rs) > 1}

    plan = RenumberPlan(renumberings=[], unfixable=[], base_ref=base_ref)
    if not duplicates:
        return plan

    on_base = base_numbers(base_ref, {r.file for r in records})
    reserved = _reserved_numbers(records, base_ref, catalog_text)
    for number, group in duplicates.items():
        keeper = _keeper(group, on_base)
        losers = sorted((r for r in group if r is not keeper), key=lambda r: (r.file, r.lineno))
        if any(r.number_pos is None for r in losers):
            plan.unfixable.append(UnfixableDuplicate(number, tuple(group), "error_number is not a single-line literal"))
            continue
        for loser in losers:
            fresh = next_error_number(records, reserved)
            reserved.add(fresh)
            plan.renumberings.append(Renumbering(record=loser, new_number=fresh))
    return plan


def _splice(data: bytes, pos: tuple[int, int, int], new_number: str) -> bytes:
    """Replace the string literal at ``pos`` with the same literal holding ``new_number``."""

    lineno, col, end_col = pos
    line_starts = [0]
    for line in data.splitlines(keepends=True):
        line_starts.append(line_starts[-1] + len(line))
    start = line_starts[lineno - 1] + col
    end = line_starts[lineno - 1] + end_col
    quote = data[start : start + 1]
    if quote not in (b'"', b"'"):
        quote = b'"'
    return data[:start] + quote + new_number.encode("utf-8") + quote + data[end:]


def apply_renumbering(plan: RenumberPlan, repo_root: Path | None = None) -> list[Path]:
    """Write every planned renumbering to disk; returns the files that changed."""

    repo_root = repo_root or REPO_ROOT
    by_file: dict[str, list[Renumbering]] = {}
    for item in plan.renumberings:
        by_file.setdefault(item.record.file, []).append(item)

    changed: list[Path] = []
    for rel_path, items in sorted(by_file.items()):
        path = repo_root / rel_path
        data = path.read_bytes()
        # Splice from the bottom of the file upwards so the earlier spans keep their offsets.
        for item in sorted(items, key=lambda i: i.record.number_pos or (0, 0, 0), reverse=True):
            if item.record.number_pos is not None:  # guaranteed by plan_renumbering
                data = _splice(data, item.record.number_pos, item.new_number)
        path.write_bytes(data)
        changed.append(path)
    return changed


# Where full ``FLYNC-...`` ids are quoted outside the raising code: negative tests pin them, and
# prose documents cite them. Generated trees are excluded -- they are rebuilt, not edited.
_REFERENCE_ROOTS = ("tests", "docs/source", "scripts")
_REFERENCE_SUFFIXES = frozenset({".py", ".rst", ".md", ".txt", ".yaml", ".yml", ".json"})
_SKIPPED_DIRS = frozenset({"__pycache__", "build", ".venv", "node_modules", ".git"})


def _child_of(root: Path, path: Path) -> bool:
    """True when ``path`` resolves to a file or directory strictly inside ``root``."""

    try:
        path.resolve().relative_to(root.resolve())
        return True
    except ValueError:
        return False


def _reference_files(repo_root: Path, roots: tuple[str, ...]) -> list[Path]:
    """Text files that may quote an error id, under the given repo-relative roots."""

    found: list[Path] = []
    for root in roots:
        base = repo_root / root
        if not _child_of(repo_root, base) or not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if not _child_of(base, path):
                continue
            if path.suffix in _REFERENCE_SUFFIXES and not _SKIPPED_DIRS.intersection(path.parts):
                found.append(path)
    return found


def propagate_ids(
    plan: RenumberPlan,
    repo_root: Path | None = None,
    roots: tuple[str, ...] = _REFERENCE_ROOTS,
) -> list[Path]:
    """Rewrite every quoted occurrence of a renumbered error id to its new form.

    Without this a renumbering would silently break the negative tests, which pin the exact
    ``FLYNC-...`` id via ``assert_single_error``. New numbers are always above everything ever
    assigned, so no rewrite can land on an id that is already in use.

    The one case this cannot get right is a collision where the two call sites agree on module,
    severity *and* category: their ids are then textually identical, so a reference to the keeper
    is indistinguishable from a reference to the renumbered site. Such rewrites are listed by the
    command so they can be eyeballed.
    """

    replacements = {item.old_error_id: item.new_error_id for item in plan.renumberings}
    if not replacements:
        return []
    repo_root = repo_root or REPO_ROOT

    changed: list[Path] = []
    for pp in _reference_files(repo_root, roots):
        if not _child_of(repo_root, pp) or pp.is_dir():
            continue
        try:
            with pp.open(mode="r", encoding="utf-8") as f:
                text = f.read()
        except (UnicodeDecodeError, OSError):
            continue
        updated = text
        for old_id, new_id in replacements.items():
            updated = updated.replace(old_id, new_id)
        if updated != text:
            with pp.open(mode="w", encoding="utf-8") as f:
                f.write(updated)
            changed.append(pp)
    return changed


@dataclass
class SyncResult(object):
    """Outcome of one ``flync errors sync`` run."""

    plan: RenumberPlan
    changed_sources: list[Path]
    changed_references: list[Path]
    catalog_changed: bool
    blocking: list[str]  # human-readable reasons the catalog could not be generated

    @property
    def ok(self) -> bool:
        return not self.blocking

    @property
    def clean(self) -> bool:
        """True when the tree already agreed with the code and nothing had to change."""

        return self.ok and self.plan.empty and not self.catalog_changed


def sync_catalog(
    base_ref: str | None = None,
    fix_duplicates: bool = True,
    write: bool = True,
    catalog_path: Path | None = None,
) -> SyncResult:
    """Renumber duplicated errors, then bring the catalog back in step with the code.

    Regeneration is what retires orphaned catalog entries: the document is rendered from the
    scan, so anything no longer raised anywhere simply stops being emitted. With ``write=False``
    nothing is touched and the result reports what a real run would change.
    """

    catalog_path = catalog_path or CATALOG_PATH
    catalog_text = catalog_path.read_text(encoding="utf-8") if catalog_path.exists() else None
    records = scan_error_calls()
    plan = plan_renumbering(records, base_ref=base_ref, catalog_text=catalog_text)

    changed_sources: list[Path] = []
    changed_references: list[Path] = []
    if fix_duplicates and plan.renumberings and write:
        changed_sources = apply_renumbering(plan)
        changed_references = propagate_ids(plan)
        records = scan_error_calls()  # re-scan so the catalog reflects the new numbers

    report = validate_catalog(records, None)
    blocking = [f"unnumbered call site at {r.file}:{r.lineno} ({r.location})" for r in report.unnumbered]
    blocking += [f"invalid category {r.bad_category!r} at {r.file}:{r.lineno}" for r in report.invalid_category]
    blocking += [f"duplicate number {d.number} needs a human: {d.reason}" for d in plan.unfixable]
    if not fix_duplicates:
        blocking += [
            f"duplicate number {number}: " + ", ".join(f"{r.file}:{r.lineno}" for r in group) for number, group in report.duplicate_numbers.items()
        ]

    catalog_changed = False
    if not blocking:
        rendered = render_catalog(records)
        catalog_changed = rendered != catalog_text
        if catalog_changed and write:
            catalog_path.write_text(rendered, encoding="utf-8")

    return SyncResult(
        plan=plan,
        changed_sources=changed_sources,
        changed_references=changed_references,
        catalog_changed=catalog_changed,
        blocking=blocking,
    )
