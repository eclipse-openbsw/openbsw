"""Base Model that is used by FLYNC Model classes."""

from pydantic import BaseModel, ConfigDict


class FLYNCBaseModel(BaseModel):
    """Base Model that is used by FLYNC Model classes."""

    model_config = ConfigDict(extra="forbid", validate_by_name=True, validate_assignment=True)

    def model_dump(self, **kwargs):
        """Override pydantics model_dump to dump with defaults."""

        kwargs.setdefault("exclude_none", True)
        kwargs.setdefault("by_alias", True)
        return super().model_dump(**kwargs)
