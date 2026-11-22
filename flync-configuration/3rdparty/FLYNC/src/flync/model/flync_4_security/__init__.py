"""
This package provides security models for FLYNC.
"""

from .firewall import Firewall, FirewallRule
from .macsec import (
    CipherSuiteBaseModel,
    IntegrityWithConfidentiality,
    IntegrityWithoutConfidentiality,
    MACsecConfig,
)

KEY = "SEC"
__all__ = [
    "CipherSuiteBaseModel",
    "Firewall",
    "FirewallRule",
    "IntegrityWithConfidentiality",
    "IntegrityWithoutConfidentiality",
    "MACsecConfig",
]
