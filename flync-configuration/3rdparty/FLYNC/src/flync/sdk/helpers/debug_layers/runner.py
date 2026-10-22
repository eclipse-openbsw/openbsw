"""Layered FLYNC model debugger.

Runs five validation layers in order and stops at the first layer that
produces hard errors, so the user always sees the most actionable message.

  1  Folder & file structure
  2  YAML syntax
  3  Schema validation  (required / extra fields)
  4  Field value errors
  5  System-wide validation
"""

from __future__ import annotations

from pathlib import Path

from rich import box
from rich.console import Console
from rich.panel import Panel

from flync.model.flync_model import FLYNCModel
from flync.sdk.context.diagnostics_result import DiagnosticsResult

_console = Console(force_terminal=True)


# ---------------------------------------------------------------------------
# Formatting helpers
# ---------------------------------------------------------------------------


def _section(title: str) -> None:
    """Print a section header for one debug layer."""
    _console.print(f"\n[bold cyan]--- {title} ---[/bold cyan]")


def _ok(label: str) -> None:
    """Print a green OK line for a layer that found no issues."""
    _console.print(f"  [green]OK[/green]  {label}")


def _print_issue(
    tag: str,
    tag_color: str,
    message: str,
    path: str = "",
    hint: str = "",
    line: int | None = None,
    field: str = "",
) -> None:
    """Print one issue line, followed by its location and hint if present."""
    loc_parts: list[str] = []
    if path:
        loc_parts.append(path)
    if line:
        if loc_parts:
            loc_parts[-1] += f":{line}"
        else:
            loc_parts.append(f":{line}")
    if field:
        loc_parts.append(f"field: {field}")

    _console.print(f"  [{tag_color}]{tag}[/{tag_color}]  {message}")
    if loc_parts:
        _console.print(f"    [dim]{'  |  '.join(loc_parts)}[/dim]")
    if hint:
        _console.print(f"    [dim italic]-> {hint}[/dim italic]")


def _error(message: str, path: str = "", hint: str = "", line: int | None = None, field: str = "") -> None:
    """Print message as an error line."""
    _print_issue("ERROR  ", "bold red", message, path, hint, line, field)


def _warn(message: str, path: str = "", hint: str = "", line: int | None = None, field: str = "") -> None:
    """Print message as a warning line."""
    _print_issue("WARNING", "bold yellow", message, path, hint, line, field)


# ---------------------------------------------------------------------------
# Public entry point
# ---------------------------------------------------------------------------


def run_debug(dir_path: Path) -> DiagnosticsResult | None:
    """Run all five debug layers against dir_path, print results, and return the workspace result.

    Returns ``None`` when the path does not exist or the run stopped before the workspace could be loaded
    (layer 1 or layer 2 errors), since no :class:`DiagnosticsResult` exists yet in that case.
    """
    if not dir_path.exists():
        _console.print(f"[bold red]Path does not exist:[/bold red] {dir_path}")
        return None

    _console.print(
        Panel(
            f"[bold]FLYNC Model Debug[/bold]\n[dim]{dir_path}[/dim]",
            box=box.ROUNDED,
            expand=False,
        )
    )

    return _run_layers(dir_path)


def _run_pre_workspace_layers(check_structure, check_yaml_syntax, dir_path: Path) -> list | None:
    """Run layers 1-2 (structure & YAML syntax), stopping at the first hard-error layer.

    Returns the layer 1 warnings, or ``None`` if the run should stop before loading the workspace.
    """
    l1_errors, l1_warnings = _report_layer1(check_structure, dir_path)
    if l1_errors:
        _console.print("\n[bold red]Stopped at Layer 1.[/bold red]  Fix the structure issues above before proceeding.")
        _print_summary(l1_errors=len(l1_errors), l1_warnings=len(l1_warnings))
        return None

    yaml_issues = _report_layer2(check_yaml_syntax, dir_path)
    if yaml_issues:
        _console.print("\n[bold red]Stopped at Layer 2.[/bold red]  Fix YAML syntax errors before proceeding.")
        _print_summary(l1_warnings=len(l1_warnings), l2_errors=len(yaml_issues))
        return None

    return l1_warnings


