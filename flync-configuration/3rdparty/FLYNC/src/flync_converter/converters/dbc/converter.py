"""The DBC converter class (encode/decode glue)."""

import logging
from pathlib import Path
from typing import Optional

from flync.model import FLYNCModel  # type: ignore[import-untyped]

from ...base.base_converter import BaseConverter
from .dbc_config import DbcConverterConfig
from .decoder import decode_dbc_files
from .encoder import write_dbc_files
from .loading import load_dbc_files

logger = logging.getLogger(__name__)


class DbcConverter(BaseConverter):
    """Converter between FLYNCModel and DBC format.

    Supports both directions: encoding (FLYNC to DBC) and decoding
    (DBC to FLYNC).
    """

    name = "dbc"
    config: Optional[DbcConverterConfig] = None

    def can_decode(self):
        """Return True — the DBC converter supports decoding."""
        return True

    def encode(self, source: FLYNCModel):
        """Encode a FLYNCModel into target representation.

        Args:
            source (FLYNCModel): The model to encode.
        """

        if self.config is None:
            raise ValueError("config must be set before encoding")

        logger.debug("Encoding FLYNCModel to DBC at: %s", self.config.config_path)
        Path(self.config.config_path).mkdir(parents=True, exist_ok=True)

        write_dbc_files(source, self.config.config_path)

        logger.debug("DBC encode complete: %s", self.config.config_path)

    def decode(self) -> FLYNCModel:
        """Decode data into a FLYNCBaseModel.

        Returns:
            FLYNCBaseModel: The decoded model.
        """

        if self.config is None:
            raise ValueError("config must be set before decoding")
        logger.debug(
            "Decoding FLYNCModel from DBC path: %s",
            self.config.config_path,
        )

        dbc_files = load_dbc_files(self.config.config_path)

        model = decode_dbc_files(dbc_files, self.config)
        logger.debug("DBC decode complete")
        return model
