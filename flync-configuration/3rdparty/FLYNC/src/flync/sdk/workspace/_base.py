"""
Shared state and path helpers for :class:`~flync.sdk.workspace.flync_workspace.FLYNCWorkspace`.

This is the bottom layer of the workspace class chain: it owns every instance attribute and the
small, dependency-free helpers that every other layer builds on.
"""

import logging
from dataclasses import dataclass
from functools import lru_cache
from pathlib import Path
from typing import Any, Dict, Literal, Optional

from pydantic_core import ErrorDetails

from flync.core.base_models.base_model import FLYNCBaseModel
from flync.model.flync_model import FLYNCModel
from flync.sdk.context.workspace_config import (
    ListObjectsMode,
    WorkspaceConfiguration,
)
from flync.sdk.utils.model_dependencies import ModelDependencyGraph, get_model_dependency_graph
from flync.sdk.utils.sdk_types import PathType

from .document import Document
from .ids import ObjectId
from .objects import SemanticObject
from .source import SourceRef

logger = logging.getLogger(__name__)


@dataclass
class ParentLink(object):
    """
    Records where a loaded node hangs off its parent so a reloaded value can be put back in place.

    This is captured while the workspace loads. When a single document later changes we re-validate
    just that node and use this link to splice the fresh value into the parent model, instead of
    rebuilding the whole tree.

    Attributes:
        parent_path (Path): Absolute path of the parent load-node (a file or a directory).
        field_name (str): Python field name on the parent model that holds this value.
        container (Literal["scalar", "list", "dict"]): How the value sits on that field. ``"scalar"`` is a
            plain field (e.g. an ECU's ``ports`` file or its ``topology``); ``"list"``/``"dict"`` are
            folder-based collections whose items load from separate documents.
        key (int | str | None): List index or dict key of this value; ``None`` for scalar fields.
    """

    parent_path: Path
    field_name: str
    container: Literal["scalar", "list", "dict"]
    key: int | str | None = None


@dataclass
class LoadNode(object):
    """
    Everything needed to reload one document on its own, i.e. one ``_load_from_path`` call.

    Attributes:
        path (Path): Absolute path (file or directory) this node was loaded from.
        doc_id (str): Workspace-relative id; matches the keys of ``FLYNCWorkspace.documents``.
        current_type (type): The model type this node was validated against.
        current_type_name (str | None): Parent field name, used to rebuild the effective validation type.
        object_paths (list[str]): The object-path context this node was registered under.
        link (ParentLink | None): How this node attaches to its parent; ``None`` for the root node.
        model: The last successfully loaded value for this node, or ``None`` if it failed to load.
    """

    path: Path
    doc_id: str
    current_type: type[FLYNCBaseModel]
    current_type_name: Optional[str]
    object_paths: list[str]
    link: Optional[ParentLink] = None
    model: object = None


