"""Base class for all controller interfaces in FLYNC."""

from pydantic import Field

from flync.core.base_models import FLYNCBaseModel


class ControllerInterface(FLYNCBaseModel):
    """
    Base class for all controller interfaces (Ethernet, CAN, LIN).

    Subclasses must declare ``name`` with an appropriate :class:`~flync.core.annotations.Implied`
    strategy (``FOLDER_NAME`` for Ethernet, ``FILE_NAME`` for the bus interfaces).

    Parameters
    ----------
    name : str
        Name of the interface, implied from its file or folder name on disk.
    """

    name: str = Field()
