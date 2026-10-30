"""Base converter abstractions.

This module defines the abstract BaseConverter class used by concrete
converters to encode/decode between FLYNCModel and other representations.

The BaseConverter methods are documented in Google docstring style so that
IDE help and generated docs show parameter and return contracts clearly.
"""

from abc import ABC, abstractmethod
from typing import Optional

from flync.model import FLYNCModel  # type: ignore[import-untyped]

from .converter_config import ConverterConfig

"""Base classes for converters between :class:`FLYNCModel` and other
representations.

Provides abstract base class :class:`BaseConverter` defining the required
``encode`` and ``decode`` methods for concrete converter implementations.
"""


class BaseConverter(ABC):
    """Abstract base class defining the interface for converters.

    Converters convert between FLYNCModel instances and other representations.
    Concrete implementations must implement decode/encode and indicate whether
    they can decode a given source.

    Attributes:
        config (Optional[ConverterConfig]): Optional configuration for the
            converter.
    """

    name: str = ""

    def __init__(self, config: Optional[ConverterConfig] = None):
        """Initializes the converter.

        Args:
            config: Optional converter-specific configuration.
        """
        self.config = config

    @abstractmethod
    def can_decode(self) -> bool:
        """Determine whether this converter can decode the configured source.

        Returns:
            bool: True if decoding is possible from the configured location.
        """

    @abstractmethod
    def encode(self, source: FLYNCModel):
        """Encode a FLYNCModel into the target representation.

        Args:
            source: The FLYNCModel instance to encode.

        Raises:
            ConverterError: If encoding fails (implementation specific).
        """

    @abstractmethod
    def decode(self) -> FLYNCModel:
        """Decode the configured source into a FLYNCModel.

        Returns:
            FLYNCModel: The decoded model.

        Raises:
            ConverterError: If decoding fails (implementation specific).
        """
