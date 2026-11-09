"""Session-persisted workspace path (``flync config``) and workspace loading shared by every command."""

import json
from pathlib import Path
from typing import Optional

import platformdirs
import typer
from typing_extensions import Annotated

from flync.sdk.helpers.validation_helpers import validate_workspace
from flync.sdk.workspace.flync_workspace import FLYNCWorkspace
from flync_cli.utils.console import console

CONFIG_DIR = Path(platformdirs.user_config_dir("FLYNC"))
CONFIG_FILE = CONFIG_DIR / "cli.json"

WorkspacePathArg = Annotated[
    Optional[str],
    typer.Argument(help="Path to the FLYNC config directory. Defaults to the path stored with `flync config set`."),
]


def _read_config() -> dict:
    """Return the persisted CLI configuration, or an empty dict if none exists or it cannot be parsed."""
    if not CONFIG_FILE.exists():
        return {}
    try:
        return json.loads(CONFIG_FILE.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return {}


def _write_config(data: dict) -> None:
    """Save a workspace config to a txt-file."""
    CONFIG_DIR.mkdir(parents=True, exist_ok=True)
    CONFIG_FILE.write_text(json.dumps(data, indent=2), encoding="utf-8")


def get_stored_workspace_path() -> Optional[str]:
    """Return the workspace path stored by ``flync config set``, or ``None`` if none is stored."""
    return _read_config().get("workspace_path")


def set_stored_workspace_path(path: str) -> None:
    """Persist *path* as the workspace path used when a command omits its ``path`` argument."""
    data = _read_config()
    data["workspace_path"] = path
    _write_config(data)


def clear_stored_workspace_path() -> None:
    """Forget the stored workspace path."""
    data = _read_config()
    data.pop("workspace_path", None)
    _write_config(data)


def resolve_workspace_path(path: Optional[str]) -> Path:
    """
    Resolve the workspace path a command should use.

    An explicit *path* always wins; otherwise fall back to the path stored via ``flync config set``. Exits 1 with a
    clear message when neither is available or the resolved path does not exist.
    """

    raw = path or get_stored_workspace_path()
    if raw is None:
        console.print("⚠️ [bold red] No path given and no workspace configured. Pass a path or run `flync config set <path>`.[/bold red]")
        raise typer.Exit(code=1)

    resolved = Path(raw).resolve()
    if not resolved.exists():
        console.print(f"⚠️ [bold red] Path does not exist: {resolved}[/bold red]")
        raise typer.Exit(code=1)

    return resolved


def load_workspace(path: Optional[str]) -> FLYNCWorkspace:
    """Resolve *path* (or the stored one), validate the workspace there, and return it - or exit 1 with a message."""
    resolved = resolve_workspace_path(path)
    result = validate_workspace(resolved)

    if result.workspace is None:
        console.print("⚠️ [bold red] Validate your model first with `flync validate`.[/bold red]")
        raise typer.Exit(code=1)

    return result.workspace
