"""Defines an inclusive integer range model."""

from pydantic import Field

from flync.core.base_models.base_model import FLYNCBaseModel


class ValueRange(FLYNCBaseModel):
    """
    Defines an inclusive range of integer values.

    This datatype is typically used to express valid numeric intervals for parameters, identifiers, or signal values.

    Parameters
    ----------
    from_value : int
        Lower bound of the range (inclusive).

    to_value : int
        Upper bound of the range (inclusive).
    """

    from_value: int = Field()
    to_value: int = Field()
