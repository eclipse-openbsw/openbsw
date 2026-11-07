"""
Validation helper functions for the FLYNC SDK.

Provides high-level functions to validate FLYNC workspaces and individual nodes, returning structured
:class:`~flync.sdk.context.diagnostics_result.DiagnosticsResult` objects that capture the validation state, errors, and loaded model.
"""

import faulthandler
import logging
from pathlib import Path

from pydantic_core import (
    InitErrorDetails,
    PydanticCustomError,
    ValidationError,
)

from flync.model import FLYNCBaseModel, FLYNCModel
from flync.sdk.context.diagnostics_result import (
    DiagnosticsResult,
    WorkspaceState,
)
from flync.sdk.workspace.flync_workspace import (
    FLYNCWorkspace,
    WorkspaceConfiguration,
)
from flync.sdk.workspace.ids import ObjectId

from .nodes_helpers import type_from_input

logger = logging.getLogger(__name__)

faulthandler.enable()


def validate_workspace(
    workspace_path: str | Path,
    workspace_config: WorkspaceConfiguration | None = None,
) -> DiagnosticsResult:
    """
    Validate an entire FLYNC workspace rooted at the default ``FLYNCModel``.

    Args:
        workspace_path (str | Path): Path to the workspace directory.
        workspace_config (WorkspaceConfiguration | None): Optional workspace configuration. Uses defaults if ``None``.

    Returns:
        DiagnosticsResult: The validation outcome including state, errors, and the loaded model.
    """

    return validate_external_node(FLYNCModel, workspace_path, workspace_config)


def validate_external_node(
    node: str | type[FLYNCBaseModel],
    node_path: Path | str,
    workspace_config: WorkspaceConfiguration | None = None,
) -> DiagnosticsResult:
    """
    Validate a specific FLYNC node type at a given filesystem path.

    Loads the node using a fresh workspace configured with ``node`` as the root model, then inspects per-document errors to determine the overall
    :class:`~flync.sdk.context.diagnostics_result.WorkspaceState`.

    Args:
        node (str | type[FLYNCBaseModel]): The model class to validate, or its string name.
        node_path (Path | str): Path to the directory containing the node's FLYNC configuration files.
        workspace_config (WorkspaceConfiguration | None): Optional workspace configuration. \
            Uses defaults if ``None``. The ``root_model`` field is always overwritten with ``node``.

    Returns:
        DiagnosticsResult: Validation outcome with state, per-document errors, the loaded model, and the workspace instance.
    """

    node = type_from_input(node)
    state = WorkspaceState.EMPTY
    errors = {}
    model = None
    ws = None
    try:
        if workspace_config:
            workspace_config = WorkspaceConfiguration.create_from_config(workspace_config, root_model=node)
        else:
            workspace_config = WorkspaceConfiguration(root_model=node)
        ws = FLYNCWorkspace.safe_load_workspace(
            "validation_workspace",
            node_path,
            workspace_config=workspace_config,
        )
        model = ws.flync_model
        state = WorkspaceState.VALID
        # only add documents that have problems
        for doc_url, doc_errors in ws.documents_diags.items():
            if doc_errors:
                errors[doc_url] = doc_errors
                state = WorkspaceState.WARNING
        if ws.flync_model is None:
            state = WorkspaceState.INVALID
    except Exception as ex:
        state = WorkspaceState.BROKEN
        logger.exception("Encountered issue while validating node %s: %s", node_path, ex)
    return DiagnosticsResult(state=state, errors=errors, model=model, workspace=ws)


def validate_node(
    ws_path: Path | str,
    node_path: str = "",
    workspace_config: WorkspaceConfiguration | None = None,
) -> DiagnosticsResult:
    """
    Validate a single node within an already-loaded workspace.

    First validates the full workspace, then checks that the node at ``node_path`` exists and extracts its model. If the node is missing, a
    fatal validation error is recorded.

    Args:
        ws_path (Path | str): Path to the workspace root directory.
        node_path (str): Dot-separated path to the target node within the workspace object graph.
        workspace_config (WorkspaceConfiguration | None): Optional workspace configuration forwarded to :func:`validate_workspace`.

    Returns:
        DiagnosticsResult: Validation outcome for the specified node.
    """

    # load entire workspace
    workspace_results = validate_workspace(ws_path, workspace_config=workspace_config)
    # validate node in workspace
    if not workspace_results.workspace or node_path not in workspace_results.workspace.objects:
        workspace_results.state = WorkspaceState.INVALID
        fatal_ctx = {"node_path": node_path}
        error = InitErrorDetails(
            type=PydanticCustomError(
                "fatal",
                "unhandled exception caught: {ex}",
                fatal_ctx,
            ),
            ctx=fatal_ctx,
            input=workspace_results.model,
        )
        try:
            raise ValidationError.from_exception_data(title="partial node validation", line_errors=[error])
        except ValidationError as ex:
            workspace_results.errors[node_path] = ex.errors()
    else:
        workspace_results.model = workspace_results.workspace.objects[ObjectId(node_path)].model  # type: ignore[assignment]
    return workspace_results
