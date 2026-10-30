"""Assembles the FLYNC Typer application and its sub-commands."""

from importlib import metadata
from typing import Optional

import typer
from rich import print as rprint
from typing_extensions import Annotated

from flync_cli.commands.config import app as config_app
from flync_cli.commands.errors import app as errors_app
from flync_cli.commands.filetree import _show_filetree
from flync_cli.commands.filetree import app as filetree_app
from flync_cli.commands.generate_system_uml import app as generate_uml
from flync_cli.commands.info import EcuNameOpt, _resolve_service_by_name, _show_instances, _show_vlans
from flync_cli.commands.info import app as info_app
from flync_cli.commands.validate import _run_validate
from flync_cli.commands.validate import app as validate_app
from flync_cli.utils.deprecation import warn_deprecated
from flync_cli.utils.workspace import WorkspacePathArg, load_workspace

app = typer.Typer(
    help="FLYNC CLI tool for validating the model, visually displaying the relevant information and generating system UML diagrams",
    add_completion=True,
    no_args_is_help=True,
)

app.add_typer(validate_app)
app.add_typer(info_app, name="info")
app.add_typer(filetree_app)
app.add_typer(generate_uml)
app.add_typer(config_app, name="config")
app.add_typer(errors_app, name="errors")


@app.callback(invoke_without_command=True)
def main(
    ctx: typer.Context,
    version: bool = typer.Option(
        False,
        "--version",
        help="Show the installed version of FLYNC.",
    ),
):
    """Handle global options like --version."""
    if version and ctx.invoked_subcommand is None:
        version_str = metadata.version("flync")
        rprint(f"[green]Version: {version_str}[/green]")
        raise typer.Exit(code=0)


# ---------------------------------------------------------------------------
# Deprecated top-level aliases (hidden from --help)
# ---------------------------------------------------------------------------


@app.command(name="display-vlan-info", hidden=True, deprecated=True)
def _display_vlan_info_alias(
    vlan_id: Annotated[int, typer.Argument(help="VLAN ID for which the information needs to be displayed")],
    path: WorkspacePathArg = None,
    ecu_name: EcuNameOpt = None,
):
    """Deprecated alias for ``flync info vlans``."""
    warn_deprecated("display-vlan-info", "info vlans --vlan-id")
    ws = load_workspace(path)
    _show_vlans(ws.flync_model, ecu_name, vlan_id)


@app.command(name="display-service-info", hidden=True, deprecated=True)
def _display_service_info_alias(
    service: Annotated[str, typer.Argument(help="Service for which the information needs to be displayed")],
    path: WorkspacePathArg = None,
):
    """Deprecated alias for ``flync info instances``."""
    warn_deprecated("display-service-info", "info instances")
    ws = load_workspace(path)
    service_id, major_version = _resolve_service_by_name(ws.flync_model, service)
    _show_instances(ws.flync_model, service_id, major_version)


@app.command(name="display-repo-structure", hidden=True, deprecated=True)
def _display_repo_structure_alias(
    cls_name: Annotated[Optional[str], typer.Option("--class")] = None,
):
    """Deprecated alias for ``flync filetree``."""
    warn_deprecated("display-repo-structure", "filetree")
    _show_filetree(cls_name)


@app.command(name="debug", hidden=True, deprecated=True)
def _debug_alias(
    dir_path: WorkspacePathArg = None,
):
    """Deprecated alias for ``flync validate --verbose``."""
    warn_deprecated("debug", "validate --verbose")
    _run_validate(dir_path, "", "flync_config", True)


if __name__ == "__main__":
    app()
