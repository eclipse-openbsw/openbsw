"""Configuration for the DBC converter."""

from ...base import ConverterConfig

_DEFAULT_BAUD_RATE = 500_000
_DEFAULT_FD_BAUD_RATE = 2_000_000


class DbcConverterConfig(ConverterConfig):
    """Configuration for the DBC converter.

    Parameters
    ----------
    baud_rate_default : int
        Nominal CAN bit rate used when the DBC defines no ``Baudrate``
        attribute, or defines one that is not on the FLYNC allow-list.
        Defaults to ``500000``.
    fd_baud_rate_default : int
        CAN FD data-phase bit rate used when the DBC defines no
        ``BaudrateCANFD`` attribute, or defines one that is not on the FLYNC
        allow-list.  Defaults to ``2000000``.
    """

    baud_rate_default: int = _DEFAULT_BAUD_RATE
    fd_baud_rate_default: int = _DEFAULT_FD_BAUD_RATE
