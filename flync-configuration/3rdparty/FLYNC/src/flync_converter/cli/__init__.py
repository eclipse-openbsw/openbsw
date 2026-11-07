"""CLI, TUI, and GUI entry points for flync_converter.

Provides three modes:
- quick CLI commands for scripted conversions (``convert``, ``list-converters``)
- interactive Textual TUI (``tui`` subcommand, ``-i``/``--interactive`` flag,
  or the ``flync-converter-interactive`` entry point) — requires the ``tui``
  extra (``pip install 'flync[tui]'``)
- PySide6 desktop GUI (``gui`` subcommand, ``--gui`` flag, or the
  ``flync-converter-gui`` entry point) — requires the ``gui`` extra
  (``pip install 'flync[gui]'``)

The module exposes ``main``, ``main_interactive``, and ``main_gui`` entry
points used by packaging.
"""

import click

from . import commands  # noqa: F401 — registers subcommands on cli
from .group import cli


def main():
    """Entry point for the non-interactive CLI."""
    from flync_converter.registry import registry

    registry.load_plugins()
    cli()


def _run_standalone(loader):
    """Run an optional front-end from a bare console-script entry point.

    Click renders ClickException itself only inside ``cli()``; these entry
    points are called directly by the console script, so do it here.
    """
    import sys

    try:
        run = loader()
    except click.ClickException as exc:
        exc.show()
        sys.exit(exc.exit_code)
    run()


def main_interactive():
    """Entry point for the interactive TUI CLI."""
    from flync_converter.cli._optional import load_run_tui

    _run_standalone(load_run_tui)


def main_gui():
    """Entry point for the PySide6 GUI."""
    from flync_converter.cli._optional import load_run_gui

    _run_standalone(load_run_gui)


if __name__ == "__main__":
    main()
