"""
This package provides models for the communication configuration
in FLYNC.
"""

from .flync_channels import FLYNCChannelConfig
from .flync_communication import FLYNCCommunicationConfig

__all__ = [
    "FLYNCChannelConfig",
    "FLYNCCommunicationConfig",
]
