"""Shared helper for deprecated CLI command aliases."""

from flync_cli.utils.console import console


def warn_deprecated(old: str, new: str) -> None:
    """Print a pointer from a hidden deprecated alias ``flync <old>`` to its replacement ``flync <new>``."""
    console.print(f"[yellow]`flync {old}` is deprecated and will be removed in a future release. Use `flync {new}` instead.[/yellow]")
