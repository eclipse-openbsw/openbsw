"""
Model dependency graph utilities for the FLYNC SDK.

Provides functions and classes to build, traverse, and query the dependency graph between Pydantic models that make up a FLYNC workspace.
"""

import hashlib
import importlib
import json
import shelve
import time
import types
from functools import lru_cache
from os import listdir, makedirs, remove, stat, walk
from os.path import abspath, dirname, join
from types import NoneType
from typing import Annotated, Literal, Union, get_args, get_origin
from weakref import WeakKeyDictionary

import platformdirs
from filelock import FileLock
from pydantic import BaseModel

from flync.core.annotations import External, Implied, OutputStrategy, Reference
from flync.sdk.context.node_info import NodeInfo

from .field_utils import get_metadata


def _collect_union_options(args):
    """Return container-model dicts for each non-None union member type."""
    models = []
    for arg in args:
        result = extract_container_model(arg)
        if result:
            models.append(result)
    return models


def extract_container_model(annotation):  # noqa
    """
    Recursively extract ``BaseModel`` types from nested container annotations.

    Handles ``list[Model]``, ``dict[str, Model]``,
    ``list[dict[str, Model]]``,
    ``Union[A, B]``, and bare ``BaseModel`` subclasses.

    Args:
        annotation: A Python type annotation to inspect.

    Returns:
        dict | None: A dict describing the container shape, or ``None`` when
        no ``BaseModel`` type is reachable. The dict always has a ``"container"``
        key set to ``"list"``, ``"dict"``, ``"union"``, or ``"model"``.
    """

    if get_origin(annotation) is Annotated:
        annotation = get_args(annotation)[0]

    if isinstance(annotation, type) and issubclass(annotation, BaseModel):
        return {"container": "model", "model": annotation}

    origin = get_origin(annotation)
    args = get_args(annotation)
    result = None
    if origin is list:
        items = extract_container_model(args[0])
        if items:
            result = {"container": "list", "items": items}
    elif origin is dict:
        values = extract_container_model(args[1])
        if values:
            result = {"container": "dict", "keys": args[0], "values": values}
    elif origin in (Union, types.UnionType):
        models = _collect_union_options(args)
        if models:
            result = {"container": "union", "options": models}

    return result


def unwrap_annotated(annotation):
    """
    Recursively strip ``Annotated`` wrappers from a type annotation.

    Args:
        annotation: A Python type annotation, potentially wrapped in one or more ``Annotated[T, ...]`` layers.

    Returns:
        tuple[type, list]: A ``(clean_type, metadata)`` pair where ``clean_type`` is the innermost unwrapped type and ``metadata`` is
        the flat list of all collected annotation arguments.
    """

    metadata = []

    while get_origin(annotation) is Annotated:
        args = get_args(annotation)
        annotation = args[0]
        metadata.extend(args[1:])

    return annotation, metadata


@lru_cache(maxsize=None)
def model_force_rebuild(model: type[BaseModel]):
    """Force rebuild of a Pydantic model class, regenerating schema and validators.
    Cached with lru_cache to avoid redundant rebuilds for the same model."""
    return model.model_rebuild(force=True)


@lru_cache(maxsize=None)
def extract_model_dependencies(model: type[BaseModel]) -> dict:
    """
    Return the cached dependency tree for a Pydantic model.

    Results are memoized so the graph is only built once per model class.

    Args:
        model (type[BaseModel]): The model to inspect.

    Returns:
        dict: A dependency tree mapping field names to their structure and external annotation metadata.
    """

    return _extract_model_dependencies(model, visited=set())


def _extract_model_dependencies(model: type[BaseModel], visited: set[type[BaseModel]]) -> dict:
    """
    Recursively extract model dependencies, guarding against cycles.

    Args:
        model (type[BaseModel]): The model being inspected.
        visited (set[type[BaseModel]]): Models currently on the call stack, used to detect and break circular references.

    Returns:
        dict: Dependency structure for the model's fields, or ``{"__cycle__": True}`` when a cycle is detected.
    """

    # ---- CYCLE BREAKER ----
    if model in visited:
        return {"__cycle__": True}
    visited.add(model)
    model_force_rebuild(model)

    deps = {}

    for name, field in model.model_fields.items():
        annotation, _ = unwrap_annotated(field.annotation)
        container_info = extract_container_model(annotation)

        if container_info:
            external = get_metadata(field.metadata, External)
            reference = get_metadata(field.metadata, Reference)
            implied = get_metadata(field.metadata, Implied)
            if reference or implied:
                continue
            structure_info = build_dependency_structure(container_info, visited)
            deps[name] = {
                "external": external,
                "structure": structure_info,
            }
    visited.remove(model)
    return deps


