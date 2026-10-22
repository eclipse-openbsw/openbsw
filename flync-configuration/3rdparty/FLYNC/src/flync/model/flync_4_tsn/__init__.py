"""
This package provides TSN and time synchronization models
for FLYNC.
"""

from .qos import (
    ATSInstance,
    ATSShaper,
    CBSShaper,
    ChildClass,
    DoubleRateThreeColorMarker,
    FrameFilter,
    HTBFilter,
    HTBInstance,
    SingleRateThreeColorMarker,
    SingleRateTwoColorMarker,
    Stream,
    TrafficClass,
)
from .timesync import (
    PTPConfig,
    PTPPdelayConfig,
    PTPPort,
    PTPTimeReceiverConfig,
    PTPTimeTransmitterConfig,
)

KEY = "TSN"
__all__ = [
    "ATSInstance",
    "ATSShaper",
    "CBSShaper",
    "SingleRateTwoColorMarker",
    "SingleRateThreeColorMarker",
    "DoubleRateThreeColorMarker",
    "FrameFilter",
    "Stream",
    "TrafficClass",
    "HTBFilter",
    "ChildClass",
    "HTBInstance",
    "PTPTimeTransmitterConfig",
    "PTPTimeReceiverConfig",
    "PTPPdelayConfig",
    "PTPPort",
    "PTPConfig",
]
