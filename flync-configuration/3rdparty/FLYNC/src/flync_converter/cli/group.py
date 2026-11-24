"""Click group definition and top-level flags for the flync_converter CLI."""

import click


def _open_tui(ctx, _param, value):
    """Click eager callback that launches the Textual TUI when -i/--interactive is passed."""
    if not value or ctx.resilient_parsing:
        return
    from flync_converter.cli._optional import load_run_tui

    load_run_tui()()
    ctx.exit()


def _open_gui(ctx, _param, value):
    """Click eager callback that launches the PySide6 GUI when --gui is passed."""
    if not value or ctx.resilient_parsing:
        return
    from flync_converter.cli._optional import load_run_gui

    load_run_gui()()
    ctx.exit()


@click.group()
@click.option(
    "-i",
    "--interactive",
    is_flag=True,
    is_eager=True,
    expose_value=False,
    callback=_open_tui,
    help="Launch the interactive Textual TUI session (requires flync[tui]).",
)
@click.option(
    "--gui",
    is_flag=True,
    is_eager=True,
    expose_value=False,
    callback=_open_gui,
    help="Launch the PySide6 desktop GUI (requires flync[gui]).",
)
def cli():
    """flync_converter CLI tool."""
