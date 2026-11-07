"""Dynamic Click command: injects converter config options at parse time."""

import click

from flync_converter.utils import get_config_model

from .types import _annotation_to_click_type


class DynamicConverterCommand(click.Command):
    """Click Command that injects per-field config options for converters.

    Before Click's normal argument parsing, this command pre-scans the raw
    argument list for ``-sf``/``--source-format`` and
    ``-of``/``--output-format``,
    looks up each converter's pydantic config model, and injects
    ``--src-<field>`` / ``--dst-<field>`` options so they are available as
    regular Click options (and appear in ``--help``).
    """

    _SRC_PREFIX = "src_"
    _DST_PREFIX = "dst_"

    def _prescan_value(self, args: list, *flags: str):
        """Pre-scan argument list for flag values before Click parses.

        Args:
            args: Raw argument list.
            *flags: Flag strings to search for (e.g., '-sf', '--source-format').

        Returns:
            The value following a flag, or None if not found.
        """
        for i, arg in enumerate(args):
            for flag in flags:
                if arg == flag and i + 1 < len(args):
                    return args[i + 1]
                if arg.startswith(flag + "="):
                    return arg.split("=", 1)[1]
        return None

    def _inject_config_params(self, converter_type: str, prefix: str) -> None:
        """Inject converter-specific config options into Click command params.

        Looks up the converter's pydantic config model and creates a Click
        Option for each field (excluding config_path). Silently skips if the
        model has no fields or lookup fails.

        Args:
            converter_type: Converter key/name as registered.
            prefix: Prefix for option names ('src_' or 'dst_').
        """
        try:
            model = get_config_model(converter_type)
            if not hasattr(model, "model_fields"):
                return
            for name, fld in model.model_fields.items():
                if name == "config_path":
                    continue  # satisfied by --source / --output
                param_name = f"{prefix}{name}"
                if any(p.name == param_name for p in self.params):
                    continue
                required = fld.is_required()
                default = None if required else fld.get_default()
                click_type = _annotation_to_click_type(fld.annotation)
                opt_flag = f"--{prefix.rstrip('_').replace('_', '-')}-{name.replace('_', '-')}"
                self.params.append(
                    click.Option(
                        [opt_flag],
                        type=click_type,
                        default=default,
                        required=False,
                        help=f"{'[required] ' if required else ''}{name} for {converter_type} ({prefix.rstrip('_')} config)",
                        show_default=default is not None,
                    )
                )
        except Exception:
            pass

    def parse_args(self, ctx, args):
        """Pre-scan format flags and inject per-converter config options before Click parses args.

        Args:
            ctx: Click context.
            args: Raw argument list.

        Returns:
            Result of parent parse_args.
        """
        source_format = self._prescan_value(args, "-sf", "--source-format")
        output_format = self._prescan_value(args, "-of", "--output-format") or "flync"
        if source_format:
            self._inject_config_params(source_format, self._SRC_PREFIX)
        self._inject_config_params(output_format, self._DST_PREFIX)
        return super().parse_args(ctx, args)
