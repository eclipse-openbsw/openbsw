"""Provides annotations for fields that are automatically derived rather than manually set."""

from dataclasses import dataclass
from enum import IntEnum


class ImpliedStrategy(IntEnum):
    """
    The strategy on how an implied field will be calculated.
    """

    AUTO = 0
    FOLDER_NAME = AUTO
    FILE_NAME = 1


@dataclass(frozen=True)
class Implied(object):
    """
    Indicates this field is implied instead of loaded/generated.
    """

    strategy: ImpliedStrategy = ImpliedStrategy.AUTO
