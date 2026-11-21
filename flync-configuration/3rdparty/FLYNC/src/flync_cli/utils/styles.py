"""
Centralized table styling constants for all ``flync_cli`` commands.

Import from here instead of hard-coding color strings in each command module.  Every
semantic role — ECU name, controller, IP address, etc. — maps to exactly one style so
the output stays visually consistent across all ``info``, ``validate``, and ``errors``
sub-commands.
"""

from rich import box
from rich.table import Table

# ---------------------------------------------------------------------------
# Semantic colour palette
# ---------------------------------------------------------------------------

# Structural / identity columns
STYLE_ECU = "bold cyan"
STYLE_CONTROLLER = STYLE_ECU
STYLE_INTERFACE = "cyan"
STYLE_SWITCH = "cyan"

# Network / addressing columns
STYLE_IP = "bold green"
STYLE_SUBNET = "green"
STYLE_MAC = "magenta"
STYLE_VLAN = "yellow"
STYLE_PORT_NO = "yellow"

# Protocol / service columns
STYLE_SERVICE_NAME = STYLE_ECU
STYLE_SERVICE_ID = "yellow"
STYLE_ROLE = "bold white"
STYLE_PROVIDERS = "green"
STYLE_CONSUMERS = "blue"

# Enumeration / index column
STYLE_INDEX = "dim"

# Validation-result columns
STYLE_ERROR_ID = "red"
STYLE_ERROR_TYPE = "red"
STYLE_ERROR_MSG = "yellow"
STYLE_WARNING_ID = "yellow"
STYLE_WARNING_TYPE = "yellow"
STYLE_WARNING_MSG = "white"
STYLE_LOCATION = "cyan"
STYLE_SOURCE = "green"
STYLE_DETAILS = "magenta"

# ---------------------------------------------------------------------------
# Table factory helpers
# ---------------------------------------------------------------------------

# Default box style used across all rich info tables.
TABLE_BOX = box.SQUARE


def make_table(*args, **kwargs) -> Table:
    """Return a :class:`~rich.table.Table` with project-wide defaults applied.

    ``show_lines`` and ``box`` are set to the project defaults unless the caller
    explicitly overrides them.  All other keyword arguments are forwarded as-is.
    """
    kwargs.setdefault("show_lines", True)
    kwargs.setdefault("box", TABLE_BOX)
    return Table(*args, **kwargs)