def _run_layers(dir_path: Path) -> DiagnosticsResult | None:
    """Run layers 1-5 in order, stopping and printing a summary at the first hard-error layer."""
    from .layer1_structure import check_structure
    from .layer2_yaml import check_yaml_syntax
    from .layer3_4_5_workspace import run_workspace_validation

    l1_warnings = _run_pre_workspace_layers(check_structure, check_yaml_syntax, dir_path)
    if l1_warnings is None:
        return None

    _console.print("\n[dim]Loading workspace...[/dim]")
    ws_result, ws_issues = run_workspace_validation(dir_path)
    l3 = [i for i in ws_issues if i.layer == 3]
    l4 = [i for i in ws_issues if i.layer == 4]
    l5 = [i for i in ws_issues if i.layer == 5]

    _report_layer3(l3)
    if l3:
        _console.print("\n[bold red]Stopped at Layer 3.[/bold red]  Fix schema errors above before checking constraints and system validation.")
        _print_summary(l1_warnings=len(l1_warnings), l3_errors=len(l3), model_loaded=ws_result.model is not None)
        return None

    _report_layer4(l4)
    _report_layer5(l5)

    _print_summary(
        l1_warnings=len(l1_warnings),
        l3_errors=len(l3),
        l4_errors=len(l4),
        l5_warnings=len(l5),
        model_loaded=ws_result.model is not None,
    )

    return ws_result


# ---------------------------------------------------------------------------
# Per-layer reporting
# ---------------------------------------------------------------------------


def _report_layer1(check_structure, dir_path: Path) -> tuple[list, list]:
    """Run Layer 1 (folder & file structure) and print its findings.

    Returns (errors, warnings) so the caller can decide whether to stop.
    """
    _section("Layer 1 - Folder & File Structure")
    structure_issues = check_structure(FLYNCModel, dir_path, dir_path)
    l1_errors = [i for i in structure_issues if i.severity == "error"]
    l1_warnings = [i for i in structure_issues if i.severity == "warning"]

    if not structure_issues:
        _ok("Folder and file structure looks correct")
    else:
        for issue in l1_errors:
            _error(issue.message, issue.path, issue.hint)
        for issue in l1_warnings:
            _warn(issue.message, issue.path, issue.hint)

    return l1_errors, l1_warnings


def _report_layer2(check_yaml_syntax, dir_path: Path) -> list:
    """Run Layer 2 (YAML syntax) and print its findings. Returns the issues found."""
    _section("Layer 2 - YAML Syntax")
    yaml_issues = check_yaml_syntax(dir_path)

    if not yaml_issues:
        _ok("All .flync.yaml files parse correctly")
    else:
        for yaml_issue in yaml_issues:
            _error(yaml_issue.message, yaml_issue.path, line=yaml_issue.line)

    return yaml_issues


def _report_layer3(l3: list) -> None:
    """Print the Layer 3 (schema: required / extra fields) findings."""
    _section("Layer 3 - Schema Validation  (required / extra fields)")
    if not l3:
        _ok("No missing or extra fields detected")
    else:
        for l3_issue in l3:
            _error(l3_issue.message, l3_issue.path, hint=l3_issue.hint, line=l3_issue.line, field=l3_issue.field)


def _report_layer4(l4: list) -> None:
    """Print the Layer 4 (field value / constraint errors) findings."""
    _section("Layer 4 - Field Value Errors")
    if not l4:
        _ok("No constraint violations detected")
    else:
        for l4_issue in l4:
            _error(l4_issue.message, l4_issue.path, line=l4_issue.line, field=l4_issue.field)


def _report_layer5(l5: list) -> None:
    """Print the Layer 5 (system-wide warnings) findings."""
    _section("Layer 5 - System-Wide Validation")
    if not l5:
        _ok("No system-wide issues detected")
    else:
        for l5_issue in l5:
            _warn(l5_issue.message, l5_issue.path, line=l5_issue.line, field=l5_issue.field)


# ---------------------------------------------------------------------------
# Summary footer
# ---------------------------------------------------------------------------


def _print_summary(
    l1_errors: int = 0,
    l1_warnings: int = 0,
    l2_errors: int = 0,
    l3_errors: int = 0,
    l4_errors: int = 0,
    l5_warnings: int = 0,
    model_loaded: bool = False,
) -> None:
    """Print the final error/warning counts across all layers."""
    total_errors = l1_errors + l2_errors + l3_errors + l4_errors
    total_warnings = l1_warnings + l5_warnings

    _console.print()
    if total_errors == 0 and total_warnings == 0:
        _console.print("[bold green]✓ Model is valid[/bold green]")
        return

    parts: list[str] = []
    if total_errors:
        parts.append(f"[bold red]{total_errors} error(s)[/bold red]")
    if total_warnings:
        parts.append(f"[bold yellow]{total_warnings} warning(s)[/bold yellow]")
    if model_loaded:
        parts.append("[green]model loaded[/green]")

    _console.print("Result: " + "  |  ".join(parts))
