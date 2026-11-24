"""
Semantic object registry and source-location APIs for the FLYNC workspace.

Builds the ``objects``/``sources`` maps while a model is validated, and exposes the lookup APIs
used by the SDK and the language server. Depends only on :mod:`._base`.
"""

import logging
from itertools import chain
from pathlib import Path
from typing import TYPE_CHECKING, Optional, cast

from pydantic import RootModel, ValidationError
from pydantic.fields import FieldInfo
from ruamel.yaml.nodes import MappingNode, Node, SequenceNode
from typing_extensions import deprecated

from flync.core.annotations.reference import Reference, resolve_reference
from flync.core.base_models.base_model import FLYNCBaseModel
from flync.core.utils.exceptions_handling import get_name_by_alias, get_unique_errors, has_validators, validate_with_policy
from flync.sdk.context.workspace_config import ListObjectsMode
from flync.sdk.utils.field_utils import get_field_name_from_alias, get_metadata
from flync.sdk.utils.model_dumper import dump_model_with_discriminators

from ._base import _WorkspaceBase
from .ids import ObjectId
from .objects import ObjectMetadata, SemanticObject
from .source import SourceRef, get_range

if TYPE_CHECKING:
    from .flync_workspace import FLYNCWorkspace

logger = logging.getLogger(__name__)


