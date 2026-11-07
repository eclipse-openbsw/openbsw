"""
Disk-to-model loading for the FLYNC workspace.

Walks a workspace directory, routes externally annotated fields to their files or folders, and
validates each load node. Depends on :mod:`._object_mapping` to register what it loads.
"""

import logging
import multiprocessing
import os
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
from itertools import islice
from pathlib import Path
from types import UnionType
from typing import Annotated, Optional, Union, get_args, get_origin

from pydantic_core import ValidationError

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions_handling import is_semantic_validation_error, validate_with_policy
from flync.sdk.utils.field_utils import get_metadata
from flync.sdk.utils.model_dependencies import model_force_rebuild
from flync.sdk.utils.sdk_types import PathType

from ._base import LoadNode, ParentLink
from ._object_mapping import _WorkspaceObjectMapping
from .document import Document, parse_documents, read_file

logger = logging.getLogger(__name__)


class _WorkspaceLoading(_WorkspaceObjectMapping):
    """Reads FLYNC documents from disk and validates them into model instances."""

    def _open_documents(self):
        """
        Open all documents in the workspace matching the configured file extension.
        File I/O and YAML parsing happen inside the ProcessPool workers; the raw text
        is NOT returned through IPC to reduce pickle overhead — it is re-read from the
        OS page cache in the main process for Document construction.

        Returns:
            None
        """
        if self.workspace_root is None:
            return
        files = [p for p in self.workspace_root.rglob(f"*{self.configuration.flync_file_extension}") if p.is_file()]
        if len(files) == 0:
            return

        def batched(iterable, size):
            """Yield successive batches of given size from an iterable."""
            i = iter(iterable)
            while batch := list(islice(i, size)):
                yield batch

        workers = os.cpu_count() or 1
        batch_size = max(32, len(files) // (workers * 4))
        batches = list(batched(files, batch_size))

        # A forking ProcessPoolExecutor is unsafe here: load_workspace is driven from
        # asyncio.to_thread(...), so the default POSIX "fork" start method forks from a
        # multi-threaded process and can deadlock the child. Use a non-fork context
        # (forkserver on POSIX, spawn elsewhere); everything submitted (module-level
        # parse_document, path/str/bool args) is picklable, so this is behaviour-neutral.
        mp_context = multiprocessing.get_context("forkserver" if sys.platform != "win32" else "spawn")
        with ProcessPoolExecutor(mp_context=mp_context) as pool:
            futures = [
                pool.submit(
                    parse_documents,
                    batch,
                    self.workspace_root,
                    self.configuration.map_objects,
                )
                for batch in batches
            ]

            for future in as_completed(futures):
                for uri, ast, compose_ast in future.result():
                    text = read_file(self.workspace_root / uri)
                    doc = Document(uri, text, self.configuration.map_objects)
                    doc.assign_ast(ast, compose_ast)
                    self.documents[doc.uri] = doc

    def _open_document(self, uri: PathType):
        """
        Open a document, parse it, and add it to the workspace.

        Args:
            uri (str): The document's URI.

            text (str): The raw text content of the document.

        Returns: None
        """
        text = read_file(uri)
        uri = Document.normalize_uri(uri, self.workspace_root)
        doc = Document(uri, text, self.configuration.map_objects)
        doc.parse()
        self.documents[uri] = doc

    def __load_list_item(
        self,
        sub_item_path: Path,
        base_type,
        base_type_args: tuple,
        list_element_type,
        field_name: str,
        item_dir: Path,
        external,
        list_paths: list[str],
        parent_path: Path,
        position: int,
    ):
        """
        Load one item from a list-folder entry, handling Union and concrete types.

        Args:
            sub_item_path (Path): Path to the file or folder for this item.
            base_type: Origin type of the list element (e.g. ``Union`` or ``None``).
            base_type_args (tuple): Generic args of ``base_type``.
            list_element_type: Declared element type of the list field.
            field_name (str): Field name on the parent model.
            item_dir (Path): Parent directory containing the list items.
            external: The ``External`` annotation for this field.
            list_paths (list[str]): Dot-path context for this item.
            parent_path (Path): Path of the parent load-node that owns the list field.
            position (int): Index this item occupies in the built list (skipped entries excluded),
                used to splice a reloaded value back into the parent.

        Returns:
            The loaded model instance, or ``None`` if loading failed.
        """

        link = ParentLink(parent_path, field_name, "list", position)
        if base_type is Union or base_type is UnionType:
            item_info: dict = {}
            self.__handle_generic_types_union(
                base_type_args,
                external,
                sub_item_path.name,
                field_name,
                field_name,
                item_info,
                item_dir,
                list_paths,
                link,
            )
            if field_name not in item_info:
                logger.warning(
                    "Skipping file %s: could not be loaded as any of the expected types.",
                    str(sub_item_path),
                )
                return None
            return item_info[field_name]
        else:
            item = self._load_from_path(
                sub_item_path,
                list_element_type,
                field_name,
                list_paths,
                link,
            )
            if item is None:
                logger.warning(
                    "Skipping file %s: failed to load.",
                    str(sub_item_path),
                )
            return item

    def __handle_generic_types_list(
        self,
        base_type_args: tuple,
        external: External,
        external_path: str,
        field_name: str,
        module_load_info: dict,
        path: Path,
        current_object_paths: list[str],
    ) -> bool:
        """
        Load an external ``list`` field from disk into ``module_load_info``.

        Iterates files/folders under the external directory for ``FOLDER`` strategy, or delegates to a single-file loader for
        ``SINGLE_FILE`` strategy.

        Args:
            base_type_args (tuple): Generic args of the list annotation.
            external (External): Annotation controlling the load strategy.
            external_path (str): Relative path segment for this field.
            field_name (str): Field name on the parent model.
            module_load_info (dict): Accumulator for loaded field values; updated in place.
            path (Path): Absolute path of the current directory.
            current_object_paths (str): Dot-path context for object tracking.

        Returns:
            bool: ``True`` if the field was handled, ``False`` otherwise.
        """

        list_item_value: list = []
        list_element_type = base_type_args[0]
        if OutputStrategy.FOLDER in external.output_structure:
            item_dir = path / external_path
            effective_element_type = list_element_type
            if get_origin(list_element_type) is Annotated:
                effective_element_type = get_args(list_element_type)[0]
            base_type = get_origin(effective_element_type)
            base_type_args = get_args(effective_element_type)
            # Sort so list indices (and therefore object ids and list order) are
            # deterministic across loads and filesystems; iterdir() order is not.
            for idx, sub_item_path in enumerate(sorted(item_dir.iterdir())):
                if not self.is_path_supported(sub_item_path):
                    logger.warning(
                        "Unrecognized file found in FLYNC workspace: %s",
                        str(sub_item_path),
                    )
                    continue
                list_name = self.name_form_file(sub_item_path)
                list_paths = self.add_list_item_object_path(list_name, current_object_paths, idx)
                item = self.__load_list_item(
                    sub_item_path,
                    base_type,
                    base_type_args,
                    list_element_type,
                    field_name,
                    item_dir,
                    external,
                    list_paths,
                    path,
                    len(list_item_value),
                )
                if item is None:
                    continue
                list_item_value.append(item)
            module_load_info[field_name] = list_item_value
            return True
        if OutputStrategy.SINGLE_FILE in external.output_structure:
            new_base_type = base_type_args[0]
            single_info: dict = {}
            self.__handle_generic_types(
                attribute_type=new_base_type,
                base_type=get_origin(new_base_type),
                base_type_args=get_args(new_base_type),
                external=external,
                path=path,
                external_path=external_path,
                module_load_info=single_info,
                field_name=field_name,
                storage_key=field_name,
                current_object_paths=current_object_paths,
            )
            module_load_info.update(single_info)
            return True
        return False

    def __handle_generic_types_dict(
        self,
        base_type_args: tuple,
        external: External,
        external_path: str,
        field_name: str,
        module_load_info: dict,
        path: Path,
        current_object_paths: list[str],
    ) -> bool:
        """
        Load an external ``dict`` field from disk into ``module_load_info``.

        Iterates items under the external directory for ``FOLDER`` strategy, or delegates to a single-file loader for ``SINGLE_FILE`` strategy.

        Args:
            base_type_args (tuple): Generic args of the dict annotation ``(key_type, value_type)``.
            external (External): Annotation controlling the load strategy.
            external_path (str): Relative path segment for this field.
            field_name (str): Field name on the parent model.
            module_load_info (dict): Accumulator for loaded field values; updated in place.
            path (Path): Absolute path of the current directory.
            current_object_paths (list[str]): Dot-path contexts for object tracking.

        Returns:
            bool: ``True`` if the field was handled, ``False`` otherwise.
        """

        dict_item_value = {}
        dict_element_type = base_type_args[1]
        if OutputStrategy.FOLDER in external.output_structure:
            item_dir = path / external_path
            for sub_item_path in sorted(item_dir.iterdir()):
                if not self.is_path_supported(sub_item_path):
                    logger.warning(
                        "Unrecognized file found in FLYNC workspace: %s",
                        str(sub_item_path),
                    )
                    continue
                dict_item_value[sub_item_path.name] = self._load_from_path(
                    sub_item_path,
                    dict_element_type,
                    field_name,
                    self.update_objects_path(current_object_paths, sub_item_path.name),
                    ParentLink(path, field_name, "dict", sub_item_path.name),
                )
            module_load_info[field_name] = dict_item_value
            return True
        if OutputStrategy.SINGLE_FILE in external.output_structure:
            new_base_type = base_type_args[1]
            dict_info: dict = {}
            self.__handle_generic_types(
                attribute_type=new_base_type,
                base_type=get_origin(new_base_type),
                base_type_args=get_args(new_base_type),
                external=external,
                path=path,
                external_path=external_path,
                module_load_info=dict_info,
                field_name=field_name,
                storage_key=field_name,
                current_object_paths=current_object_paths,
            )
            module_load_info.update(dict_info)
            return True
        return False

    def __try_load_union_type(
        self,
        path: Path,
        external_path: str,
        possible_type,
        field_name: str,
        current_object_paths: list[str],
        link: Optional[ParentLink] = None,
    ):
        """
        Attempt to load one union member type, restoring diagnostics on failure.

        Args:
            path (Path): Absolute path of the current directory.
            external_path (str): Relative path segment for this field.
            possible_type: The union member type to attempt.
            field_name (str): Field name on the parent model.
            current_object_paths (list[str]): Dot-path contexts for object tracking.

        Returns:
            The loaded model instance, or ``None`` if the type did not match.
        """

        attempt_path = (path / external_path).absolute()
        doc_id = self.document_id_from_path(attempt_path)
        diags_existed = doc_id in self.documents_diags
        saved_diags = list(self.documents_diags.get(doc_id, []))
        saved_count = len(saved_diags)
        result = self._load_from_path(
            path / external_path,
            possible_type,
            field_name,
            current_object_paths,
            link,
        )
        if result is None:
            new_diags = self.documents_diags.get(doc_id, [])[saved_count:]
            # If the failed attempt produced a user-raised semantic error
            # (err_major / err_minor / err_fatal on the matched type), keep
            # the diags so the user sees them. Discard only purely structural
            # mismatches, which signal "wrong union member".
            has_semantic_error = any(is_semantic_validation_error(d) for d in new_diags)
            if not has_semantic_error:
                if diags_existed:
                    self.documents_diags[doc_id] = saved_diags
                elif doc_id in self.documents_diags:
                    del self.documents_diags[doc_id]
        return result

    def __handle_generic_types_union(
        self,
        base_type_args: tuple,
        external,
        external_path: str,
        field_name: str,
        storage_key: str,
        module_load_info: dict,
        path: Path,
        current_object_paths: list[str],
        link: Optional[ParentLink] = None,
    ) -> bool:
        """
        Attempt to load an external ``Union`` field by trying each member type.

        Iterates through the union's member types and loads the first one that succeeds. ``NoneType`` members are skipped.

        Args:
            base_type_args (tuple): The union member types.
            external: The ``External`` annotation for this field.
            external_path (str): Relative path segment for this field.
            field_name (str): Field name on the parent model.
            storage_key (str): The key under which the successfully loaded field value.
            Typically corresponds to the field name or its alias.
            module_load_info (dict): Accumulator for loaded field values; updated in place.
            path (Path): Absolute path of the current directory.
            current_object_paths (list[str]): Dot-path contexts for object tracking.

        Returns:
            bool: ``True`` if at least one union member loaded successfully.
        """

        success_union = False
        for possible_type in base_type_args:
            try:
                if possible_type is type(None):
                    # optional external field, don't do anything
                    continue
                possible_base_type = get_origin(possible_type)
                if issubclass(possible_base_type or possible_type, FLYNCBaseModel):
                    result = self.__try_load_union_type(
                        path,
                        external_path,
                        possible_type,
                        field_name,
                        current_object_paths,
                        link,
                    )
                    if result is None:
                        continue
                    module_load_info[storage_key] = result
                else:
                    self.__handle_generic_types(
                        possible_type,
                        possible_base_type,
                        get_args(possible_type),
                        external,
                        path,
                        external_path,
                        module_load_info,
                        field_name,
                        storage_key,
                        current_object_paths,
                    )
                success_union = True
                break
            # A failed member load means "wrong union member" -- try the next one.
            except (TypeError, ValueError, KeyError, AttributeError, OSError):
                continue
        return success_union

    def __handle_generic_types(
        self,
        attribute_type: type,
        base_type: type | None,
        base_type_args: tuple,
        external: External,
        path: Path,
        external_path: str,
        module_load_info: dict,
        field_name: str,
        storage_key: str,
        current_object_paths: list[str],
    ):
        """
        Dispatch an external field to the correct type-specific loader.

        Routes ``list``, ``dict``, and ``Union`` types to their dedicated handlers.
        Falls through to a direct model load for concrete ``FLYNCBaseModel`` subclasses, or does nothing for optional fields whose value is absent.

        Args:
            attribute_type (type): The full (possibly generic) annotation type.
            base_type (type | None): The ``get_origin`` of ``attribute_type``, or ``None`` for non-generic types.
            base_type_args (tuple): The ``get_args`` of ``attribute_type``.
            external (External): Annotation controlling load strategy.
            path (Path): Absolute path of the current directory.
            external_path (str): Relative path segment for this field.
            module_load_info (dict): Accumulator for loaded field values; updated in place.
            field_name (str): Field name on the parent model.
            current_object_paths (str): Dot-path context(s) for object tracking.

        Raises:
            ValueError: If the field type is not supported for external loading.
        """

        done = False
        scalar_link = ParentLink(path, field_name, "scalar")

        if base_type is list:
            done = self.__handle_generic_types_list(
                base_type_args,
                external,
                external_path,
                field_name,
                module_load_info,
                path,
                current_object_paths,
            )

        elif base_type is dict:
            done = self.__handle_generic_types_dict(
                base_type_args,
                external,
                external_path,
                field_name,
                module_load_info,
                path,
                current_object_paths,
            )

        elif base_type is Union or base_type is UnionType:
            done = self.__handle_generic_types_union(
                base_type_args,
                external,
                external_path,
                field_name,
                storage_key,
                module_load_info,
                path,
                current_object_paths,
                scalar_link,
            )

        if not done and type(None) in base_type_args:
            # optional type
            done = True

        if done:
            # this field might not have been added to the objects since it's
            # not a flync model and has no document. Add it manually.
            doc_id = self.document_id_from_path(path / external_path if external_path else path)
            self._add_object_to_path(
                doc_id=doc_id,
                model=(module_load_info[field_name] if field_name in module_load_info else None),
                current_object_paths=current_object_paths,
                start_line=0,
                end_line=0,
                end_column=0,
                start_column=0,
            )
            return

        if not issubclass(get_origin(attribute_type) or attribute_type, FLYNCBaseModel):
            raise ValueError(f"externally annotated field {field_name} cannot be loaded")

        module_load_info[field_name] = self._load_from_path(
            path / external_path,
            attribute_type,
            field_name,
            current_object_paths,
            scalar_link,
        )

    def _load_from_path(
        self,
        path: PathType,
        current_type: Optional[type[FLYNCBaseModel]] = None,
        current_type_name: Optional[str] = None,
        current_object_paths: Optional[list[str]] = None,
        link: Optional[ParentLink] = None,
    ) -> FLYNCBaseModel | None:
        """
        Load and validate a model from a filesystem path.

        Recursively processes all fields of ``current_type``, routing external fields to their files/directories and collecting implied values.
        After gathering all field data it validates the dict against the type and updates the workspace's object and diagnostic stores.

        Args:
            path (PathType): Directory (or file) path to load from.
            current_type (type[FLYNCBaseModel] | None): The expected model type. Defaults to the workspace's configured root model.
            current_type_name (str | None): The parent field name for this type, used to reconstruct the correct validation type.
            current_object_paths (list[str] | None): Dot-path context(s) for object tracking.

        Returns:
            FLYNCBaseModel | None: The validated model instance, or ``None`` if validation failed.
        """

        # if no type is passed, then this is the starting point
        if current_type is None:
            current_type = self.configuration.root_model
        model_force_rebuild(current_type)
        if isinstance(path, str):
            path = Path(path)
        if not current_object_paths:
            current_object_paths = [""]
        path = path.absolute()
        doc_id = self.document_id_from_path(path)
        self._doc_index[doc_id] = LoadNode(
            path=path,
            doc_id=doc_id,
            current_type=current_type,
            current_type_name=current_type_name,
            object_paths=list(current_object_paths),
            link=link,
        )
        module_load_info: dict = {}
        # start by loading each field
        for field_name, field_info in current_type.model_fields.items():
            external: External | None = get_metadata(field_info.metadata, External)
            self.__handle_external_field_load(
                path,
                current_object_paths,
                module_load_info,
                field_name,
                field_info,
                external,
            )
            if implied := get_metadata(field_info.metadata, Implied):
                self._handle_implied_field_load(path, module_load_info, field_name, implied)
                # Implied fields carry no YAML node and no model value of their
                # own, so the AST walk never registers them. Register a marker
                # object (model=None) under the linked path; without it
                # get_child_ids would drop the field, since it filters children
                # by has_object().
                paths = self.update_objects_path(current_object_paths, field_name)
                self._add_object_to_path("", None, paths, 0, 0, 0, 0)

        # then group all the fields into the same object and return it
        self._append_to_info_dict(path, module_load_info)

        if doc_id in self.documents_diags:
            logger.error("File %s was already loaded.", doc_id)
        return self._validate_node(self._doc_index[doc_id], module_load_info, map_paths=current_object_paths)

    def _handle_implied_field_load(
        self,
        path: Path,
        module_load_info: dict,
        field_name: str,
        implied: Implied,
    ):
        if implied.strategy == ImpliedStrategy.FOLDER_NAME:
            module_load_info[field_name] = path.name
        elif implied.strategy == ImpliedStrategy.FILE_NAME:
            module_load_info[field_name] = self.name_form_file(path)

    def __handle_external_field_load(
        self,
        path,
        current_object_paths,
        module_load_info,
        field_name,
        field_info,
        external,
    ):
        if external is not None:
            # field will need to be added to to a new separate document
            attribute_type = field_info.annotation
            if attribute_type is None:
                raise ValueError("Attribute {} has an invalid type.", field_name)
            base_type: type | None = get_origin(attribute_type)
            base_type_args = get_args(attribute_type)
            storage_key = field_name
            external_path = self.__get_external_path(path, external, field_name)
            if not external_path.exists() and field_info.alias is not None:
                external_path = self.__get_external_path(path, external, field_info.alias)
                storage_key = field_info.alias
            if OutputStrategy.SINGLE_FILE in external.output_structure:
                if OutputStrategy.OMMIT_ROOT not in external.output_structure:
                    # the output file is a dictionary
                    # we need to load it accordingly
                    attribute_type = dict[str, attribute_type]  # type: ignore[valid-type]
                    base_type = get_origin(attribute_type)
                    base_type_args = get_args(attribute_type)
            new_paths = self.update_objects_path(current_object_paths, field_name)
            self.__handle_generic_types(
                attribute_type,
                base_type,
                base_type_args,
                external,
                path,
                external_path,
                module_load_info,
                field_name,
                storage_key,
                new_paths,
            )

    def _append_to_info_dict(
        self,
        path: Path,
        model_load_info: dict,
        output_strategy: Optional[OutputStrategy] = None,
        field_name: Optional[str] = None,
        fixed_name: Optional[str] = None,
    ):
        """
        Merge the contents of a FLYNC file into a model load-info dict.

        Opens the file at ``path``, registers it as a document, and merges its parsed YAML content into ``model_load_info``.
        The merge behaviour depends on ``output_strategy``:

        - ``OMMIT_ROOT``: assigns the raw content to ``model_load_info[field_name]``.
        - ``FIXED_ROOT``: assigns only the ``fixed_name`` key of the content.
        - Default: updates ``model_load_info`` with all top-level keys.

        Does nothing when ``path`` is not a file or is not a recognised FLYNC file extension.

        Args:
            path (Path): Path to the FLYNC YAML file.
            model_load_info (dict): Accumulator dict; updated in place.
            output_strategy (OutputStrategy | None): Optional output strategy that controls how the file content is merged.
            field_name (str | None): Target key in ``model_load_info`` for ``OMMIT_ROOT`` / ``FIXED_ROOT`` strategies.
            fixed_name (str | None): Key inside the file content to extract for ``FIXED_ROOT`` strategy.
        """

        if not path.is_file():
            return

        if not self.is_flync_file(path):
            logger.error("trying to load an unsupported file: %s", str(path))
            return

        uri: str = self.document_id_from_path(path)
        if uri not in self.documents:
            self._open_document(path)
        content = self.documents[uri].ast
        if content is None:
            return

        if output_strategy and OutputStrategy.OMMIT_ROOT in output_strategy:
            model_load_info[field_name] = content
        elif output_strategy and OutputStrategy.FIXED_ROOT in output_strategy:
            model_load_info[field_name] = content[fixed_name]
        else:
            model_load_info.update(content)

    def __get_external_path(self, base_path: Path, external: External, field_name: str) -> Path:
        """
        Resolve the filesystem path for an external field.

        Constructs the path to an external file or directory based on the provided `External` configuration and naming strategy.
        If the `External` specifies a fixed path, that path is used; otherwise, the field name is used as the base.
        A file extension is appended if the output structure requires a single file.

        Args:
            base_path (Path): The root directory path where external files are located.
            external (External): The external configuration annotation for the field.
            field_name (str): The name of the field on the parent model.

        Returns:
            Path: The resolved absolute path to the external resource.
        """
        ext = self.configuration.flync_file_extension if OutputStrategy.SINGLE_FILE in external.output_structure else ""
        path = external.path if ((external.naming_strategy == NamingStrategy.FIXED_PATH) and (external.path is not None)) else field_name
        return base_path / (path + ext)

    def _validate_node(self, node: LoadNode, module_load_info: dict, map_paths: Optional[list[str]] = None):
        """
        Validate ``module_load_info`` for ``node`` the same way the initial load does.

        Resets the node's diagnostics, applies the parent-aware type rebuild/normalisation, records any
        validation errors against the node's document id and stores the result on ``node.model``.

        Args:
            node (LoadNode): The node being (re)validated.
            module_load_info (dict): The gathered field data to validate.
            map_paths (list[str] | None): When object mapping is enabled and these paths are given, the
                freshly validated model is registered in the object map under them. Left ``None`` for
                ancestor rebuilds, where the object map is maintained separately.

        Returns:
            The validated (and parent-normalized) model, or ``None`` on failure.
        """

        self.documents_diags[node.doc_id] = []
        if not module_load_info:
            node.model = None
            return None
        original_type = node.current_type
        current_type = node.current_type
        try:
            if node.current_type_name:
                current_type = self.model_graph.rebuild_type_from_parent(current_type, node.current_type_name)
            relative_path = node.path.relative_to(self.workspace_root.absolute())  # type: ignore[union-attr]
            model, errors = validate_with_policy(current_type, module_load_info, relative_path.as_posix())
            self.documents_diags[node.doc_id].extend(errors)
            if map_paths is not None and self.configuration.map_objects:
                self._update_objects(node.doc_id, model, map_paths, parent_name=node.current_type_name)
            if node.current_type_name:
                model = self.model_graph.normalize_child_to_parent(original_type, node.current_type_name, model)
            node.model = model
            return model
        except ValidationError as e:
            self.documents_diags[node.doc_id].extend(e.errors())
            node.model = None
            return None
