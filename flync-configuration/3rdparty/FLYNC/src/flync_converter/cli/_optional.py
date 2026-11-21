"""Loaders for the optional UI front-ends (Textual TUI, PySide6 GUI).

``textual`` and ``PySide6`` are optional dependencies (see the ``tui`` and
``gui`` extras). Everything else in flync_converter works without them.

Each loader probes for the third-party module with ``find_spec`` before
importing the front-end package, so that a genuinely missing extra produces
an actionable message while a real ImportError raised *inside* the front-end
code still surfaces as itself.
"""

from __future__ import annotations

from importlib.util import find_spec
from typing import Callable

import click


def _require(module: str, extra: str, what: str) -> None:
    """Raise a ClickException if *module* is not installed."""
    if find_spec(module) is not None:
        return
    raise click.ClickException(
        f"{what} requires '{module}', which is an optional dependency of flync.\n"
        f"  Install it with:  pip install 'flync[{extra}]'\n"
        f"  From a checkout:  uv sync --extra {extra}"
    )


def load_run_tui() -> Callable[[], None]:
    """Import and return ``run_tui``, or raise a ClickException with install hints."""
    _require("textual", "tui", "The interactive TUI")
    from flync_converter.cli.tui import run_tui

    return run_tui


def load_run_gui() -> Callable[[], None]:
    """Import and return ``run_gui``, or raise a ClickException with install hints."""
    _require("PySide6", "gui", "The desktop GUI")
    from flync_converter.cli.gui import run_gui

    return run_gui
