"""``flync errors`` — helper commands for the error catalog."""

from typing import Annotated

import typer
from rich.table import Table

from flync_cli.utils.console import console
from flync_cli.utils.error_renumber import apply_renumbering, plan_renumbering, propagate_ids, resolve_base_ref, sync_catalog
from flync_cli.utils.errors import (
    CATALOG_PATH,
    CatalogReport,
    next_error_number,
    render_catalog,
    scan_error_calls,
    validate_catalog,
)

app = typer.Typer(help="Inspect and maintain the FLYNC error catalog.")

_ALL_ERRORS_KINDS = {
    "unnumbered",
    "invalid_category",
    "uncategorised",
    "duplicate_numbers",
    "missing_from_catalog",
    "orphaned_in_catalog",
}
_GENERATE_BLOCKING_KINDS = {
    "unnumbered",
    "duplicate_numbers",
    "invalid_category",
}


def _drift_rows(report: CatalogReport) -> dict[str, list[tuple[str, str]]]:
    """Pre-compute ``(issue label, detail)`` rows for every drift kind."""
    return {
        "unnumbered": [("unnumbered", f"{r.file}:{r.lineno} ({r.location})") for r in report.unnumbered],
        "invalid_category": [("invalid category", f"{r.bad_category!r} at {r.file}:{r.lineno} ({r.location})") for r in report.invalid_category],
        "uncategorised": [("uncategorised", f"{r.error_id} — {r.file}:{r.lineno}") for r in report.uncategorised],
        "duplicate_numbers": [
            ("duplicate number", f"{number}: " + ", ".join(f"{r.file}:{r.lineno}" for r in rs)) for number, rs in report.duplicate_numbers.items()
        ],
        "missing_from_catalog": [("missing from catalog", e) for e in report.missing_from_catalog],
        "orphaned_in_catalog": [("orphaned in catalog", e) for e in report.orphaned_in_catalog],
    }


def _drift_table(report: CatalogReport, kinds: set[str] | None = None) -> Table:
    """Render a table of the requested catalog drift issues (default: all kinds)."""
    rows = _drift_rows(report)
    if kinds is None:
        kinds = _ALL_ERRORS_KINDS
    table = Table(show_lines=True, title="Catalog drift")
    table.add_column("Issue", style="red")
    table.add_column("Detail", style="yellow", overflow="fold")
    for kind in kinds:
        for label, detail in rows.get(kind, []):
            table.add_row(label, detail)
    return table


@app.command(name="get-next-number", help="Print the next free globally-unique error number.")
def get_next_number():
    """Print the next free error number (highest assigned + 1, never reused)."""
    console.print(next_error_number(scan_error_calls()))


@app.command(name="validate-catalog", help="Check that the catalog matches the code (source of truth).")
def validate():
    """Report drift between the code and the committed ``error_catalog.rst``."""
    records = scan_error_calls()
    text = CATALOG_PATH.read_text(encoding="utf-8") if CATALOG_PATH.exists() else None
    report = validate_catalog(records, text)

    if report.ok:
        console.print(f"[green]Catalog is in sync with {len(records)} error call sites.[/green]")
        return

    console.print(_drift_table(report))
    raise typer.Exit(code=1)


@app.command(name="generate-catalog", help="(Re)generate docs/source/error_catalog.rst from the code.")
def generate():
    """Write ``error_catalog.rst`` from the current source; requires all call sites numbered."""
    records = scan_error_calls()
    report = validate_catalog(records, None)
    if report.unnumbered or report.duplicate_numbers or report.invalid_category:
        console.print("[red]Cannot generate: fix the drift listed below first, then re-run.[/red]")
        console.print(_drift_table(report, kinds=_GENERATE_BLOCKING_KINDS))
        raise typer.Exit(code=1)

    CATALOG_PATH.write_text(render_catalog(records), encoding="utf-8")
    console.print(f"[green]Wrote {len(records)} entries to {CATALOG_PATH}.[/green]")


