"""Defines the end-to-end (E2E) protection configuration for FLYNC."""

from pydantic import Field

from flync.core.base_models import FLYNCBaseModel


class E2EConfig(FLYNCBaseModel):
    """
    Defines an E2E (End-to-End) communication configuration.

    This model is used to describe the E2E protection settings for a communication channel, including the profile type and the \
    associated data identifier.

    Parameters
    ----------
    profile : str
        The profile for E2E communication, specifying the protection scheme to be used.

    data_id : int
        The data ID for E2E communication, used to uniquely identify the protected data.
    """

    profile: str = Field(description="The profile for E2E communication")
    data_id: int = Field(description="The data ID for E2E communication")
