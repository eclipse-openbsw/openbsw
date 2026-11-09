"""
Diagnostic result types for the FLYNC SDK.

Provides :class:`WorkspaceState` and :class:`DiagnosticsResult`, which represent the outcome of validating a FLYNC workspace or node.
"""

from enum import Enum
from os import getcwd
from typing import Optional

from pydantic import ConfigDict, field_serializer
from pydantic.dataclasses import dataclass
from pydantic_core import ErrorDetails

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.sdk.workspace.flync_workspace import FLYNCWorkspace


class WorkspaceState(str, Enum):
    """
    Enumeration of possible workspace validation states.

    Attributes:
        **UNKNOWN**: State has not been determined.

        **EMPTY**: No model was loaded; the workspace is empty.

        **LOADING**: The workspace is currently being loaded.

        **VALID**: All documents validated successfully with no errors.

        **WARNING**: Validation completed but some documents have non-fatal errors.

        **INVALID**: The model could not be constructed due to validation errors.

        **BROKEN**: An unexpected exception occurred during loading.
    """

    UNKNOWN = "unknown"
    EMPTY = "empty"
    LOADING = "loading"
    VALID = "valid"
    WARNING = "warning"
    INVALID = "invalid"
    BROKEN = "broken"


@dataclass(config=ConfigDict(arbitrary_types_allowed=True))
class DiagnosticsResult:
    """
    Result of a workspace or node validation operation.

    Attributes:
        state (WorkspaceState): The overall validation state.
        errors (dict[str, list[ErrorDetails]]): Mapping of document URIs to their associated Pydantic validation error details.
        model (Optional[FLYNCBaseModel]): The validated root model, or ``None`` if validation failed.
        workspace (Optional[FLYNCWorkspace]): The loaded workspace instance, or ``None`` if the workspace could not be created.
    """

    state: WorkspaceState
    errors: dict[str, list[ErrorDetails]]
    model: Optional[FLYNCBaseModel] = None
    workspace: Optional[FLYNCWorkspace] = None

    @property
    def passed(self) -> bool:
        """
        Whether validation got through with nothing worse than warnings.

        Checks the state as well as the diagnostics, because a workspace that never loaded reports ``BROKEN`` with an empty ``errors``
        dict. Judging by diagnostics alone would call that a pass.
        """

        if self.state not in (WorkspaceState.VALID, WorkspaceState.WARNING):
            return False

        return all(entry.get("type") == "warning" for entries in self.errors.values() for entry in entries)

    @field_serializer("workspace")
    def serialize_workspace(self, value: FLYNCWorkspace) -> str:
        """
        Serialize the workspace to a compact string representation.

        Args:
            value (FLYNCWorkspace): The workspace to serialize.

        Returns:
            str: A human-readable string with the workspace name and relative path.
        """

        workspace_path = value.workspace_root.relative_to(getcwd()) if value.workspace_root is not None else None
        return f"workspace name: {value.name}, workspace path: {workspace_path}"