class _WorkspaceObjectMapping(_WorkspaceBase):
    """Semantic object registry, plus the source APIs layered on top of it."""

    def _update_objects(
        self,
        path_id: str,
        model: FLYNCBaseModel | None,
        current_object_paths: list[str],
        node: Node | None = None,
        parent_name: str | None = None,
    ):
        """
        Recursively register model values and their source positions.

        Walks the YAML AST node alongside the validated model, calling :meth:`_add_object_to_path` for every value encountered so that
        each semantic object is associated with its source location.

        Args:
            path_id (str): document id of the document containing this node.
            model (FLYNCBaseModel): The validated model value at this node.
            current_object_paths (str | list[str]): Dot-path context(s) for the current model value.
            node (Node | None): The ruamel.yaml AST node corresponding to ``model``. Defaults to the document's root compose AST.
            parent_name (str | None): The field name on the parent that points to this node, used for sequence items.
        """

        start_line = 0
        end_line = 0
        start_column = 0
        end_column = 0
        if isinstance(model, RootModel):
            model = model.root
        if model is not None and path_id in self.documents:
            # object is all external fields
            # should already be updated
            document = self.documents[path_id]
            if node is None:
                node = document.compose_ast
            if isinstance(node, MappingNode):
                self._update_mapping_node_objects(path_id, model, current_object_paths, node)
            elif isinstance(node, SequenceNode):
                self._update_sequence_node_objects(path_id, model, current_object_paths, node, parent_name)
            if node is not None:
                start_line, start_column = (
                    node.start_mark.line + 1,
                    node.start_mark.column + 1,
                )
                end_line, end_column = (
                    node.end_mark.line + 1,
                    node.end_mark.column + 1,
                )
        self._add_object_to_path(
            path_id,
            model,
            current_object_paths,
            start_line,
            end_line,
            start_column,
            end_column,
        )

    def _update_sequence_node_objects(self, doc_id, model, current_object_paths, node, parent_name):
        for idx, item in enumerate(node.value):
            list_paths = self.add_list_item_object_path(
                getattr(model[idx], "name", None),  # type: ignore
                current_object_paths,
                idx,
            )
            self._update_objects(
                doc_id,
                model[idx],  # type: ignore[index]
                list_paths,
                item,
                parent_name=parent_name,
            )

    def _update_mapping_node_objects(self, doc_id, model, current_object_paths, node):
        for key_node, val_node in node.value:
            if isinstance(model, dict):
                model_value = model[key_node.value]
            else:
                model_fields = getattr(type(model), "model_fields", {})
                if key_node.value in model_fields:
                    field_name = key_node.value
                else:
                    try:
                        field_name = get_name_by_alias(type(model), key_node.value)
                    except KeyError:
                        field_name = key_node.value
                model_value = getattr(model, field_name, None)
            self._update_objects(
                doc_id,
                model_value,
                self.update_objects_path(current_object_paths, key_node.value),
                val_node,
                key_node.value,
            )

    def add_list_item_object_path(self, item_name, current_object_paths, idx):
        """
        Build the object path(s) for a single list item.

        Depending on :attr:`~flync.sdk.context.workspace_config.WorkspaceConfiguration.list_objects_mode`,
        the item may be registered under its numeric index, its name, or both:

        - :attr:`~flync.sdk.context.workspace_config.ListObjectsMode.INDEX`: appends the zero-based integer index as a path segment.
        - :attr:`~flync.sdk.context.workspace_config.ListObjectsMode.NAME`: appends ``item_name`` as an additional path segment when the name is
          non-empty. For external (folder-based) lists the name comes from the file/directory stem; for inline lists it comes from the model's
          ``name`` attribute.

        Both flags are active by default, so a list item is accessible under two IDs simultaneously (e.g. ``controllers.0`` and
        ``controllers.my_ctrl``).

        Args:
            item_name (str | None): Name of the list item, or ``None`` empty string when the item has no name.
            current_object_paths (list[str]): Parent path(s) to extend.
            idx (int): Zero-based position of the item in the list.

        Returns:
            list[str]: New list of object paths for this item.
        """
        idx_paths = []
        name_paths = []
        index_mode = ListObjectsMode.INDEX in self.configuration.list_objects_mode
        name_mode = ListObjectsMode.NAME in self.configuration.list_objects_mode
        if index_mode or not item_name:
            idx_paths += self.update_objects_path(current_object_paths, idx)
        if name_mode and item_name:
            name_paths += self.update_objects_path(current_object_paths, item_name)

        if name_mode and index_mode:
            if len(idx_paths) > 1:
                self._duplicated_objects_ids.setdefault(idx_paths[0], []).extend(idx_paths[1:] + name_paths)

            elif len(idx_paths) == 1 and name_paths:
                self._duplicated_objects_ids.setdefault(idx_paths[0], []).extend(name_paths)

        return idx_paths + name_paths

    def _add_object_to_path(
        self,
        doc_id: str,
        model,
        current_object_paths: list[str],
        start_line,
        end_line,
        start_column,
        end_column,
    ):
        """
        Register a model value and its source location for each given document id.

        Creates entries in :attr:`objects` and :attr:`sources` for every path in ``current_object_paths``. Skips paths that are already registered.

        Args:
            doc_id (str): document id of the document containing the object.
            model: The semantic object value to store.
            current_object_paths (list[str]): Dot-separated object ids to register.
            start_line (int): 1-based start line of the object in the document.
            end_line (int): 1-based end line of the object.
            start_column (int): 1-based start column.
            end_column (int): 1-based end column.
        """
        if not self.configuration.map_objects:
            return

        model_key = None if model is None else id(model)
        src_ref = SourceRef(doc_id, get_range(start_line, start_column, end_line, end_column))
        if self._duplicated_objects_ids and current_object_paths:
            common = [p for p in current_object_paths if p in self._duplicated_objects_ids]
            if common:
                current_object_paths = common

        for object_path in current_object_paths:
            object_id = ObjectId(object_path)
            if object_id in self.objects:
                return
            self.objects[object_id] = SemanticObject(object_id, model)
            self.sources[object_id] = src_ref
            if model_key is not None:
                self._model_to_object_ids.setdefault(model_key, []).append(object_id)
                if oids := self._duplicated_objects_ids.get(object_id):
                    self._model_to_object_ids[model_key].extend(oids)

    def _resolve_duplicate_object_id(self, oid: ObjectId):
        """
        Resolve a duplicate object ID into a fully registered object.

        This method checks whether the given `oid` exists in the duplicates mapping
        (`self._duplicated_objects_ids`). If found, it promotes the duplicate ID into
        `self.objects` and `self.sources` by copying the canonical object's model and source.

        Args
        ----------
        oid : ObjectId
            The object identifier to resolve from duplicates.
        """
        if oid in self.objects or not self._duplicated_objects_ids:
            return
        parent = oid.split(".")[0]
        for name_id, idx_ids in self._duplicated_objects_ids.items():
            if parent != name_id.split(".")[0] or oid not in idx_ids:
                continue

            model = self.objects[name_id].model
            source = self.sources[name_id]
            self.objects.update({dup: SemanticObject(oid, model) for dup in idx_ids})
            self.sources.update(dict.fromkeys(idx_ids, source))
            del self._duplicated_objects_ids[name_id]
            return

    def get_object(self, id: ObjectId) -> SemanticObject:
        """
        Retrieve a semantic object by its ObjectId.

        Args:
            id (ObjectId):
                Identifier of the semantic object.

        Returns:
            SemanticObject:
                The requested semantic object.
        """
        self._resolve_duplicate_object_id(id)
        return self.objects[id]

    def has_object(self, id: ObjectId) -> bool:
        """
        Checks if a specific key exists within a dictionary of objects.

        Args:
            id (ObjectId):
                Identifier of the semantic object.

        Returns:
            bool:
                True if the key is found, False otherwise.
        """
        self._resolve_duplicate_object_id(id)
        return id in self.objects

    def get_metadata(self, id: ObjectId) -> ObjectMetadata:
        """
        Retrieve metadata about a semantic object without exposing the full model.

        Provides type information, field details, relationships, and source location.
        The model itself is stored privately and not exposed in serialization.

        Args:
            id (ObjectId):
                Identifier of the semantic object.

        Returns:
            ObjectMetadata:
                Metadata object with type, fields, annotations, parents, children, and source.

        Raises:
            KeyError:
                If the object does not exist in the workspace.
        """

        semantic_obj = self.get_object(id)
        # ``FLYNCWorkspace`` is the only concrete subclass of this layer, and ObjectMetadata only
        # calls back into methods defined here, so the cast is safe.
        return ObjectMetadata(semantic_obj, cast("FLYNCWorkspace", self))

    def list_objects(self) -> list[ObjectId]:
        """
        Return a list of all ObjectIds present in the workspace.

        Returns:
            list[ObjectId]:
                List of object identifiers.
        """
        if not self._duplicated_objects_ids:
            return list(self.objects.keys())

        duplicated_ids = chain.from_iterable(self._duplicated_objects_ids.values())
        return list(chain(self.objects.keys(), duplicated_ids))

    def get_child_ids(self, id: ObjectId) -> list[str]:
        """
        Return the immediate child ObjectId strings of a given object.

        Backed by the ``_children_by_parent`` index built during object
        mapping, so this is an O(1) lookup rather than a full scan of the
        workspace.

        Args:
            id (ObjectId): Identifier of the parent object.

        Returns:
            list[str]: Immediate child id strings (empty if none / not mapped).
        """

        return [child for child in self._children_by_parent.get(str(id), []) if self.has_object(ObjectId(child))]

    def get_definition(self, object_id: ObjectId, field_name: str) -> Optional[ObjectId]:
        """
        Resolve and return definition identifiers for a given field reference.

        Args:
            object_id (ObjectId):
                Identifier of the semantic object.
            field_name (str)
                The field name of referencing object

        Returns:
            ObjectId
                A list of object identifiers that match the resolved reference criteria.
                The list may be empty if no definitions are found or if the field has no valid reference metadata.
        """

        sematic_obj: SemanticObject = self.get_object(object_id)
        def_obj = resolve_reference(sematic_obj.model, field_name)
        if not isinstance(def_obj, FLYNCBaseModel):
            return None
        so = self.get_semantic_object_from_model(def_obj)
        return so.id if so else None

    def get_references_of(self, object_id: ObjectId) -> list[ObjectId]:
        """
        Return all ObjectIds that reference the given object.

        Iterates over every semantic object in the workspace and checks whether any of its fields are defined by the same model as the target object.
        For each matching field, the concrete path to that field is collected via `find_path_from_field`.

        Args:
            object_id (ObjectId):
                The id of the object whose references should be found.

        Returns:
            list[ObjectId]:
                A list of ObjectIds representing all fields across the workspace that reference the given object.
        """

        refs: list[ObjectId] = []
        current_obj = self.get_object(object_id)

        for semantic_obj in self.objects.values():
            fields: dict | None = getattr(type(semantic_obj.model), "model_fields", None)
            if fields is None:
                continue

            for field, info in fields.items():
                if obj_id_def := self.get_definition(semantic_obj.id, field):
                    model_def = self.get_object(obj_id_def)
                    if model_def.model is current_obj.model:
                        self.find_path_from_field(object_id, refs, semantic_obj, field, info)
        return refs

    def find_path_from_field(
        self,
        object_id: ObjectId,
        refs: list[ObjectId],
        semantic_obj: SemanticObject,
        field: str,
        info: FieldInfo,
    ):
        """
        Resolve the concrete ObjectId path for a field and append it to refs.

        Tries to build the path as `<semantic_obj.id>.<field>`, falling back to `<semantic_obj.id>.<info.alias>` when the first candidate is not
        present in the workspace. Raises if neither candidate exists.

        Args:
            object_id (ObjectId):
                The id of the target object being referenced (used in the error message when the path cannot be resolved).
            refs (list[ObjectId]):
                Accumulator list to which the resolved path is appended.
            semantic_obj (SemanticObject):
                The semantic object that owns the field being inspected.
            field (str):
                The field name on `semantic_obj`'s model.
            info:
                The Pydantic `FieldInfo` for the field, used to access the field alias as a fallback path segment.

        Raises:
            ValueError:
                If neither the field name nor its alias resolves to a known object in the workspace.
        """

        path_candidate = ObjectId(f"{semantic_obj.id}.{field}")
        if not self.has_object(path_candidate):
            path_candidate = ObjectId(f"{semantic_obj.id}.{info.alias}")
        if not self.has_object(path_candidate):
            raise ValueError(
                "object with path {} not found in map",
                object_id,
            )
        refs.append(path_candidate)

    def get_semantic_objects_ids_from_model(self, model: FLYNCBaseModel) -> list[ObjectId]:
        """
        Find and return ObjectIds for all semantic objects that correspond to a model.

        A single model instance may be registered under multiple ObjectIds (e.g., a list item
        indexed by both numeric position and name).

        Args:
            model (FLYNCBaseModel):
                Validated Flync model.

        Returns:
            list[ObjectId]:
                List of ObjectIds that correspond to the Flync object. Empty if none found.
        """

        return self._model_to_object_ids.get(id(model), []).copy()

    def get_semantic_objects_from_model(self, model: FLYNCBaseModel) -> list[SemanticObject]:
        """
        Find and return all semantic objects that correspond to a validated Flync object.

        A single model instance may be registered under multiple ObjectIds (e.g., a list item
        indexed by both numeric position and name).

        Args:
            model (FLYNCBaseModel):
                Validated Flync model.

        Returns:
            list[SemanticObject]:
                List of semantic objects that correspond to the Flync object. Empty if none found.
        """

        return [self.get_object(oid) for oid in self._model_to_object_ids.get(id(model), [])]

    @deprecated("Use get_semantic_objects_from_model() instead, which returns all matches")
    def get_semantic_object_from_model(self, model: FLYNCBaseModel) -> SemanticObject | None:
        """
        Find and return the first semantic object that corresponds to a validated Flync object.

        This method maintains backward compatibility by returning only the first result.

        Args:
            model (FLYNCBaseModel):
                Validated Flync model.

        Returns:
            SemanticObject | None:
                First semantic object that corresponds to Flync object, or None if not found.
        """

        results = self.get_semantic_objects_from_model(model)
        return results[0] if results else None

    def get_source(self, id: ObjectId) -> SourceRef:
        """
        Retrieve the source reference for a given ObjectId.

        Args:
            id (ObjectId):
                Identifier of the object.

        Returns:
            SourceRef:
                The source reference associated with the object.
        """
        self._resolve_duplicate_object_id(id)
        return self.sources[id]

    def objects_at(self, uri: str, line: int, character: int) -> list[ObjectId]:
        """
        Return the list of ObjectIds located at the specified position in a document.

        Args:
            uri (str):
                Document URI.
            line (int):
                1-based line number, consistent with the :class:`~flync.sdk.workspace.source.Position` values stored during YAML parsing.
            character (int):
                1-based character offset within the line.

        Returns:
            list[ObjectId]:
                List of object identifiers at the given position.
        """

        result = []
        for oid, src in self.sources.items():
            if src.uri != uri:
                continue
            r = src.range
            if (line > r.start.line or (line == r.start.line and character >= r.start.character)) and (
                line < r.end.line or (line == r.end.line and character <= r.end.character)
            ):
                result.append(oid)
        return result

    def revalidate_references_of(self, id: ObjectId) -> None:
        """
        Revalidate all reference relationships for a given semantic object and
        propagate changes upward through its parent objects.

        This method ensures that when a model is modified at runtime, every
        field in the workspace that references it is brought up to date. It
        performs a breadth-first walk of the reference graph:

        1. Collects all references pointing to the changed object.
        2. For each referencing parent, reads ``changed_model.<source_key>``
           (default: ``"name"``) to obtain the new scalar value for the
           reference field and writes it onto the parent via ``setattr``.
        3. Re-validates the parent so that any ``@model_validator`` or
           ``@field_validator`` methods are re-run.
        4. Enqueues the parent's own parent for further re-validation so
           that changes cascade upward through the hierarchy.

        Once all affected objects have been re-validated, every document that
        changed is serialized back to disk.

        Args:
            id (ObjectId): The ObjectId of the changed object whose references
                should be updated.
        """
        model_changed: SemanticObject = self.get_object(id)
        need_revalidate = self.get_references_of(id)
        affected_files: set[str] = set()
        affected_files.add(self.get_source(id).uri)
        revalidated_objects: set[ObjectId] = set()
        objects_to_revalidate: list[ObjectId] = list(need_revalidate)

        while objects_to_revalidate:
            nr = objects_to_revalidate.pop(0)
            if nr in revalidated_objects:
                continue

            field_obj_ref: SemanticObject = self.get_object(nr)
            meta = self.get_metadata(nr)
            parent_id = meta.parent_id
            child_field_name = meta.name
            if parent_id is None or child_field_name is None or not self.has_object(ObjectId(parent_id)):
                continue

            parent_id = ObjectId(parent_id)
            parent_obj_ref: SemanticObject = self.get_object(parent_id)
            if not isinstance(parent_obj_ref.model, FLYNCBaseModel):
                continue

            field_name = get_field_name_from_alias(type(parent_obj_ref.model), child_field_name)
            field_info = type(parent_obj_ref.model).model_fields[field_name]

            if ref := get_metadata(field_info.metadata, Reference):
                parent_source = self.get_source(parent_id)
                try:
                    # Read source_key (default "name") from the changed model
                    # to obtain the new scalar value for the parent's reference
                    # field (e.g. the new name of the referenced object).
                    new_value = getattr(model_changed.model, ref.source_key)
                    field_obj_ref.model = new_value
                    setattr(parent_obj_ref.model, field_name, new_value)

                except ValidationError as e:
                    merged_errors = self.documents_diags[parent_source.uri] + e.errors()
                    self.documents_diags[parent_source.uri] = get_unique_errors(merged_errors)

                affected_files.add(parent_source.uri)

                self.__revalidate_model(parent_id, parent_obj_ref)
                revalidated_objects.add(nr)

                # Always walk up to parent, even if parent has no direct references
                # Parent might have @model_validator or @field_validator
                self.__enqueue_parent_for_revalidation(parent_id, objects_to_revalidate, revalidated_objects)

        for affected_file in affected_files:
            self.__persist_document_changes(affected_file)

    def __enqueue_parent_for_revalidation(
        self,
        object_id: ObjectId,
        queue: list[ObjectId],
        revalidated: set[ObjectId],
    ) -> None:
        """
        Add an object's parent to the revalidation queue if it has validators.

        Walks up the hierarchy to find parents with custom validators
        (@model_validator or @field_validator).

        Args:
            object_id (ObjectId): The object whose parent to check.
            queue (list[ObjectId]): The revalidation queue (updated in place).
            revalidated (set[ObjectId]): Already-revalidated objects (to avoid cycles).
        """
        parent_id: ObjectId
        while True:
            meta_parent_id = self.get_metadata(object_id).parent_id
            if meta_parent_id is None or ObjectId(meta_parent_id) in revalidated:
                break
            parent_id = ObjectId(meta_parent_id)
            if not self.has_object(parent_id):
                continue

            parent_obj = self.get_object(parent_id)
            if isinstance(parent_obj.model, FLYNCBaseModel) and has_validators(type(parent_obj.model)):
                if parent_id not in queue and parent_id not in revalidated:
                    queue.append(parent_id)
                    break
            object_id = parent_id
            if object_id == "":
                break

    def __revalidate_model(self, object_id: ObjectId, semantic_obj: SemanticObject) -> None:
        """
        Revalidate a single semantic object and update its diagnostics.

        Dumps the model to a dict via ``dump_model_with_discriminators`` and
        runs ``validate_with_policy`` to re-check all constraints — including
        any ``@model_validator`` or ``@field_validator`` decorated methods.
        New errors are merged into the document's diagnostics store.

        Args:
            object_id (ObjectId): The ID of the object being revalidated.
            semantic_obj (SemanticObject): The semantic object wrapper.
        """
        if not isinstance(semantic_obj.model, FLYNCBaseModel):
            return

        parent_type = type(semantic_obj.model)
        source_ref = self.get_source(object_id)
        model_dict = dump_model_with_discriminators(semantic_obj.model, exclude_unset=self.configuration.exclude_unset)
        doc_id = source_ref.uri
        _, errors = validate_with_policy(parent_type, model_dict, Path(doc_id).as_posix())
        # Update diagnostics for this document
        merged_errors = self.documents_diags[doc_id] + errors
        self.documents_diags[doc_id] = get_unique_errors(merged_errors)

    def __persist_document_changes(self, uri: str) -> None:
        """
        Serialize and write a document's top-level model to disk.

        Finds the root object(s) for a document, serializes them with all fields
        (including external fields), and writes to disk in FLYNC file format.

        Args:
            uri (str): The document URI (workspace-relative path).
        """
        root_id = min([obj_id for obj_id, src in self.sources.items() if src.uri == uri], key=lambda x: x.count("."))
        while True:
            root_object: SemanticObject | None
            if self.has_object(root_id):
                root_object = self.get_object(root_id)
            if root_object is None or not isinstance(root_object.model, FLYNCBaseModel):
                if (parent_id := self.get_metadata(root_id).parent_id) is None:
                    break
                root_id = ObjectId(parent_id)
                continue

            cast("FLYNCWorkspace", self).load_flync_model(
                root_object.model,
                self.get_source(root_id).uri.removesuffix(self.configuration.flync_file_extension),
            )
            break
