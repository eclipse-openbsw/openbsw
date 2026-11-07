"""Rich-based interactive prompt helpers for converter selection and config."""

import logging
from typing import Type, cast

from pydantic import BaseModel
from rich.console import Console
from rich.panel import Panel
from rich.prompt import Prompt
from rich.table import Table

from flync_converter.base import ConverterConfig
from flync_converter.registry import registry
from flync_converter.utils import cast_value, get_config_model

logger = logging.getLogger(__name__)
console = Console()


def select_converter(step: str) -> str:
    """Present a Rich selection table of available converters.

    Args:
        step: 'source' or 'destination' label used in the UI.

    Returns:
        The selected converter key/name.
    """
    available_converters = list(registry.keys())

    table = Table(title=f"[bold cyan]SELECT {step.upper()} FORMAT[/bold cyan]")
    table.add_column("#", style="cyan", width=3)
    table.add_column("Format", style="magenta")

    for i, conv in enumerate(available_converters, 1):
        table.add_row(str(i), conv)

    console.print(Panel(table, expand=False))

    choice = Prompt.ask(
        "Enter format number or name",
        choices=[str(i) for i in range(1, len(available_converters) + 1)] + available_converters,
    )
    if choice.isdigit():
        return available_converters[int(choice) - 1]
    return choice


def interactive_configure_converter(converter_type: str, step: str) -> ConverterConfig:  # NOSONAR
    """Interactively build a converter config via Rich prompts.

    Inspects the converter registered under `converter_type` to locate a
    pydantic configuration model and prompts for each field. Falls back to
    prompting for common fields if no pydantic model is found.

    Args:
        converter_type: Converter key/name as registered.
        step: Either "source" or "destination" to influence prompts/labels.

    Returns:
        An instantiated configuration object (pydantic model or
        ConverterConfig).
    """
    config_dict = {}

    config_model = get_config_model(converter_type)
    is_pydantic = isinstance(config_model, type) and issubclass(config_model, BaseModel)
    cfg_title = f"[bold cyan]{step.upper()} CONFIGURATION[/bold cyan]: {converter_type}"
    table = Table(title=cfg_title)
    table.add_column("Parameter", style="magenta")
    table.add_column("Value", style="green")

    if is_pydantic:
        pydantic_cls = cast(Type[ConverterConfig], config_model)
        fields = pydantic_cls.model_fields
        for name, fld in fields.items():
            required = fld.is_required()
            default = None if fld.is_required() else fld.get_default()
            prompt_label = f"[{'red' if required else 'yellow'}]{name}[/]"
            if not required and default is not None:
                prompt_label += f" [dim]({default})[/dim]"

            default_str = str(default) if default is not None else None
            raw = Prompt.ask(prompt_label, default=default_str)

            if raw is not None and raw != "":
                ann = fld.annotation
                target_type = ann.__origin__ if ann is not None and hasattr(ann, "__origin__") else ann
                casted = cast_value(raw, target_type)
                config_dict[name] = casted
                table.add_row(name, str(casted))
    else:
        # fallback: prompt for common fields
        common_fields = ("input_path", "output_path", "path")
        for f in common_fields:
            raw = Prompt.ask(f"[yellow]{f}[/]", default=None)
            if raw:
                config_dict[f] = raw
                table.add_row(f, raw)

    console.print(Panel(table, expand=False))

    if is_pydantic:
        return cast(Type[ConverterConfig], config_model)(**config_dict)
    else:
        return ConverterConfig(**config_dict)
