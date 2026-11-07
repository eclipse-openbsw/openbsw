"""This package provides annotations that define how model fields are named, stored, and inferred."""

from .external import External, NamingStrategy, OutputStrategy  # noqa: F401
from .implied import Implied, ImpliedStrategy  # noqa: F401
from .reference import Reference  # noqa: F401
