"""
Incremental single-document reload for the FLYNC workspace.

Reconciles the workspace with one changed, added or removed document by re-validating only the
affected node's ancestor spine, falling back to a full reload when that is not possible.
"""

import logging
from pathlib import Path
from typing import Optional, cast

from flync.core.annotations import External, Implied
from flync.sdk.utils.field_utils import get_metadata
from flync.sdk.utils.sdk_types import PathType

from ._base import LoadNode, ParentLink
from ._loading import _WorkspaceLoading
from .document import Document, read_file
from .ids import ObjectId
from .objects import SemanticObject

logger = logging.getLogger(__name__)


class _WorkspaceIncremental(_WorkspaceLoading):
    """Partial reload of a single document, with full-reload fallbacks."""

    def update_document(self, uri: PathType) -> list[str]:
        """
        Incrementally reconcile the workspace with a single document that changed on disk.

        Three cases are handled in place, all re-validating the affected node's ancestor spine up to the
        root while reusing the already-loaded sibling models (so cross-document references, resolved by
        ancestor validators, stay correct) without a full :meth:`load_workspace`:

        - **changed** (known document, still on disk): reload just that node and its subtree.
        - **added** (unknown document, now on disk): reload the nearest existing ancestor, whose
          directory scan picks up the new file.
        - **removed** (known document, gone from disk): reload the parent, whose scan drops the file.

        New or removed items directly under the root, and any change that breaks a root-resolved
        reference, fall back to :meth:`_revalidate_all` (a full re-validation from the cached ASTs, no
        disk re-read). An unexpected failure falls back to a from-disk :meth:`_reset_and_reload`.

        Args:
            uri (PathType): Path of the affected document, absolute or workspace-relative.

        Returns:
            list[str]: The ids of every document whose model or diagnostics were recomputed.
        """

        uri = Document.normalize_uri(uri, self.workspace_root)
        target = self.workspace_root / uri if self.workspace_root else Path(uri)
        node = self._doc_index.get(uri)
        exists = target.exists()
        try:
            if node is not None and exists:
                result = self._reload_document(node, uri)
            elif node is None and exists:
                result = self._add_document(uri)
            elif node is not None:
                result = self._remove_document(node)
            else:
                result = self._reset_and_reload()
            return result
        except Exception:
            logger.exception("update_document: partial update of %s failed, reloading workspace", uri)
            return self._reset_and_reload()

    def _add_document(self, uri: str) -> list[str]:
        """
        Bring a newly created document into the workspace.

        The nearest existing ancestor load-node is reloaded; its directory scan picks up the new file and
        the spine above it is re-validated. A new item directly under the root (e.g. a whole new ECU)
        has no partial ancestor, so it falls back to a full reload.

        Args:
            uri (str): Workspace-relative id of the new document.

        Returns:
            list[str]: Ids of every document that was recomputed.
        """

        ancestor = self._nearest_ancestor_node(uri)
        if ancestor is None or ancestor.link is None:
            return self._revalidate_all()
        return self._reload_from_node(ancestor)

    def _remove_document(self, node: LoadNode) -> list[str]:
        """
        Drop a document that no longer exists on disk.

        The parent load-node is reloaded; its directory scan no longer sees the file, so the item drops
        out of the model, and the spine above is re-validated. Removing the root falls back to a full
        reload.

        Args:
            node (LoadNode): The indexed node for the removed document.

        Returns:
            list[str]: Ids of every document that was recomputed.
        """

        if node.link is None:
            return self._revalidate_all()
        parent = self._doc_index.get(self.document_id_from_path(str(node.link.parent_path)))
        if parent is None:
            return self._revalidate_all()
        return self._reload_from_node(parent)

    def _nearest_ancestor_node(self, uri: str) -> Optional[LoadNode]:
        """
        Return the closest indexed load-node that contains ``uri`` in its directory tree.

        Walks up the path parents until an indexed node is found (the root always matches once loaded).

        Args:
            uri (str): Workspace-relative id to locate.

        Returns:
            LoadNode | None: The nearest ancestor node, or ``None`` if none is indexed.
        """

        current = Path(uri).parent
        while True:
            doc_id = current.as_posix()
            ancestor = self._doc_index.get(doc_id)
            if ancestor is not None:
                return ancestor
            if doc_id in (".", ""):
                return None
            current = current.parent

    def _reload_document(self, node: LoadNode, uri: str) -> list[str]:
        """
        Perform the in-place reload of ``node`` and re-validate its ancestor spine.

        Args:
            node (LoadNode): The indexed node for the changed document.
            uri (str): Workspace-relative id of the changed document.

        Returns:
            list[str]: Ids of all documents that were recomputed (leaf subtree + ancestors).
        """

        text = read_file(self.workspace_root / uri)  # type: ignore[operator]
        if uri in self.documents:
            self.documents[uri].update_text(text)
        else:
            doc = Document(uri, text, self.configuration.map_objects)
            doc.parse()
            self.documents[uri] = doc

        return self._reload_from_node(node)

    def _reload_from_node(self, node: LoadNode) -> list[str]:
        """
        Reload ``node``'s subtree and re-validate every ancestor up to the root.

        Shared by every partial-update flavour: a changed document reloads its own node, while a new or
        removed document reloads the nearest surviving ancestor (whose directory scan then picks up or
        drops the affected file).

        Args:
            node (LoadNode): The node whose subtree is reloaded first.

        Returns:
            list[str]: Ids of all documents that were recomputed.
        """

        affected: list[str] = []
        cur_child, ids = self._reload_subtree(node)
        affected += ids

        cur_link = self._doc_index[node.doc_id].link
        while cur_link is not None:
            parent_id = self.document_id_from_path(str(cur_link.parent_path))
            parent_node = self._doc_index.get(parent_id)
            if parent_node is None:
                raise ValueError(f"missing parent node for {cur_link.parent_path}")
            rebuilt = self._rebuild_ancestor(parent_node, cur_link, cur_child)
            if rebuilt is None:
                # Reusing sibling instances is cheap but leaves error-recovery unable to prune a bad
                # value out of a reused child. When that happens, re-validate this ancestor's whole
                # subtree (from the cached ASTs, no disk read) so the policy pruning behaves as it does
                # on a full load. The root's subtree is the whole workspace, so revalidate everything.
                if parent_node.link is None:
                    return self._revalidate_all()
                rebuilt, ids = self._reload_subtree(parent_node)
                affected += ids
            affected.append(parent_id)
            cur_child = rebuilt
            cur_link = self._doc_index[parent_id].link
        self.flync_model = cur_child  # type: ignore[assignment]

        return list(dict.fromkeys(affected))

    def _reload_subtree(self, node: LoadNode) -> tuple[object, list[str]]:
        """
        Drop and re-load ``node`` together with everything indexed underneath it.

        Args:
            node (LoadNode): Root of the subtree to reload.

        Returns:
            tuple[object, list[str]]: The reloaded model value and the ids of the documents touched.
        """

        before = [sub.doc_id for sub in self._subtree_nodes(node)]
        for doc_id in before:
            self.documents_diags.pop(doc_id, None)
            if doc_id != node.doc_id:
                self._doc_index.pop(doc_id, None)
            # drop a cached document whose file has been deleted so the rescan does not resurrect it
            if doc_id in self.documents and not (self.workspace_root / doc_id).exists():  # type: ignore[operator]
                self.documents.pop(doc_id, None)
        if self.configuration.map_objects:
            self._purge_object_subtree(node.object_paths)
        model = self._load_from_path(
            node.path,
            node.current_type,
            node.current_type_name,
            list(node.object_paths),
            node.link,
        )
        # union with the post-reload subtree so newly added documents are reported too
        after = [sub.doc_id for sub in self._subtree_nodes(node)]
        return model, list(dict.fromkeys(before + after))

    def _rebuild_ancestor(self, parent_node: LoadNode, link: ParentLink, new_child):
        """
        Re-validate ``parent_node`` with its changed child replaced, reusing every other child instance.

        The parent's own inline file is re-read but the unchanged external children are taken straight
        from the previously loaded parent instance, so validation only re-runs the parent's own
        validators (including reference binding) instead of re-parsing sibling documents.

        Args:
            parent_node (LoadNode): The ancestor being rebuilt.
            link (ParentLink): How ``new_child`` attaches to this ancestor.
            new_child: The freshly loaded value for ``link.field_name``.

        Returns:
            The rebuilt (and parent-normalized) ancestor model.
        """

        old_parent = parent_node.model
        parent_type = parent_node.current_type
        new_branch = self._splice_branch(old_parent, link, new_child)
        module_load_info = self._ancestor_load_info(parent_node, old_parent, link, new_branch)

        new_parent = self._validate_node(parent_node, module_load_info)
        if self.configuration.map_objects and old_parent is not None:
            if new_parent is not None:
                self._remap_ancestor_objects(parent_type, old_parent, new_parent)
            else:
                self._detach_object(parent_node)
        return new_parent

    def _ancestor_load_info(self, parent_node: LoadNode, old_parent, link: ParentLink, new_branch) -> dict:
        """
        Gather the field data to re-validate ``parent_node`` with ``link.field_name`` replaced.

        Unchanged external children are reused straight from ``old_parent`` (so they are not re-read or
        re-validated), implied fields are recomputed from the path, and the parent's own inline file is
        merged back in.
        """

        module_load_info: dict = {}
        for field_name, field_info in parent_node.current_type.model_fields.items():
            if field_name == link.field_name:
                module_load_info[field_name] = new_branch
                continue
            if get_metadata(field_info.metadata, External) is not None:
                value = getattr(old_parent, field_name, None)
                if value is not None:
                    module_load_info[field_name] = value
                continue
            implied = get_metadata(field_info.metadata, Implied)
            if implied is not None:
                self._handle_implied_field_load(parent_node.path, module_load_info, field_name, implied)
        self._append_to_info_dict(parent_node.path, module_load_info)
        return module_load_info

    def _detach_object(self, node: LoadNode) -> None:
        """Clear the object-map entry of a node whose model failed to rebuild, so it holds no stale model."""
        if not node.object_paths:
            return
        oid = ObjectId(node.object_paths[0].strip("."))
        semantic = self.objects.get(oid)
        if semantic is None or semantic.model is None:
            return
        self._unmap_object_id(semantic.model, oid)
        cast(SemanticObject, semantic).model = None  # type: ignore[assignment]

    @staticmethod
    def _splice_branch(old_parent, link: ParentLink, new_child):
        """Build the new value for ``link.field_name`` with ``new_child`` put in the right slot."""
        if link.container == "list":
            new_list = list(getattr(old_parent, link.field_name, None) or [])
            if isinstance(link.key, int) and 0 <= link.key < len(new_list):
                new_list[link.key] = new_child
            else:
                new_list.append(new_child)
            return new_list
        if link.container == "dict":
            new_dict = dict(getattr(old_parent, link.field_name, None) or {})
            new_dict[link.key] = new_child
            return new_dict
        return new_child

    def _subtree_nodes(self, node: LoadNode) -> list[LoadNode]:
        """Return ``node`` and every indexed node whose object path sits underneath it."""
        own = {p.strip(".") for p in node.object_paths}
        own.discard("")
        prefixes = tuple(f"{p}." for p in own)
        result = []
        for candidate in self._doc_index.values():
            ids = {p.strip(".") for p in candidate.object_paths}
            if ids & own or (prefixes and any(cid.startswith(prefixes) for cid in ids)):
                result.append(candidate)
        return result

    def _purge_object_subtree(self, object_paths: list[str]) -> None:
        """
        Remove all object-map entries under ``object_paths`` so a reload can register them fresh.

        The node's own ids are dropped from :attr:`objects`/:attr:`sources` (the reload re-creates them
        with the new model) but kept in the parent's child index, since the parent is not reloaded and
        would otherwise lose the edge to this node.
        """

        own = {p.strip(".") for p in object_paths}
        own.discard("")
        prefixes = tuple(f"{p}." for p in own)

        def in_subtree(oid: str) -> bool:
            """Utility function to check if object id is in the subtree."""
            return oid in own or (bool(prefixes) and oid.startswith(prefixes))

        def is_descendant(oid: str) -> bool:
            """Utility function to check if object id is descendant in the path."""
            return bool(prefixes) and oid.startswith(prefixes)

        # Use self.objects keys directly — list_objects() includes virtual
        # aliases from _duplicated_objects_ids.values() that are not actual keys
        # in self.objects and would cause a KeyError on pop.
        real_objs = [o for o in self.objects if in_subtree(str(o))]
        for oid in real_objs:
            semantic = self.objects.pop(oid)
            self.sources.pop(oid, None)
            self._unmap_object_id(semantic.model, oid)
        if self._duplicated_objects_ids:
            for oid in [o for o in self.list_objects() if in_subtree(str(o))]:
                self._duplicated_objects_ids.pop(oid, None)

        for pid in [p for p in self._children_by_parent if in_subtree(p)]:
            self._children_by_parent.pop(pid, None)
        for parent_id, children in self._children_by_parent.items():
            self._children_by_parent[parent_id] = [c for c in children if not is_descendant(c)]
        self._linked_child_ids = {c for c in self._linked_child_ids if not is_descendant(c)}

    def _remap_ancestor_objects(self, parent_type, old_parent, new_parent) -> None:
        """Point the ancestor's (and its external container fields') object entries at the rebuilt instances."""
        self._remap_model_ids(old_parent, new_parent)
        for field_name, field_info in parent_type.model_fields.items():
            if get_metadata(field_info.metadata, External) is None:
                continue
            old_value = getattr(old_parent, field_name, None)
            new_value = getattr(new_parent, field_name, None)
            if isinstance(old_value, (list, dict)) and old_value is not new_value:
                self._remap_model_ids(old_value, new_value)

    def _unmap_object_id(self, model, oid: ObjectId) -> None:
        """Remove a single object id from the ``model -> ids`` reverse index."""
        if model is None:
            return
        ids = self._model_to_object_ids.get(id(model))
        if ids is None:
            return
        remaining = [i for i in ids if i != oid]
        if remaining:
            self._model_to_object_ids[id(model)] = remaining
        else:
            self._model_to_object_ids.pop(id(model), None)

    def _remap_model_ids(self, old_model, new_model) -> None:
        """Move the object ids registered for ``old_model`` onto ``new_model``."""
        ids = self._model_to_object_ids.pop(id(old_model), None)
        if not ids:
            return
        for oid in ids:
            semantic = self.objects.get(oid)
            if semantic is not None:
                semantic.model = new_model
        self._model_to_object_ids[id(new_model)] = ids

    def _clear_derived_state(self) -> None:
        """Drop the model, object graph and reload index, keeping (or not) the parsed documents."""
        self.documents_diags.clear()
        self.objects.clear()
        self.sources.clear()
        self._model_to_object_ids.clear()
        self._children_by_parent.clear()
        self._linked_child_ids.clear()
        self._doc_index.clear()
        self._duplicated_objects_ids.clear()

    def _revalidate_all(self) -> list[str]:
        """
        Rebuild the whole model and object graph from the already-parsed documents.

        Used when a partial update cannot be completed in place (e.g. a change breaks a root-resolved
        reference, or a new item lands directly under the root). Unlike :meth:`_reset_and_reload` it does
        not re-read or re-parse files through the process pool - it reuses the in-memory document ASTs
        that ``update_document`` keeps in sync, reading from disk only files not seen before (a newly
        created document). This makes the fallback markedly cheaper than a cold load.
        """

        self._clear_derived_state()
        self.flync_model = self._load_from_path(self.workspace_root)  # type: ignore[arg-type]
        return list(self.documents_diags.keys())

    def _reset_and_reload(self) -> list[str]:
        """Clear all derived state and reload the workspace from disk (the last-resort safety path)."""
        self.documents.clear()
        self._clear_derived_state()
        self._open_documents()
        self.flync_model = self._load_from_path(self.workspace_root)  # type: ignore[arg-type]
        return list(self.documents_diags.keys())
