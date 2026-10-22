"""Converter for native FLYNC workspace files."""

import logging

from flync.model import FLYNCModel  # type: ignore[import-untyped]
from flync.sdk.helpers.generation_helpers import (  # noqa # type: ignore[import-untyped]
    dump_flync_workspace,
)
from flync.sdk.workspace.flync_workspace import (  # noqa # type: ignore[import-untyped]
    FLYNCWorkspace,
)

from ..base.base_converter import BaseConverter
from ..registry import hookimpl

"""classe for converter between :class:`FLYNCModel` a FLYNC workspace."""

logger = logging.getLogger(__name__)


class FLYNCConverter(BaseConverter):
    """Converter between FLYNCModel and FLYNC workspace format.

    Reads/writes FLYNCModel instances to/from FLYNC workspaces.
    """

    name = "flync"

    def can_decode(self):
        """Return True — the FLYNC workspace converter supports decoding."""
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
        logger.debug(
            "Encoding FLYNCModel to FLYNC workspace at: %s",
            self.config.config_path,
        )
        dump_flync_workspace(source, self.config.config_path, "converted workspace")
        logger.debug("FLYNC encode complete")

    def decode(self) -> FLYNCModel:
        """Decode data into a FLYNCBaseModel.

        Returns:
            FLYNCBaseModel: The decoded model.
        """
        if self.config is None:
            raise ValueError("config must be set before decoding")
        logger.debug("Loading FLYNC workspace from: %s", self.config.config_path)
        ws = FLYNCWorkspace.load_workspace("converted_workspace", self.config.config_path)
        logger.debug("FLYNC workspace loaded, extracting FLYNCModel")
        return ws.flync_model  # type: ignore[return-value]


@hookimpl
def register_converters():
    """Register the FLYNCConverter with the pluggy plugin manager."""
    return [FLYNCConverter()]
