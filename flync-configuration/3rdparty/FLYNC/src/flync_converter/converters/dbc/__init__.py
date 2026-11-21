"""Converter between the FLYNC model and CAN database (DBC) files.

:func:`load_dbc_files` reads DBC files into cantools databases and :func:`write_dbc_files` writes a FLYNC
model back out; the ``decode_*`` helpers translate cantools signals and messages into FLYNC signals and
PDUs (standard, multiplexed and container). :class:`DbcConverter` registers this conversion as a plugin
and supports both directions: FLYNC to DBC (via :meth:`DbcConverter.encode`) and DBC to FLYNC (via
:meth:`DbcConverter.decode`).
"""

from ...registry import hookimpl
from .converter import DbcConverter
from .dbc_config import DbcConverterConfig
from .decoder import decode_dbc_files
from .encoder import write_dbc_files
from .loading import load_dbc_files

__all__ = ["DbcConverter", "DbcConverterConfig", "load_dbc_files", "decode_dbc_files", "write_dbc_files"]


@hookimpl
def register_converters():
    """Register the DbcConverter with the pluggy plugin manager."""
    return [DbcConverter()]
