"""Defines the bindings of a Controller to the Applications it hosts."""

from typing import Annotated, Dict, List

from pydantic import Field

from flync.core.annotations.reference import Reference
from flync.core.base_models import FLYNCBaseModel
from flync.core.utils.exceptions import Category, err_major

from .application import App


class AppBindings(FLYNCBaseModel):
    """
    Reference of Applications a Controller should bind to.

    Parameters
    ----------
    app_refs: list of str
        List of application names to bind to.

    Private Attributes
    ------------------
    _apps : list of :class:`~flync.model.flync_4_app.App`
        Applications to bind to. Managed internally.

    """

    app_refs: Annotated[List[str], Reference(source="_apps")] = Field(default_factory=list, description="List of application names to bind to.")

    _apps: List[App] = []

    @property
    def apps(self) -> List["App"]:
        return self._apps

    def resolve_apps(self, apps_by_name: Dict[str, App], controller_name: str) -> None:
        """Resolve ``app_refs`` against the system-wide apps, raising if any name is undefined."""
        resolved = []
        for name in self.app_refs:
            app = apps_by_name.get(name)
            if app is None:
                raise err_major(
                    f"App '{name}' referenced in app_bindings of {controller_name} was not found or was not validated",
                    category=Category.REFERENCE,
                    error_number="187",
                )
            resolved.append(app)
        self._apps = resolved
