"""Converter that loads and writes FLYNC models as YAML."""

import logging
import os
from pathlib import Path

import yaml

from flync.model import FLYNCModel  # type: ignore[import-untyped]

from ..base.base_converter import BaseConverter
from ..registry import hookimpl
from .helpers import pydantic_dump

"""classe for converter between :class:`FLYNCModel` a YAML file."""

logger = logging.getLogger(__name__)


def load_yaml_files(root_folder):
    """Recursively load all YAML files and merge into a single dict.

    Args:
        root_folder: Root folder path to search for YAML files.

    Returns:
        Merged dictionary from all YAML files found.

    Raises:
        ValueError: If a YAML file does not contain a YAML object.
    """
    combined = {}
    root = Path(root_folder)
    logger.debug("Scanning for YAML files under: %s", root_folder)

    for yaml_file in root.rglob("*.yaml"):
        logger.debug("Loading YAML file: %s", yaml_file)
        with yaml_file.open("r", encoding="utf-8") as f:
            data = yaml.safe_load(f)
            if isinstance(data, dict):
                logger.debug("Merging %d keys from %s", len(data), yaml_file.name)
                combined.update(data)
            else:
                raise ValueError(f"File {yaml_file} is not a JSON object")

    logger.debug("Finished loading YAML files: %d total keys merged", len(combined))
    return combined


class YamlConverter(BaseConverter):
    """Converter between FLYNCModel and YAML format.

    Reads/writes FLYNCModel instances to/from YAML files in a folder.
    """

    name = "yaml"

    def can_decode(self):
        """Return True — the YAML converter supports decoding."""
        return True

    def encode(self, source: FLYNCModel):
        """Encode a FLYNCModel into target representation.

        Args:
            source (FLYNCModel): The model to encode.

        Returns:
            Any: The encoded representation.
        """
        if self.config is None:
            raise ValueError("config must be set before encoding")
        output_path = os.path.join(self.config.config_path, source.__class__.__name__ + ".yaml")
        logger.debug("Encoding FLYNCModel to YAML at: %s", output_path)
        Path(output_path).parent.mkdir(parents=True, exist_ok=True)
        with open(output_path, "w", encoding="utf-8") as output:
            yaml.safe_dump(pydantic_dump(source), output, indent=2, sort_keys=False)
        logger.debug("YAML encode complete: %s", output_path)

    def decode(self) -> FLYNCModel:
        """Decode data into a FLYNCModel.

        Returns:
            FLYNCModel: The decoded model.
        """
        if self.config is None:
            raise ValueError("config must be set before decoding")
        logger.debug(
            "Decoding FLYNCModel from YAML path: %s",
            self.config.config_path,
        )
        dict_content = load_yaml_files(self.config.config_path)
        logger.debug("Validating FLYNCModel from %d keys", len(dict_content))
        model = FLYNCModel.model_validate(dict_content)
        logger.debug("YAML decode complete")
        return model


@hookimpl
def register_converters():
    """Register the YamlConverter with the pluggy plugin manager."""
    return [YamlConverter()]
