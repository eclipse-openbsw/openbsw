"""Converter that loads and writes FLYNC models as JSON."""

import json
import logging
import os
from pathlib import Path

from flync.model import FLYNCModel  # type: ignore[import-untyped]

from ..base.base_converter import BaseConverter
from ..registry import hookimpl
from .helpers import pydantic_dump

"""classe for converter between :class:`FLYNCModel` a JSON file."""

logger = logging.getLogger(__name__)


def load_json_files(root_folder):
    """Recursively load all JSON files and merge into a single dict.

    Args:
        root_folder: Root folder path to search for JSON files.

    Returns:
        Merged dictionary from all JSON files found.

    Raises:
        ValueError: If a JSON file does not contain a JSON object.
    """
    combined = {}
    root = Path(root_folder)
    logger.debug("Scanning for JSON files under: %s", root_folder)

    for json_file in root.rglob("*.json"):
        logger.debug("Loading JSON file: %s", json_file)
        with json_file.open("r", encoding="utf-8") as f:
            data = json.load(f)
            if isinstance(data, dict):
                logger.debug("Merging %d keys from %s", len(data), json_file.name)
                combined.update(data)
            else:
                raise ValueError(f"File {json_file} is not a JSON object")

    logger.debug("Finished loading JSON files: %d total keys merged", len(combined))
    return combined


class JsonConverter(BaseConverter):
    """Converter between FLYNCModel and JSON format.

    Reads/writes FLYNCModel instances to/from JSON files in a folder.
    """

    name = "json"

    def can_decode(self):
        """Return True — the JSON converter supports decoding."""
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
        output_path = os.path.join(self.config.config_path, source.__class__.__name__ + ".json")
        logger.debug("Encoding FLYNCModel to JSON at: %s", output_path)
        Path(output_path).parent.mkdir(parents=True, exist_ok=True)
        with open(output_path, "w", encoding="utf-8") as output:
            json.dump(pydantic_dump(source), output, indent=2, sort_keys=False)
        logger.debug("JSON encode complete: %s", output_path)

    def decode(self) -> FLYNCModel:
        """Decode data into a FLYNCBaseModel.

        Returns:
            FLYNCBaseModel: The decoded model.
        """
        if self.config is None:
            raise ValueError("config must be set before decoding")
        logger.debug(
            "Decoding FLYNCModel from JSON path: %s",
            self.config.config_path,
        )
        dict_content = load_json_files(self.config.config_path)
        logger.debug("Validating FLYNCModel from %d keys", len(dict_content))
        model = FLYNCModel.model_validate(dict_content)
        logger.debug("JSON decode complete")
        return model


@hookimpl
def register_converters():
    """Register the JsonConverter with the pluggy plugin manager."""
    return [JsonConverter()]