BASE_OPTION = Annotated[
    str | None,
    typer.Option("--base", help="Base branch to diff against when deciding which duplicate keeps its number."),
]
DRY_RUN_OPTION = Annotated[bool, typer.Option("--dry-run", help="Report what would change without writing anything.")]


def _report_blocking(reasons: list[str]) -> None:
    """Print the reasons the catalog could not be regenerated."""

    table = Table(show_lines=True, title="Needs a human")
    table.add_column("Reason", style="red", overflow="fold")
    for reason in reasons:
        table.add_row(reason)
    console.print(table)


@app.command(name="fix-numbers", help="Renumber duplicated error numbers, keeping the one already on the base branch.")
def fix_numbers(base: BASE_OPTION = None, dry_run: DRY_RUN_OPTION = False):
    """Resolve duplicate ``error_number`` collisions introduced by the current branch."""

    base_ref = resolve_base_ref(base)
    records = scan_error_calls()
    text = CATALOG_PATH.read_text(encoding="utf-8") if CATALOG_PATH.exists() else None
    plan = plan_renumbering(records, base_ref=base_ref, catalog_text=text)

    if base_ref is None:
        console.print("[yellow]No base branch resolved - falling back to file order to pick the keeper.[/yellow]")
    if plan.empty and not plan.unfixable:
        console.print("[green]No duplicated error numbers.[/green]")
        return

    for item in plan.renumberings:
        console.print(f"  {item.describe()}")
    if plan.unfixable:
        _report_blocking([f"duplicate number {d.number} needs a human: {d.reason}" for d in plan.unfixable])

    if dry_run:
        console.print(f"[yellow]--dry-run: {len(plan.renumberings)} call site(s) would be renumbered.[/yellow]")
    elif plan.renumberings:
        changed = apply_renumbering(plan)
        references = propagate_ids(plan)
        console.print(f"[green]Renumbered {len(plan.renumberings)} call site(s) in {len(changed)} file(s).[/green]")
        for path in references:
            console.print(f"  updated id reference in {path}")
        console.print("[yellow]Run `flync errors generate-catalog` (or `errors sync`) to refresh the catalog.[/yellow]")
    if plan.unfixable:
        raise typer.Exit(code=1)


@app.command(name="sync", help="Fix duplicate numbers and (re)generate the catalog")
def sync(
    base: BASE_OPTION = None,
    check: Annotated[bool, typer.Option("--check", help="Write nothing; exit 1 if anything would change.")] = False,
    fix_duplicates: Annotated[bool, typer.Option("--fix-duplicates/--no-fix-duplicates", help="Renumber duplicates before generating.")] = True,
):
    """Bring sources and ``error_catalog.rst`` into a self-consistent state in one step.

    Duplicated numbers are reassigned, the catalog is rendered fresh from the code (which drops
    orphaned entries), and anything that genuinely needs a decision exits non-zero.
    """

    base_ref = resolve_base_ref(base)
    result = sync_catalog(base_ref=base_ref, fix_duplicates=fix_duplicates, write=not check)

    for item in result.plan.renumberings:
        console.print(f"  renumber {item.describe()}")
    if not result.ok:
        _report_blocking(result.blocking)
        raise typer.Exit(code=1)

    if result.clean:
        console.print("[green]Catalog and error numbers are already in sync.[/green]")
        return

    verb = "would be" if check else "were"
    if result.plan.renumberings:
        console.print(f"[yellow]{len(result.plan.renumberings)} error number(s) {verb} reassigned.[/yellow]")
    for path in result.changed_references:
        console.print(f"  updated id reference in {path}")
    if result.catalog_changed:
        console.print(f"[yellow]{CATALOG_PATH} {verb} regenerated.[/yellow]")
    if check:
        console.print("[red]--check: run `flync errors sync` and commit the result.[/red]")
        raise typer.Exit(code=1)
