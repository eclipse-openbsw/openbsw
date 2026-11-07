"""External field annotations and output strategies."""

from dataclasses import dataclass
from enum import IntEnum, IntFlag


class NamingStrategy(IntEnum):
    """
    The strategy on how the external field file/folder will be named.
    """

    AUTO = 0
    FIELD_NAME = AUTO
    FIXED_PATH = 1


class OutputStrategy(IntFlag):
    """
    The strategy on how an external field will be generated.
    """

    AUTO = 1
    FOLDER = AUTO
    SINGLE_FILE = 2
    OMMIT_ROOT = 4
    FIXED_ROOT = 8


@dataclass(frozen=True)
class External(object):
    """
    Indicates this field is loaded from a separate location.
    """

    path: str | None = None
    root: str | None = None
    output_structure: OutputStrategy = OutputStrategy.AUTO
    naming_strategy: NamingStrategy = NamingStrategy.AUTO