def build_dependency_structure(info, visited):
    """
    Convert extracted container info into a resolved dependency tree.

    Recursively replaces ``"model"`` leaf entries with their full child dependency trees as returned by :func:`_extract_model_dependencies`.

    Args:
        info (dict): A container-shape dict as produced by
            :func:`extract_container_model`.
        visited (set[type[BaseModel]]): Models currently on the call stack,
            forwarded to :func:`_extract_model_dependencies` for cycle detection.

    Returns:
        dict: A dependency tree node with a ``"type"`` key and additional keys depending on the container kind (``"items"``, ``"values"``,
        ``"options"``, or ``"children"``).
    """
    container = info["container"]

    if container == "model":
        model = info["model"]
        return {
            "type": model,
            "children": _extract_model_dependencies(model, visited),
        }

    builders = {
        "list": lambda: {
            "type": "list",
            "items": build_dependency_structure(info["items"], visited),
        },
        "dict": lambda: {
            "type": "dict",
            "keys": str(info["keys"]),
            "values": build_dependency_structure(info["values"], visited),
        },
        "union": lambda: {
            "type": "union",
            "options": [build_dependency_structure(opt, visited) for opt in info["options"]],
        },
    }
    return builders[container]()


def walk_structure(parent_model, structure, edges, visited=None):
    """
    Walk a dependency structure and collect directed edges between models.

    Traverses container types (list, dict, union) recursively until reaching concrete model types, then records a directed edge from ``parent_model``
    to each discovered child model.

    Args:
        parent_model: The parent model class from which the walk started.
        structure (dict): A dependency structure dict as returned by
            :func:`build_dependency_structure`.
        edges (set): Mutable set accumulating ``(parent, child)`` model pairs.
        visited (set | None): Models currently being traversed; used to avoid
            infinite recursion on cycles.
    """

    if visited is None:
        visited = set()
    if structure["type"] == "list":
        walk_structure(parent_model, structure["items"], edges, visited=visited)

    elif structure["type"] == "dict":
        walk_structure(parent_model, structure["values"], edges, visited=visited)

    elif structure["type"] == "union":
        for option in structure["options"]:
            walk_structure(parent_model, option, edges, visited=visited)

    else:
        child_model = structure["type"]  # already class
        if child_model in visited:
            return
        visited.add(child_model)
        edges.add((parent_model, child_model))

        for f in extract_model_dependencies(child_model).values():
            walk_structure(child_model, f["structure"], edges, visited=visited)

        visited.remove(child_model)


def collect_edges(model: type[BaseModel], edges=None):
    """
    Collect all directed dependency edges reachable from a root model.

    Args:
        model (type[BaseModel]): The root model to start from.
        edges (set | None): Optional pre-existing edge set to accumulate into.

    Returns:
        tuple[set, dict]: A tuple of ``(edges, deps)`` where ``edges`` is the
        set of ``(parent, child)`` model pairs and ``deps`` is the raw
        dependency tree for ``model``.
    """

    if edges is None:
        edges = set()

    deps = extract_model_dependencies(model)

    for field in deps.values():
        walk_structure(model, field["structure"], edges)

    return edges, deps


