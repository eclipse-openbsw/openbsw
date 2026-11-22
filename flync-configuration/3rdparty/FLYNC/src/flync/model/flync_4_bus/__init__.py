"""Top-level package for flync-4-bus."""

from flync.model.flync_4_bus.can_bus import (
    CANBus,
)
from flync.model.flync_4_bus.lin_bus import (
    LINBus,
    LINScheduleEntry,
    LINScheduleTable,
)

KEY = "BUS"
__all__ = [
    # CAN
    "CANBus",
    # LIN
    "LINScheduleEntry",
    "LINScheduleTable",
    "LINBus",
]
