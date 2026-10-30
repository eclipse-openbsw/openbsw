"""Top-level package API for flync_converter.

Provides a simple convert() function and a Converter orchestration class
that use the registry to select and run concrete converters.
"""

import logging
from pathlib import Path

from .base import BaseConverter, ConverterConfig
from .converters import FLYNCConverter, JsonConverter, YamlConverter
from .registry import registry

logger = logging.getLogger(__name__)


def convert(
    source: Path | str,
    destination: Path | str,
    destination_type: str = "flync",
    source_type: str | None = "flync",
    source_config: ConverterConfig | None = None,
    destination_config: ConverterConfig | None = None,
):
    """Convenience function to run a conversion.

    Args:
        source: Path or identifier of the source.
        destination: Path or identifier for the destination.
        destination_type: Destination converter key/name.
        source_type: Optional source converter key/name.
        source_config: Optional configuration for the source converter.
        destination_config: Optional configuration for the destination
            converter.
    """
    Converter().convert(
        source,
        destination,
        source_type=source_type,
        destination_type=destination_type,
        source_config=source_config,
        destination_config=destination_config,
    )


class Converter(object):
    """High-level converter orchestration.

    Uses the registry to obtain converter implementations, wires configurations
    and executes decode/encode steps.
    """

    @staticmethod
    def convert(
        source: Path | str,
        destination: Path | str,
        source_type: str | None = None,
        destination_type: str = "flync",
        source_config: ConverterConfig | None = None,
        destination_config: ConverterConfig | None = None,
    ):
        """Run conversion from source to destination types.

        Args:
            source: Source path or identifier.
            destination: Destination path or identifier.
            source_type: Optional source converter name; if None the
                registry will try to pick.
            destination_type: Destination converter name.
            source_config: Optional source converter configuration instance.
            destination_config: Optional destination converter
                configuration instance.

        Returns:
            None

        Notes:
            This method sets converter.config on registry converter
            instances as a convenience; individual converters are expected
            to use their config when decoding/encoding.
        """
        if source_type is None:
            logger.info("No source type provided. Attempting to auto-detect source type.")
            source_type = registry.pick(source).name
            logger.debug("Auto-detected source type: %s", source_type)
        if source_type == destination_type:
            logger.info("Source and destination types are the same. No conversion needed.")
            return
        logger.info(
            "Converting from %s to %s (source_type=%s, destination_type=%s)",
            source,
            destination,
            source_type,
            destination_type,
        )

        source_converter = registry[source_type]
        logger.debug("Source converter: %s", type(source_converter).__name__)
        source_converter.config = source_config or ConverterConfig(config_path=str(source))
        logger.debug("Source config: %s", source_converter.config)

        destination_converter = registry[destination_type]
        logger.debug("Destination converter: %s", type(destination_converter).__name__)
        destination_converter.config = destination_config or ConverterConfig(config_path=str(destination))
        logger.debug("Destination config: %s", destination_converter.config)

        logger.debug("Starting decode from source")
        source_model = source_converter.decode()
        logger.debug("Decode complete, model type: %s", type(source_model).__name__)

        logger.debug("Starting encode to destination")
        destination_converter.encode(source_model)
        logger.debug("Encode complete")


__all__ = [
    "BaseConverter",
    "YamlConverter",
    "JsonConverter",
    "FLYNCConverter",
    "ConverterConfig",
    "Converter",
    "convert",
]
