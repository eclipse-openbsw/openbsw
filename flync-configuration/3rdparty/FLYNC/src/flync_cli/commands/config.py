"""``flync config`` command group: manage the session-persisted workspace path."""

from pathlib import Path
from typing import Optional

import typer
from typing_extensions import Annotated

from flync_cli.utils.console import console
from flync_cli.utils.workspace import (
    CONFIG_FILE,
    clear_stored_workspace_path,
    get_stored_workspace_path,
    set_stored_workspace_path,
)

app = typer.Typer(help="Manage the FLYNC CLI session configuration (the default workspace path).")


@app.command(name="set", help="Store a FLYNC workspace path to use when a command omits its path argument.")
def set_path(
    path: Annotated[str, typer.Argument(help="Path to a FLYNC config directory.")],
):
    """Persist *path* as the default workspace for this user, after checking it is a directory."""
    resolved = Path(path).resolve()
    if not resolved.is_dir():
        console.print(f"⚠️ [bold red] Not a directory: {resolved}[/bold red]")
        raise typer.Exit(code=1)

    set_stored_workspace_path(str(resolved))
    console.print(f"[green]Workspace path stored: {resolved}[/green]")
    console.print(f"[dim]Saved to {CONFIG_FILE}[/dim]")


@app.command(name="show", help="Print the stored workspace path.")
def show_path():
    """Print the stored workspace path and where it lives, flagging a path that no longer exists."""
    stored: Optional[str] = get_stored_workspace_path()
    if stored is None:
        console.print("[yellow]No workspace path is stored. Run `flync config set <path>`.[/yellow]")
        return

    note = "" if Path(stored).exists() else "  [bold red](path no longer exists)[/bold red]"
    console.print(f"{stored}{note}")
    console.print(f"[dim]Stored in {CONFIG_FILE}[/dim]")


@app.command(name="clear", help="Forget the stored workspace path.")
def clear_path():
    """Remove the stored workspace path."""
    clear_stored_workspace_path()
    console.print("[green]Stored workspace path cleared.[/green]")
