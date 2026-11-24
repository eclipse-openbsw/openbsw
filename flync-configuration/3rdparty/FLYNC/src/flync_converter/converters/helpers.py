"""Helper utilities for Flync converter.

This module provides utility functions for working with Pydantic models
and data serialization.
"""

from pydantic import BaseModel

from flync.sdk.utils.model_dumper import dump_model_with_discriminators


def pydantic_dump(model: BaseModel):
    """Serialize a Pydantic model to a JSON-safe dictionary.

    Uses exclude_unset=True to omit computed/default fields that are populated
    by FLYNC SDK validators during model construction (e.g. switch VLAN
    multicast groups derived from SOME/IP multicast_groups). Omitting these
    lets the decoder recompute them from source data rather than doubling them.
    mode='json' converts all Python types (IPv4Address, enums, ...) to
    primitives.

    Ensures Literal discriminator fields are included even if they weren't
    explicitly set during model construction.

    Args:
        model: A Pydantic BaseModel instance to serialize.

    Returns:
        A dictionary with explicitly-set fields serialized to JSON-compatible
        primitives, with discriminators included.
    """
    return dump_model_with_discriminators(model, mode="json", exclude_unset=True)