class ModelDependencyGraph:
    """
    Dependency graph for a hierarchy of Pydantic models.

    Builds and exposes a directed graph of model relationships, enabling look-ups of parent–child relationships and path resolution.

    Attributes:
        root (type[BaseModel]): The root model of the graph.
        edges (set): Set of ``(parent, child)`` model-class pairs.
        tree (dict): Raw dependency tree rooted at ``root``.
        reverse_tree (dict[type, set[type]]): Inverted graph mapping each child model to its set of parent models.
        fields_info (dict[str, NodeInfo]): Metadata for every node in the graph, keyed by class name.
    """

    def __init__(self, root: type[BaseModel]):
        """
        Build the dependency graph for the given root model.

        Args:
            root (type[BaseModel]): The root Pydantic model class.
        """

        self.root = root
        self.edges, self.tree = collect_edges(root)
        self.reverse_tree: dict[type[BaseModel], set[type[BaseModel]]] = self._invert()
        self.fields_info: dict[str, NodeInfo] = self._field_info()

    def _invert(self):
        """
        Invert the edge set to produce a child-to-parents mapping.

        Returns:
            dict[type, set[type]]: Mapping of each child model class to the set of parent model classes that reference it.
        """

        reverse: dict[type[BaseModel], set[type[BaseModel]]] = {}
        for p, c in self.edges:
            if c not in reverse:
                reverse[c] = set()
            reverse[c].add(p)
        return dict(reverse)

    def _field_info(self):
        """
        Build per-node metadata including all paths from the root model.

        Walks the full dependency tree starting from ``self.root`` and records, for every reachable model class, the dot-separated paths
        through which it can be reached.

        Returns:
            dict[str, NodeInfo]: Mapping of class names to :class:`NodeInfo` objects. Each ``NodeInfo`` carries the Python type and the list
            of dot-separated FLYNC paths leading to it.
        """
        field_info: dict[str, NodeInfo] = {}
        # add root to info
        root_discriminators = {
            field_name for field_name, field_info_obj in self.root.model_fields.items() if get_origin(field_info_obj.annotation) is Literal
        }
        field_info[self.root.__name__] = NodeInfo(self.root.__name__, self.root, discriminator_fields=root_discriminators)
        self._walk_fields(self.root, self.tree, field_info)
        return dict(field_info)

    def _walk_fields(self, current_model, subtree, field_info, path=()):
        """
        Iterate over a model's dependency subtree and dispatch to :meth:`_resolve_structure`.

        Args:
            current_model: The Pydantic model class whose fields are being walked.
            subtree (dict): The dependency tree for ``current_model`` as returned by :func:`_extract_model_dependencies`.
            field_info (dict): Mutable mapping of class names to :class:`NodeInfo` objects being accumulated.
            path (tuple): Accumulated path segments from the root to the current position.
        """
        for field_name, info in subtree.items():
            if field_name == "__cycle__":  # found a cyclic item
                continue
            self._resolve_structure(current_model, field_name, info["structure"], field_info, path, ())

    def _resolve_structure(self, parent_model, field_name, structure, field_info, path, container_chain):
        """
        Recursively resolve a single field's structure and record its path.

        Descends through ``"list"``, ``"dict"``, and ``"union"`` container nodes, accumulating container kinds in ``container_chain``, until a
        ``BaseModel`` leaf is reached.  At that point the leaf is recorded in ``field_info`` and the walk continues into its children.

        Args:
            parent_model: The Pydantic model class that owns the field.
            field_name (str): The attribute name of the field on ``parent_model``.
            structure (dict): The dependency structure node to resolve.
            field_info (dict): Mutable mapping of class names to :class:`NodeInfo` objects being accumulated.
            path (tuple): Path segments accumulated from the root to the current node.
            container_chain (tuple): Container kinds wrapping the current field, used to produce ``[]`` / ``{}`` path segments.
        """
        t = structure["type"]

        if t in ("list", "dict"):
            child_key = "items" if t == "list" else "values"
            self._resolve_structure(parent_model, field_name, structure[child_key], field_info, path, container_chain + (t,))
            return

        if t == "union":
            for option in structure["options"]:
                self._resolve_structure(parent_model, field_name, option, field_info, path, container_chain + ("union",))
            return

        if not issubclass(t, BaseModel):
            return

        self._register_model_node(t, field_name, structure, field_info, path, container_chain)

    def _register_model_node(self, child_model, field_name, structure, field_info, path, container_chain):
        """
        Record a ``BaseModel`` node in ``field_info`` and recurse into its children.

        Args:
            child_model: The child Pydantic model class to register.
            field_name (str): The attribute name of the field on the parent model.
            structure (dict): The dependency structure node for this model.
            field_info (dict): Mutable mapping of class names to :class:`NodeInfo` objects being accumulated.
            path (tuple): Path segments accumulated from the root to the current node.
            container_chain (tuple): Container kinds wrapping the current field.
        """
        if child_model is self.root:
            # A field pointing back to the root type is always the terminus of a cycle:
            # self.tree is built with the root kept in `visited` for the whole walk (see
            # _extract_model_dependencies), so any nested occurrence of the root class already
            # comes wrapped in a "__cycle__" marker. Registering it here would corrupt the
            # root's own NodeInfo (registered with an empty flync_paths in _field_info) with a
            # bogus non-empty path, breaking identity lookups for the root elsewhere.
            return

        new_path = path + ((child_model, field_name, container_chain),)
        model_key = child_model.__name__

        if model_key not in field_info:
            discriminators = {fname for fname, finfo in child_model.model_fields.items() if get_origin(finfo.annotation) is Literal}
            field_info[model_key] = NodeInfo(child_model.__name__, child_model, discriminator_fields=discriminators)

        field_info[model_key].flync_paths.append(ModelDependencyGraph.complex_path_to_string_path(new_path))

        if "children" in structure:
            self._walk_fields(child_model, structure["children"], field_info, new_path)

    def parent_from_child(self, field_type: type[BaseModel], parent_attribute_name: str):
        """
        Find the parent model class that owns a given child field.

        When several parents share the field name, parents whose field is
        externally serialized win: this lookup feeds
        :meth:`rebuild_type_from_parent`, which reconstructs the on-disk
        wrapper for external fields, and a same-named inline field on another
        parent must not decide that shape (``reverse_tree`` values are sets,
        so plain iteration order would vary with class allocation order).

        Args:
            field_type (type[BaseModel]): The child model class.
            parent_attribute_name (str): The field name on the parent that holds the child.

        Returns:
            type[BaseModel] | None: The parent model class, or ``None`` if not found.
        """

        candidates = [parent for parent in self.reverse_tree[field_type] if parent.model_fields.get(parent_attribute_name, None)]
        if not candidates:
            return None
        candidates.sort(
            key=lambda parent: (
                get_metadata(parent.model_fields[parent_attribute_name].metadata, External) is None,
                parent.__name__,
            )
        )
        return candidates[0]

    def field_info_from_child(self, field_type: type[BaseModel], parent_attribute_name: str):
        """
        Return the field info object for a child field on its parent model.

        Args:
            field_type (type[BaseModel]): The child model class.
            parent_attribute_name (str): The attribute name on the parent.

        Returns:
            FieldInfo | type[BaseModel]: The Pydantic ``FieldInfo`` for the attribute on the parent, or ``field_type`` itself when no parent
            is found.
        """

        parent = self.parent_from_child(field_type, parent_attribute_name)
        if not parent:
            return field_type
        return parent.model_fields[parent_attribute_name]

    def rebuild_type_from_parent(self, field_type: type[BaseModel], parent_attribute_name: str):
        """
        Compute the effective validation type for a child field.

        Accounts for ``SINGLE_FILE`` and ``OMMIT_ROOT`` output strategies that change how the YAML is structured on disk.

        Args:
            field_type (type[BaseModel]): The child model class.
            parent_attribute_name (str): The attribute name on the parent.

        Returns:
            type: The adjusted type to use for validation.
        """

        real_type = field_type
        attribute = self.field_info_from_child(field_type, parent_attribute_name)
        # in case of omit root, we need to include a dictionary
        external = get_metadata(attribute.metadata, External)
        if external is not None and OutputStrategy.SINGLE_FILE in external.output_structure:
            annotation = attribute.annotation
            # Strip Optional/Union[..., None] so Optional[List[T]] still
            # resolves to the underlying List[T] container.
            if get_origin(annotation) in (Union, types.UnionType):
                non_none_args = tuple(a for a in get_args(annotation) if a is not NoneType)
                if len(non_none_args) == 1:
                    annotation = non_none_args[0]
            real_type = annotation
            # Re-attach any pydantic validators (BeforeValidator, etc.)
            # that live in the field metadata so they run on the raw file
            # data during validate_with_policy. The External annotation
            # itself is not a pydantic validator and must be excluded.
            validator_metadata = [m for m in attribute.metadata if not isinstance(m, External)]
            if validator_metadata:
                real_type = Annotated[real_type, *validator_metadata]  # type: ignore[assignment]
            if OutputStrategy.OMMIT_ROOT not in external.output_structure:
                real_type = dict[str, real_type]  # type: ignore[valid-type, assignment]
        return real_type

    def normalize_child_to_parent(
        self,
        field_type: type[BaseModel],
        parent_attribute_name: str,
        model_data,
    ):
        """
        Strip the wrapper dict added by ``rebuild_type_from_parent`` when needed.

        When a field was loaded as ``dict[str, T]`` due to ``SINGLE_FILE`` without ``OMMIT_ROOT``, this method extracts the inner value so the
        parent receives the correctly typed object.

        Args:
            field_type (type[BaseModel]): The original child model class.
            parent_attribute_name (str): The attribute name on the parent.
            model_data: The raw validated data (may be a wrapping dict).

        Returns:
            The unwrapped value, or ``None`` if ``model_data`` is falsy.
        """

        if not model_data:
            return None
        attribute = self.field_info_from_child(field_type, parent_attribute_name)
        # in case of omit root, we need to include a dictionary
        external = get_metadata(attribute.metadata, External)
        if (
            external is not None
            and OutputStrategy.SINGLE_FILE in external.output_structure
            and OutputStrategy.OMMIT_ROOT not in external.output_structure
        ):
            return model_data[parent_attribute_name]
        return model_data

    def path_from_object(self, model_object: BaseModel, parent_name: str) -> str:
        """
        Return the dot-separated FLYNC path to an object given its parent field name.

        Args:
            model_object (BaseModel): The model instance to locate.
            parent_name (str): The field name on the parent that holds this object.

        Returns:
            str: The dot-separated path string.

        Raises:
            ValueError: If no matching path can be inferred.
        """

        object_info = self.fields_info[str(type(model_object))]
        for portential_path in object_info.flync_paths:
            if parent_name in portential_path:
                return portential_path
        raise ValueError("Could not infer path from object")

    @staticmethod
    def complex_path_to_string_path(complex_path: list[tuple]) -> str:
        """
        Convert an internal complex path representation to a dot-separated string.

        Each element of ``complex_path`` is a tuple of ``(model_class, field_name, container_chain)``. Container chains of
        ``"dict"`` produce ``{}`` segments and ``"list"`` produce ``[]`` segments in the output.

        Args:
            complex_path (list[tuple]): The internal path representation.

        Returns:
            str: A dot-separated path string such as ``"items.[].name"``.
        """

        parts = []
        for parent in complex_path:
            parts.append(parent[1])
            for kind in parent[2]:
                if kind == "dict":
                    parts.append("{}")
                elif kind == "list":
                    parts.append("[]")
        return ".".join(parts)


