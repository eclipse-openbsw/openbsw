"""
Node helper functions for the FLYNC SDK.

Provides utilities to resolve node types from the model dependency graph and enumerate all nodes reachable from a given root model.
"""

import faulthandler
import logging
from typing import Optional

from flync.model import FLYNCBaseModel, FLYNCModel
from flync.sdk.context.node_info import NodeInfo
from flync.sdk.utils.model_dependencies import get_model_dependency_graph

logger = logging.getLogger(__name__)

faulthandler.enable()


def type_from_input(node: str | type[FLYNCBaseModel]) -> type[FLYNCBaseModel]:
    """
    Resolve a node identifier to its Python type.

    Accepts either a string class name (looked up in the global dependency graph) or a type directly.

    Args:
        node (str | type[FLYNCBaseModel]): A model class or its string name.

    Returns:
        type[FLYNCBaseModel]: The resolved model class.
    """

    if isinstance(node, str):
        node = get_model_dependency_graph(root=FLYNCModel).fields_info[node].python_type
    return node  # type: ignore[return-value]


def available_flync_nodes(
    root_node: Optional[str | type[FLYNCBaseModel]] = FLYNCModel,
) -> dict[str, NodeInfo]:
    """
    Return metadata for all nodes reachable from a root model.

    Args:
        root_node (str | type[FLYNCBaseModel] | None): The root model class
            or its name. Defaults to
              :class:`~flync.model.flync_model.FLYNCModel`.

    Returns:
        dict[str, NodeInfo]: Mapping of class names to :class:`NodeInfo`
        objects describing each node in the dependency graph.
    """

    if root_node is None:
        root_node = FLYNCModel
    root_node = type_from_input(root_node)
    return get_model_dependency_graph(root_node).fields_info
