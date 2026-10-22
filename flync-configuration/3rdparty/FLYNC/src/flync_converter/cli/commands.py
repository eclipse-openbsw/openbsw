"""Click subcommands for the flync_converter CLI."""

import logging

import click
from rich.console import Console
from rich.panel import Panel
from rich.table import Table

from flync_converter import Converter
from flync_converter.registry import registry
from flync_converter.utils import get_config_model

from .dynamic import DynamicConverterCommand
from .group import cli
from .interactive import interactive_configure_converter, select_converter

logger = logging.getLogger(__name__)
console = Console()


@cli.command()
def list_converters():
    """List all registered converters with descriptions."""
    available_converters = list(registry.keys())

    if not available_converters:
        console.print("[bold yellow]No converters registered.[/bold yellow]")
        return

    table = Table(title="[bold cyan]Registered Converters[/bold cyan]")
    table.add_column("Converter Type", style="magenta", width=20)
    table.add_column("Description", style="green")

    for converter_type in available_converters:
        try:
            converter_class = registry[converter_type]
            description = (converter_class.__doc__ or "No description available").strip().split("\n")[0]
        except Exception as e:
            logger.warning(f"Failed to get description for {converter_type}: {e}")
            description = "No description available"

        table.add_row(converter_type, description)

    console.print(Panel(table, expand=False))


@cli.command()
def convert_interactive():
    """Run the interactive session to configure and execute a conversion.

    This command:
        1. Lets the user select and configure a source converter.
        2. Lets the user select and configure a destination converter.
        3. Executes the conversion and reports status.
    """
    console.print(
        Panel(
            "[bold cyan]FLYNC Converter Interactive Session[/bold cyan]",
            expand=True,
        )
    )

    console.print("\n[bold]Step 1: Configure Source[/bold]")
    source_type = select_converter("source")
    source_config = interactive_configure_converter(source_type, "source")

    console.print("\n[bold]Step 2: Configure Destination[/bold]")
    destination_type = select_converter("destination")
    destination_config = interactive_configure_converter(destination_type, "destination")

    console.print("\n[bold]Step 3: Starting Conversion[/bold]")
    try:
        converter = Converter()
        with console.status("[bold green]Converting..."):
            converter.convert(
                source=source_config.config_path,
                destination=destination_config.config_path,
                source_type=source_type,
                destination_type=destination_type,
                source_config=source_config,
                destination_config=destination_config,
            )
        console.print("[bold green]✓ Conversion completed successfully![/bold green]")
    except Exception as e:
        logger.exception("Conversion failed: %s", e)
        console.print(f"[bold red]✗ Conversion failed: {e}[/bold red]")


@cli.command(cls=DynamicConverterCommand)
@click.option(
    "-s",
    "--source",
    prompt="source location",
    help="Source location to convert from.",
)
@click.option(
    "-o",
    "--output",
    prompt="output location",
    help="output location to convert to.",
)
@click.option("-sf", "--source-format", help="Source format type.", default="flync")
@click.option("-of", "--output-format", help="Output format type.", default="flync")
@click.pass_context
def convert(ctx, source, output, source_format, output_format, **kwargs):
    """Quick command to convert a single source to a destination.

    Any config fields for the selected converter formats are exposed as
    ``--src-<field>`` and ``--dst-<field>`` options and appear in ``--help``
    once ``--source-format`` / ``--output-format`` are known.

    Args:
        source: Source folder path.
        output: Output folder path.
        source_format: Source format (default: flync).
        output_format: Destination format (default: flync).
    """
    if source_format == output_format:
        click.echo("Source and output formats are the same. No conversion needed.")
        return

    click.echo(f"Converting from {source} to {output} with source format {source_format} and output format {output_format}!")

    src_fields = {
        k[len(DynamicConverterCommand._SRC_PREFIX) :]: v
        for k, v in ctx.params.items()
        if k.startswith(DynamicConverterCommand._SRC_PREFIX) and v is not None
    }
    dst_fields = {
        k[len(DynamicConverterCommand._DST_PREFIX) :]: v
        for k, v in ctx.params.items()
        if k.startswith(DynamicConverterCommand._DST_PREFIX) and v is not None
    }

    source_config = get_config_model(source_format)(config_path=source, **src_fields)
    destination_config = get_config_model(output_format)(config_path=output, **dst_fields)

    from flync_converter import convert as convert_func

    convert_func(
        source,
        output,
        output_format,
        source_format,
        source_config=source_config,
        destination_config=destination_config,
    )


@cli.command()
def tui():
    """Launch the full interactive Textual TUI (requires flync[tui])."""
    from flync_converter.cli._optional import load_run_tui

    load_run_tui()()


@cli.command()
def gui():
    """Launch the PySide6 desktop GUI (requires flync[gui])."""
    from flync_converter.cli._optional import load_run_gui

    load_run_gui()()