_cache_cleaned = False
_cache_name = ""
# In-process cache of built dependency graphs, keyed by ``str(root)``. The shelve
# below persists graphs across process runs; this avoids re-acquiring the file
# lock and re-unpickling the shelve on every call within a single process (the
# graph is deterministic per root type, and this factory is called many times
# per dump/load).
_graph_cache: dict[str, "ModelDependencyGraph"] = {}

# Dynamically-generated roots (type-replacement engine output) have a per-build
# object identity even when two builds are structurally identical. Their graphs
# hold references to those specific synthetic class objects (edges, reverse_tree
# keys), so a structurally-keyed cache would hand back a graph whose nodes belong
# to a *previous* build. Cache such graphs by root identity instead; a weak key
# lets the graph be collected once its root is no longer referenced.
_dynamic_graph_cache: "WeakKeyDictionary[type, ModelDependencyGraph]" = WeakKeyDictionary()


def hash_directory_fast(directory: str, ext=".py") -> str:
    """
    Calculates a md5 hash from a directory.

    Args:
        directory (str): The location of the cache files.
        ext (str): which files to include (default python).
    """

    # only used for file name selection
    h = hashlib.md5()  # NOSONAR python:S4790
    for root, _, files in walk(directory):
        for fname in sorted(files):
            if fname.endswith(ext):
                fpath = join(root, fname)
                file_stat = stat(fpath)
                # Hash metadata only, not file contents
                h.update(f"{fpath}{file_stat.st_mtime}{file_stat.st_size}".encode())
    return h.hexdigest()