class _WorkspaceBase(object):
    """
    Instance state and stateless path/name helpers shared by all workspace layers.

    See :class:`~flync.sdk.workspace.flync_workspace.FLYNCWorkspace` for the documented attributes.
    """

    def __init__(
        self,
        name: str,
        workspace_path: PathType = "",
        configuration: WorkspaceConfiguration | None = None,
    ):
        """
        Initialize the workspace.

        Args:
            name (str): Human-readable name for this workspace instance.
            workspace_path (PathType): Absolute path to the workspace root directory. An empty string raises :class:`ValueError`.
            configuration (WorkspaceConfiguration | None): Optional configuration object.
                When ``None``, a default :class:`~flync.sdk.context.workspace_config.WorkspaceConfiguration` is used.
        """

        if not name:
            raise ValueError(
                "Passed an invalid value for workspace name {}",
                name,
            )
        self.name = name
        self.configuration = configuration or WorkspaceConfiguration()
        self.model_graph: ModelDependencyGraph = get_model_dependency_graph(self.configuration.root_model)
        # documents
        self.documents: Dict[str, Document] = {}
        self.documents_diags: Dict[str, list[ErrorDetails]] = {}
        # semantic graph
        self.objects: Dict[ObjectId, SemanticObject] = {}
        self.sources: Dict[ObjectId, SourceRef] = {}
        # root information (if any)
        self.flync_model: Optional[FLYNCModel | FLYNCBaseModel] = None
        self.workspace_root: Optional[Path] = None
        if not workspace_path:
            raise ValueError(
                "Passed an invalid value for workspace root {}",
                workspace_path,
            )
        self.workspace_root = Path(workspace_path).absolute()
        self._model_to_object_ids: dict[int, list[ObjectId]] = {}
        # Immediate-parent object-id -> ordered immediate child ids, recorded as
        # child paths are built during load (parent + separator + segment) so
        # child lookups never destructure the dot-separated id. ``_linked_child_ids``
        # dedups so a child is registered under its parent at most once.
        self._children_by_parent: dict[str, list[str]] = {}
        self._linked_child_ids: set[str] = set()
        # Mapping of canonical ObjectId → list of duplicate ObjectIds.
        # Keys represent the chosen canonical identifier, while values hold all alternative IDs
        # that refer to the same logical object.
        self._duplicated_objects_ids: dict[ObjectId, list[ObjectId]] = {}
        # document id -> LoadNode, built during load so update_document can
        # partially reload a single document instead of the whole workspace.
        self._doc_index: dict[str, LoadNode] = {}

    @property
    def load_errors(self):
        """
        Flattened list of all validation errors across all loaded documents.

        Returns:
            list[ErrorDetails]: All per-document errors concatenated into a
            single list.
        """

        return [error for doc_errors in self.documents_diags.values() for error in doc_errors]

    def is_path_supported(self, path: PathType):
        """
        Return whether a path is a directory or a recognised FLYNC file.

        Args:
            path (PathType): The path to check.

        Returns:
            bool: ``True`` if the path is a directory or a FLYNC file.
        """

        if not isinstance(path, Path):
            path = Path(path)
        return path.is_dir() or self.is_flync_file(path)

    def is_flync_file(self, path: PathType):
        """
        Return whether a path has a recognised FLYNC file extension.

        Args:
            path (PathType): The path to check.

        Returns:
            bool: ``True`` if the path's combined suffixes are in :attr:`~WorkspaceConfiguration.allowed_extensions`.
        """

        if not isinstance(path, Path):
            path = Path(path)
        return "".join(path.suffixes) in self.configuration.allowed_extensions

    def name_form_file(self, file_name: str | Path) -> str:
        """
        Strip all recognised FLYNC file extensions from a filename.

        Iterates over every extension in :attr:`~flync.sdk.context.workspace_config.WorkspaceConfiguration.allowed_extensions`
        and removes it as a suffix, leaving the bare stem.
        If a :class:`pathlib.Path` is passed, only its ``name`` component is used.

        Args:
            file_name (str | Path): The filename or path to strip.

        Returns:
            str: The filename with all FLYNC extensions removed (e.g. ``"my_ecu.flync.yaml"`` → ``"my_ecu"``).
        """

        if isinstance(file_name, Path):
            file_name = file_name.name
        for extension in self.configuration.allowed_extensions:
            file_name = file_name.replace(extension, "")
        return file_name

    def fill_path_from_object(self, model_object: FLYNCBaseModel, object_path: str) -> str:
        """
        Replace placeholder segments in an object path with concrete keys.

        Traverses the workspace's root model following ``object_path``, substituting ``[]`` with the actual list index and ``{}`` with the
        actual dict key when ``model_object`` is found.

        Args:
            model_object (FLYNCBaseModel): The model instance to locate.
            object_path (str): Dot-separated path containing ``[]`` or ``{}`` placeholders.

        Returns:
            str: The resolved dot-separated path with concrete index/key values.
        """

        parts = object_path.split(".")
        current_parent = self.flync_model
        for parts_idx, part in enumerate(parts):
            if part == "[]":
                resolved = self.__resolve_list_placeholder(current_parent, model_object)
            elif part == "{}":
                resolved = self.__resolve_dict_placeholder(current_parent, model_object)
            else:
                current_parent = getattr(current_parent, part)
                continue
            if resolved is None:
                continue
            parts[parts_idx], current_parent = resolved
        return ".".join(parts)

    @staticmethod
    def __resolve_list_placeholder(current_parent, model_object: FLYNCBaseModel) -> tuple[str, Any] | None:
        """
        Find ``model_object`` in a list parent and return its index as a path segment.

        Args:
            current_parent: The list currently being traversed.
            model_object (FLYNCBaseModel): The model instance to locate.

        Returns:
            tuple[str, Any] | None: The stringified index and the matched item, or ``None`` if not found.
        """

        for idx, item in enumerate(current_parent):
            if item == model_object:
                return str(idx), item
        return None

    @staticmethod
    def __resolve_dict_placeholder(current_parent, model_object: FLYNCBaseModel) -> tuple[str, Any] | None:
        """
        Find ``model_object`` in a dict parent and return its key as a path segment.

        Args:
            current_parent: The dict currently being traversed.
            model_object (FLYNCBaseModel): The model instance to locate.

        Returns:
            tuple[str, Any] | None: The matching key and its value, or ``None`` if not found.
        """

        for key, value in current_parent.items():
            if value == model_object:
                return key, value
        return None

    @lru_cache(maxsize=None)
    def document_id_from_path(self, doc_path: str) -> str:
        """
        Return the workspace-relative string identifier for a document path.

        Args:
            doc_path (str): An absolute path to a document file.

        Returns:
            str: The path relative to the workspace root, as a string.
        """

        return Path(doc_path).absolute().relative_to(self.workspace_root).as_posix()  # type: ignore[arg-type]

    @staticmethod
    def new_object_path(current_path: str, new_object_name: int | str) -> str:
        """
        Extend a dot-separated object path with a new segment.

        Args:
            current_path (str): The existing dot-separated path.
            new_object_name (int | str): The segment to append.

        Returns:
            str: The extended path string.
        """

        return ".".join([current_path, str(new_object_name)]) if current_path else str(new_object_name)

    def update_objects_path(self, current_paths: list[str], new_object_name: str) -> list[str]:
        """
        Extend every path in a list with a new segment.

        Args:
            current_paths (list[str]): Existing dot-separated paths.
            new_object_name (str): The segment to append to each path.

        Returns:
            list[str]: New list of extended path strings.
        """

        child_paths = [self.new_object_path(current_path, new_object_name) for current_path in current_paths]
        if self.configuration.map_objects:
            # Each child path is its parent path with one appended segment, so the
            # parent/child pair is known here without ever splitting the id. Index
            # alignment holds because both lists are built from ``current_paths``.
            for parent_path, child_path in zip(current_paths, child_paths):
                self._link_child_path(parent_path, child_path)
            if (
                len(child_paths) > 1
                and ListObjectsMode.INDEX in self.configuration.list_objects_mode
                and ListObjectsMode.NAME in self.configuration.list_objects_mode
            ):
                self._duplicated_objects_ids.setdefault(child_paths[0], []).extend(child_paths[1:])  # type: ignore[arg-type]
        return child_paths

    def _link_child_path(self, parent_id: str, child_id: str) -> None:
        """
        Record a parent -> child edge in the child index.

        ``parent_path``/``child_path`` are raw (possibly leading-dot) traversal
        paths; they are normalized with the same ``strip(".")`` used when objects
        are registered, so the recorded ids match :attr:`objects` keys. Root-level
        ids (which normalize to an empty parent) are skipped, preserving the
        "root has no children" behaviour.
        """

        if not parent_id or child_id in self._linked_child_ids:
            return

        self._linked_child_ids.add(child_id)
        self._children_by_parent.setdefault(parent_id, []).append(child_id)
