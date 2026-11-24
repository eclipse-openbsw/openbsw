"""Configuration object passed to FLYNC converters."""

from pydantic import BaseModel


class ConverterConfig(BaseModel):
    """Configuration model for converters, holding the path to the config directory."""

    config_path: str