def get_package_root(package_name: str | None = None) -> str:
    """
    Gets the location of a package, will default to current package.

    Args:
        package_name (str): The location of the cache files.
            None or default means FLYNC package.
    """

    if not package_name:
        package_name = __package__.split(".")[0]
    package = importlib.import_module(package_name)
    package_path = package.__file__
    if not package_path or not isinstance(package_path, str):
        raise ValueError("Unable to figure out the package location %s", package_name)
    package_path = abspath(package_path)
    return dirname(package_path)


def delete_unwanted_cache_files(cache_location: str, cache_file_name: str, force: bool = False):
    """
    deletes the cache files of the library when different.

    Args:
        cache_location (str): The location of the cache files.
        cache_file_name (str): The name of the cache file to keep.
        force (bool): force clean all cache files.
    """

    for f in listdir(cache_location):
        if force or cache_file_name not in f:
            remove(join(cache_location, f))


def cleanup_old_caches(force: bool = False):
    """Resets the cache of the library if the current version is different or if forced."""
    global _cache_cleaned, _cache_name
    shelv_location = platformdirs.user_cache_dir("FLYNC")
    if not _cache_name:
        # only keep official version cache
        makedirs(shelv_location, exist_ok=True)
        shelv_file_name = "dependency_graph_cache"
        shelv_file_name += "_" + hash_directory_fast(get_package_root())
        if not _cache_cleaned:
            delete_unwanted_cache_files(shelv_location, shelv_file_name, force=force)
            # Clean up stale lock files (older than 1 hour)
            lock_path = join(shelv_location, shelv_file_name + ".lock")
            try:
                if abspath(lock_path) and stat(lock_path).st_mtime < (time.time() - 3600):
                    remove(lock_path)
            except Exception:
                pass
            _cache_cleaned = True
            _cache_name = shelv_file_name
    return shelv_location, _cache_name


