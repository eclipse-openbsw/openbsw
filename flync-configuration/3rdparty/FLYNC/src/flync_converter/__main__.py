"""Entry point for ``python -m flync_converter``; loads converter plugins and runs the CLI."""

from .cli import cli
from .registry import registry

if __name__ == "__main__":

    registry.load_plugins()
    cli()