def _hash_model_structure(model: type[BaseModel]) -> str:
    """
    Hash a model's structure to detect changes in field definitions.

    Includes field names, types, and metadata so changes to the model
    (e.g., adding/removing fields, changing types) invalidate the cache.

    Args:
        model (type[BaseModel]): The model to hash.

    Returns:
        str: Hex digest of the model structure.
    """
    structure = {}
    for name, field in model.model_fields.items():
        structure[name] = {
            "annotation": str(field.annotation),
            "metadata": str(field.metadata),
        }

    h = hashlib.sha256()
    h.update(json.dumps(structure, sort_keys=True, default=str).encode())
    return h.hexdigest()


def _is_pickleable_class(cls: type) -> bool:
    """
    Return True if ``cls`` can be pickled by reference.

    Pickle serializes classes by ``module.qualname`` lookup, so a class is only
    picklable if importing its ``__module__`` and resolving its ``__qualname__``
    yields the same object. Dynamically generated classes (e.g. the synthetic
    ``FLYNCModel_replaced_..._with_...`` types produced by the type-replacement
    engine) live on no importable module (``abc``) and fail this check.
    """
    module_name = getattr(cls, "__module__", None)
    qualname = getattr(cls, "__qualname__", None)
    if not module_name or not qualname:
        return False
    try:
        module = importlib.import_module(module_name)
    except (ImportError, TypeError, ValueError):
        # ImportError: no module of that name, so pickle could not resolve the class either.
        # TypeError/ValueError: a synthetic ``__module__`` that is not a valid module name
        # (relative or malformed) is rejected by import_module before any import happens.
        return False
    obj = module
    for part in qualname.split("."):
        obj = getattr(obj, part, None)  # type: ignore[assignment]
    return obj is cls


def get_model_dependency_graph(root: type[BaseModel]) -> ModelDependencyGraph:
    """
    Return a cached :class:`ModelDependencyGraph` for the given root model.

    Building a graph is expensive, so instances are cached by root model class. The
    cache key includes a hash of the root's field definitions, so two distinct roots
    with the same qualified name (e.g. a node type rebuilt after a model change) do not
    share a graph.

    Always prefer this factory over instantiating :class:`ModelDependencyGraph` directly.

    Args:
        root (type[BaseModel]): The root Pydantic model class.

    Returns:
        ModelDependencyGraph: The (possibly cached) dependency graph.
    """

    # Dynamically generated models (e.g. type-replacement engine output) are not
    # importable by qualname, so the shelve/pickle disk cache cannot persist their
    # dependency graph (pickling raises PicklingError). More importantly, two such
    # models can be structurally identical (same name and hash) yet be distinct
    # class objects: the structural key below would then return a graph built from
    # a *different* object's synthetic subclasses, and lookups of this object's
    # subclasses in reverse_tree would KeyError. Cache these by object identity.
    if not _is_pickleable_class(root):
        cached = _dynamic_graph_cache.get(root)
        if cached is None:
            cached = ModelDependencyGraph(root)
            _dynamic_graph_cache[root] = cached
        return cached

    key = f"{str(root)}_{_hash_model_structure(root)}"
    cached = _graph_cache.get(key)
    if cached is not None:
        return cached

    shelv_location, shelv_file_name = cleanup_old_caches()
    lock_path = join(shelv_location, shelv_file_name + ".lock")
    with FileLock(lock_path):
        with shelve.open(join(shelv_location, shelv_file_name)) as cache:
            if key not in cache:
                cache[key] = ModelDependencyGraph(root)
            graph = cache[key]
    _graph_cache[key] = graph
    return graph
