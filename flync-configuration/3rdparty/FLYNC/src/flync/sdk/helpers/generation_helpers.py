"""
Generates FLYNC model skeletons for the SDK.

Builds example objects via polyfactory based factories - :class:`FLYNCFactory` for FLYNC model nodes,
:class:`ExternalConnectionFactory` for external connections and :class:`BASET1Factory` for BASE-T1 PHYs.
:func:`generate_node` and :func:`generate_external_node` create new objects inside a workspace and
:func:`dump_flync_workspace` writes a workspace back out.
"""

import pathlib
import random
from ipaddress import IPv4Address, IPv6Address
from os import curdir, sep
from pathlib import Path
from types import UnionType
from typing import Annotated, Any, Literal, Optional, TypeVar, Union, cast, get_args, get_origin

from polyfactory.factories.pydantic_factory import ModelFactory
from pydantic import BaseModel, IPvAnyAddress, RootModel
from pydantic.fields import FieldInfo
from pydantic_core import PydanticUndefined
from pydantic_extra_types.mac_address import MacAddress

from flync.core.datatypes.ipaddress import IPv4AddressEntry
from flync.model.flync_4_ecu.phy import BASET1
from flync.model.flync_4_ecu.sockets import IPv4AddressEndpoint
from flync.model.flync_4_topology.ethernet_topology import EthernetPointToPointConnection
from flync.model.flync_model import FLYNCBaseModel, FLYNCModel
from flync.sdk.context.workspace_config import WorkspaceConfiguration
from flync.sdk.utils.field_utils import get_field_name_from_alias
from flync.sdk.workspace.flync_workspace import FLYNCWorkspace
from flync.sdk.workspace.ids import ObjectId
from flync.sdk.workspace.objects import SemanticObject

from .nodes_helpers import available_flync_nodes, type_from_input

TModel = TypeVar("TModel", bound=BaseModel)


def safe_issubclass(candidate: object, base) -> bool:
    """
    Safely determine whether a given object is a subclass of a specified base class.
    """
    return isinstance(candidate, type) and issubclass(candidate, base)


def __get_valid_path(paths: list[str]) -> list[str]:
    """
    Validate and extract a meaningful path from a list of string segments.

    Args:
        paths (list[str]): A list of path-like strings.

    Returns:
        list[str]: A list of non-numeric chunks representing a valid path, or an empty list if no valid path is found.
    """
    if len(paths) == 1 and paths[0] in ["", "."] or not paths:
        return []
    chunks_list = [p.split(".") for p in paths if p]
    for chunks in chunks_list:
        if all(not chunk.isdigit() for chunk in chunks):
            return chunks
    return chunks_list[0]


def is_union(tp) -> bool:
    """
    Determine whether a given type annotation represents a Union type.
    """

    return tp is Union or tp is UnionType or get_origin(tp) is Union or isinstance(tp, UnionType)


class Factory(object):
    """
    Factory class for managing and generating model-specific factory instances.
    """

    __MODEL_FACTORY_REGISTRY: Optional[dict[type, type[ModelFactory]]] = None
    __FACTORY_MODELS: Optional[list[type[FLYNCBaseModel]]] = None

    @staticmethod
    def build_name(model: type[BaseModel], idx: int = 1):
        if "name" not in model.model_fields:
            return model.__name__.lower()
        return f"{model.__name__.lower()}_{idx}"

    @classmethod
    def factory_model_defined(cls, model: type[FLYNCBaseModel]) -> bool:
        return cls.__FACTORY_MODELS is not None and model in cls.__FACTORY_MODELS

    @classmethod
    def get_factory(cls, model: type) -> type[ModelFactory]:
        if cls.__MODEL_FACTORY_REGISTRY is None:
            cls.__MODEL_FACTORY_REGISTRY = cls.__build_factory_registry()
            cls.__FACTORY_MODELS = [r.__model__ for r in cls.__MODEL_FACTORY_REGISTRY.values()]
        if model not in cls.__MODEL_FACTORY_REGISTRY:
            cls.__MODEL_FACTORY_REGISTRY.update(cls.__build_factory_registry())

        if model not in cls.__MODEL_FACTORY_REGISTRY:
            cls.__MODEL_FACTORY_REGISTRY[model] = cast(
                type[ModelFactory[Any]],
                ModelFactory.create_factory(
                    model=model,
                    bases=(FLYNCFactory,),
                ),
            )

        return cls.__MODEL_FACTORY_REGISTRY[model]

    @classmethod
    def __build_factory_registry(cls) -> dict[type, type[ModelFactory]]:
        registry = {}

        def __get_all_subclasses(cls):
            """
            Recursively yield all subclasses of a given class.
            """

            for subclass in cls.__subclasses__():
                yield subclass
                yield from __get_all_subclasses(subclass)

        for factory_cls in __get_all_subclasses(ModelFactory):
            model = getattr(factory_cls, "__model__", None)
            if model and safe_issubclass(model, FLYNCBaseModel):
                registry[model] = factory_cls

        return registry


class FLYNCFactory(ModelFactory[FLYNCBaseModel]):
    """
    Specialized factory class for creating instances
    of FLYNCBaseModel subclasses.
    """

    __use_defaults__ = True
    __use_examples__ = True

    @staticmethod
    def random_multicast_ipv4():
        return IPv4Address(
            random.randint(
                int(IPv4Address("224.0.0.0")),
                int(IPv4Address("239.255.255.255")),
            )
        )

    @classmethod
    def get_provider_map(cls):
        original_providers = super().get_provider_map()
        original_providers[MacAddress] = lambda: "11:22:33:44:55:66"
        original_providers[IPv6Address] = lambda: IPv6Address("fe80::1")
        original_providers[IPv4Address] = lambda: FLYNCFactory.random_multicast_ipv4()
        original_providers[IPvAnyAddress] = lambda: FLYNCFactory.random_multicast_ipv4()
        return original_providers

    @staticmethod
    def _get_arg_type(field_info: FieldInfo, kwargs: dict):
        discriminator = field_info.discriminator
        args = get_args(field_info.annotation)
        if discriminator is not None and discriminator in kwargs and args:
            for arg in args:
                if safe_issubclass(arg, BaseModel) and discriminator in arg.model_fields:
                    valid, res = FLYNCFactory.__get_field_default_value(arg.model_fields[discriminator])
                    if valid and res == kwargs[discriminator]:
                        return arg

        return FLYNCFactory._default_arg_type(field_info.annotation)

    @staticmethod
    def _default_arg_type(arg):
        """
        Resolve the most appropriate argument type from a typing annotation.

        Args:
            arg (Any): A type annotation, possibly a Union or generic type.

        Returns:
            Any: The resolved type, either a factory-defined model,
        """

        if args := get_args(arg):
            for a in args:
                if Factory.factory_model_defined(a):
                    return a
            if arg1 := [t for t in [IPv4AddressEndpoint, IPv4AddressEntry, IPv4Address] if t in args]:
                arg = arg1[0]
            else:
                arg = FLYNCFactory._default_arg_type(next(a for a in args if a is not type(None)))
        return arg

    @staticmethod
    def _list_element_flync_type(annotation, cls: type[TModel]) -> type[TModel] | None:
        """
        Return the element type when ``annotation`` is a list of FLYNCBaseModel subclasses.

        Handles both bare ``List[X]`` and ``Optional[List[X]]`` (whose origin is a Union), returning ``X`` when it is a FLYNCBaseModel
        subclass and ``None`` otherwise.

        Args:
            annotation: The field type annotation.

        Returns:
            type | None: The list element type, or ``None`` if the annotation is not a list of FLYNC models.
        """

        candidates = [a for a in get_args(annotation) if a is not type(None)] if is_union(annotation) else [annotation]
        for candidate in candidates:
            if (get_origin(candidate) or candidate) is list:
                arg_type = FLYNCFactory._default_arg_type(candidate)
                if safe_issubclass(arg_type, cls):
                    return arg_type
        return None

    @staticmethod
    def __as_plain_and_meta(arg):
        """
        Unwrap a single ``Annotated`` layer, returning the base annotation and any ``FieldInfo`` metadata.

        Args:
            arg: A type annotation, possibly ``Annotated[T, ...]``.

        Returns:
            tuple[Any, FieldInfo | None]: The base annotation and, when present, the ``FieldInfo``
            metadata (e.g. carrying a discriminator), else ``None``.
        """

        if get_origin(arg) is Annotated:
            args = get_args(arg)
            return args[0], args[1] if len(args) > 1 and isinstance(args[1], FieldInfo) else None
        return arg, None

    @staticmethod
    def _strip_annotated(annotation):
        """
        Unwrap nested ``Annotated`` layers, returning the underlying annotation.

        ``Annotated[T, ...]`` may wrap an ``Optional[List[...]]`` at the field level (e.g.
        ``Annotated[Optional[List[X]], External(...)]``). Strip all ``Annotated`` metadata so the
        wrapped list/union/type can be inspected by the list-element resolvers.

        Args:
            annotation: The field type annotation.

        Returns:
            Any: The annotation with all ``Annotated`` metadata removed.
        """

        while get_origin(annotation) is Annotated:
            annotation = get_args(annotation)[0]
        return annotation

    @staticmethod
    def __contains_none_type(annotation) -> bool:
        """Return ``True`` when ``annotation`` (after stripping ``Annotated``) is an optional type."""
        return type(None) in get_args(FLYNCFactory._strip_annotated(annotation))

    @staticmethod
    def _list_element_annotation(annotation):
        """
        Return the element annotation of a (possibly optional) list annotation, else ``None``.

        Handles bare ``List[X]``, ``Optional[List[X]]`` and fields whose list is wrapped in
        ``Annotated``/``External`` metadata (e.g. ``Annotated[Optional[List[X]], External(...)]``).

        Args:
            annotation: The field type annotation.

        Returns:
            type | None: The list element type, or ``None`` if the annotation is not a list.
        """

        annotation = FLYNCFactory._strip_annotated(annotation)
        candidates = [a for a in get_args(annotation) if a is not type(None)] if is_union(annotation) else [annotation]
        for candidate in candidates:
            if (get_origin(candidate) or candidate) is list:
                return get_args(candidate)[0]
        return None

    @staticmethod
    def __is_union_list_element(elem_annotation) -> bool:
        """
        Determine whether a list element is a union of FLYNC models or a RootModel wrapping one.

        Args:
            elem_annotation: The list element type annotation.

        Returns:
            bool: ``True`` when the element is a union of FLYNC models or a RootModel whose ``root`` is such a union.
        """

        base, _ = FLYNCFactory.__as_plain_and_meta(elem_annotation)
        if safe_issubclass(base, RootModel):
            root_field = base.model_fields.get("root")
            return (
                root_field is not None
                and is_union(root_field.annotation)
                and any(safe_issubclass(a, FLYNCBaseModel) for a in get_args(root_field.annotation))
            )
        if is_union(base):
            return any(safe_issubclass(a, FLYNCBaseModel) for a in get_args(base))
        return False

    @staticmethod
    def __resolve_list_element(elem_annotation, kw: dict):
        """
        Resolve the concrete model type (and optional RootModel wrapper) for a single list item.

        For a RootModel wrapping a union, the concrete member is selected with :meth:`_get_arg_type`
        using the union's discriminator value found in ``kw``; the RootModel class is returned as the
        wrapper to re-wrap the built member.

        Args:
            elem_annotation: The list element type annotation.
            kw (dict): Per-item override values.

        Returns:
            tuple[type, type | None]: A ``(concrete_type, wrapper_type)`` pair where ``wrapper_type`` is
            the RootModel class to wrap the built member in, or ``None`` when no wrapping is needed.
        """

        base, meta = FLYNCFactory.__as_plain_and_meta(elem_annotation)
        if safe_issubclass(base, RootModel):
            root_field = base.model_fields["root"]
            concrete = FLYNCFactory._get_arg_type(root_field, kw)
            return concrete, base
        if is_union(base):
            if isinstance(meta, FieldInfo) and meta.discriminator:
                field_info = FieldInfo(annotation=base, discriminator=meta.discriminator)
                base = FLYNCFactory._get_arg_type(field_info, kw)
            else:
                base = FLYNCFactory._default_arg_type(base)
        return base, None

    @staticmethod
    def __get_field_default_value(field_info: FieldInfo) -> tuple[bool, Any]:
        """
        Determine the default value for a Pydantic field.

        Args:
            field_info (FieldInfo): The Pydantic field metadata object.

        Returns:
            tuple[bool, Any]:
                - `bool`: Whether a valid default value was found.
                - `Any`: The resolved default value, or `None` if unavailable.
        """

        args = get_args(field_info.annotation)
        is_optional = type(None) in args
        # Lists of FLYNC models are scaffolded with generated elements rather than left at their (empty) default, so nested structures such as
        # a controller's interfaces appear in the template and cross-field validators (e.g. "at least one interface") are satisfied.
        if FLYNCFactory._list_element_flync_type(field_info.annotation, FLYNCBaseModel) is not None and not is_optional:
            return (False, None)
        valid, result = (False, None)
        if field_info.default is not PydanticUndefined:
            valid, result = (True, field_info.default)
        elif field_info.default_factory is not None and is_optional:
            valid, result = (True, field_info.default_factory())  # type: ignore[call-arg]
        elif field_info.examples:
            valid, result = (True, field_info.examples[0])
        elif is_optional:
            valid, result = (True, None)

        return (valid, result)

    @staticmethod
    def _get_field_value_list(field_info, kw_list: list) -> tuple[bool, Any]:
        """
        Generate a default list of values for a Pydantic field when the type annotation indicates a list of FLYNCBaseModel subclasses.

        Each item is built from its own override dict, allowing a list whose element type is a
        union (or a RootModel wrapping a union) to produce items of the matching concrete member.

        Args:
            field_info (FieldInfo): Metadata about the Pydantic field.
            kw_list (list): list of override values.

        Returns:
            tuple[bool, Any]:
                - `bool`: Whether a valid list of values was generated.
                - `Any`: The generated list of model instances.
        """

        elem_annotation = FLYNCFactory._list_element_annotation(field_info.annotation)
        base_elem, _ = FLYNCFactory.__as_plain_and_meta(elem_annotation) if elem_annotation else (None, None)

        # Validation checks
        is_single = safe_issubclass(base_elem, FLYNCBaseModel) if base_elem else False
        is_union_elem = FLYNCFactory.__is_union_list_element(elem_annotation) if elem_annotation else False

        valid_annotation = elem_annotation and (is_single or is_union_elem)
        if not valid_annotation:
            return False, None

        # Optional lists: skip scaffolding if no overrides
        if not kw_list and (not is_single or FLYNCFactory.__contains_none_type(field_info.annotation)):
            return False, None

        length = len(kw_list)
        min_length = length or FLYNCFactory.__min_length_list(field_info)
        items = []
        try:
            for idx in range(1, min_length + 1):
                kw = kw_list[idx - 1] if kw_list else {}
                items.append(FLYNCFactory._get_field_value_list_item(idx, elem_annotation, kw))
            result = items
        except Exception:
            result = FLYNCFactory.__fallback_list_value(field_info)

        return True, result

    @staticmethod
    def _get_field_value_list_item(idx: int, elem_annotation, kw: dict) -> Any:
        """
        Generate a item list from its own override dict.

        Args:
            idx (int): item index.
            field_info (FieldInfo): Metadata about the Pydantic field.
            kw (list): the override values.

        Returns:
            - `Any`: The generated item list of model instance.
        """
        concrete, wrapper = FLYNCFactory.__resolve_list_element(elem_annotation, kw)
        if "name" in concrete.model_fields and "name" not in kw:
            kw["name"] = Factory.build_name(concrete, idx=idx)
        built = Factory.get_factory(concrete).build(**kw)
        if wrapper is not None:
            built = Factory.get_factory(wrapper).build(root=built)
        return built

    @staticmethod
    def __fallback_list_value(field_info):
        """
        Return the declared default for a list field that could not be scaffolded.

        Raises the original exception when no default is available.
        """
        if field_info.default is not PydanticUndefined:
            return field_info.default
        if field_info.default_factory is not None:
            return field_info.default_factory()  # type: ignore[call-arg]
        raise

    @staticmethod
    def __min_length_list(field_info: FieldInfo) -> int:
        """
        Determine the minimum list length for a Pydantic field.

        Args:
            field_info (FieldInfo): The Pydantic field metadata object.

        Returns:
            int: The minimum list length, either from metadata or the default of `1`.
        """

        min_length = 1
        if field_info.metadata is None:
            return min_length
        for m in field_info.metadata:
            if length := getattr(m, "min_length", None):
                return length
        return min_length

    @staticmethod
    def __get_field_value(
        model: type[BaseModel],
        field_name: str,
        field_info: FieldInfo,
    ) -> tuple[bool, Any]:
        field_name = field_info.alias or field_name
        valid, value = FLYNCFactory.__get_field_default_value(field_info)
        if valid:
            return valid, value

        origin_type = get_origin(field_info.annotation) or field_info.annotation
        if origin_type is list or is_union(origin_type) or origin_type is Literal:
            arg_type = FLYNCFactory._get_arg_type(field_info, {})
            valid, value = FLYNCFactory._get_field_value_list(field_info, [])
            if valid:
                return valid, value
            elif origin_type is Literal:
                valid, value = True, arg_type
            elif safe_issubclass(arg_type, FLYNCBaseModel) and is_union(origin_type):
                valid, value = True, Factory.get_factory(arg_type).build()
            else:
                valid, value = True, []
        elif origin_type and safe_issubclass(origin_type, FLYNCBaseModel):
            valid, value = True, Factory.get_factory(origin_type).build()
        elif field_name == "name":
            name = Factory.build_name(model=model)
            valid, value = True, name

        return valid, value

    @staticmethod
    def __resolve_key(field_name: str, field_info: Any, kwargs: dict) -> str | None:
        """Resolve the key to look up in kwargs, checking alias, discriminator, then field name."""
        alias = field_info.alias
        if alias and alias in kwargs:
            return alias
        if isinstance(field_info.discriminator, str) and field_info.discriminator in kwargs:
            return field_info.discriminator
        return field_name if field_name in kwargs else None

    @staticmethod
    def __build_nested_model(field_info, value: dict):
        """Build a nested FLYNCBaseModel from a dict override value."""
        field_type = FLYNCFactory._get_arg_type(field_info, value)
        if not safe_issubclass(field_type, FLYNCBaseModel):
            return value
        return Factory.get_factory(field_type).build(**value)

    @classmethod
    def __build_nested_list(cls, field_info, kw_list: list):
        """Build a list of FLYNCBaseModel items from a list override value."""

        built = [kw for kw in kw_list if not isinstance(kw, dict)]
        new_kw_list = [kw for kw in kw_list if isinstance(kw, dict)]
        if new_kw_list and not built:
            valid, result = cls._get_field_value_list(field_info, new_kw_list)
            if valid:
                built.extend(result)
        return built

    @classmethod
    def __consume_provided_fields(cls, kwargs: dict):
        """Consume kwargs, recursively building nested dict/list overrides."""
        new_kwargs = {}
        pending_fields = {}

        for field_name, field_info in cls.__model__.model_fields.items():
            if field_info.exclude:
                continue

            key = cls.__resolve_key(field_name, field_info, kwargs)
            if key is None:
                pending_fields[field_name] = field_info
                continue

            value = kwargs.pop(key)
            if isinstance(value, dict):
                value = cls.__build_nested_model(field_info, value)
            elif isinstance(value, list):
                value = cls.__build_nested_list(field_info, value)

            new_kwargs[field_name] = value

        return new_kwargs, pending_fields

    @classmethod
    def build(cls, **kwargs):
        # Handle provided values first (including nested dict/list overrides)
        new_kwargs, pending_fields = cls.__consume_provided_fields(kwargs)

        for fname, finfo in pending_fields.items():
            has_value, value = FLYNCFactory.__get_field_value(cls.__model__, fname, finfo)
            if has_value:
                new_kwargs[fname] = value

        obj = super().build(**new_kwargs)
        return obj


class ExternalConnectionFactory(FLYNCFactory):
    """
    Factory for ExternalConnection model.
    """

    __model__ = EthernetPointToPointConnection

    @classmethod
    def build(cls, **kwargs):
        kwargs.setdefault("ecu1_port", "port1")
        kwargs.setdefault("ecu2_port", "port2")
        return super().build(**kwargs)


class BASET1Factory(FLYNCFactory):
    """
    Factory for BASET1 model.
    """

    __model__ = BASET1

    @classmethod
    def build(cls, **kwargs):
        return super().build(**kwargs)


def dump_flync_workspace(
    flync_model: FLYNCModel,
    output_path: str | pathlib.Path,
    workspace_name: str | None,
    workspace_config: WorkspaceConfiguration | None = None,
) -> None:
    """
    Generate a FLYNC workspace from a FLYNCModel object.

    Args:
        flync_model (:class:`~flync.model.flync_model.FLYNCModel`): The FLYNC model to generate the workspace from.
        output_path (str | pathlib.Path): The path where the workspace will be created.
        workspace_name (str | None): Optional name for the workspace.
        workspace_config (WorkspaceConfiguration | None): Optional workspace configuration. Uses defaults if ``None``.

    Returns:
        None
    """

    ws = FLYNCWorkspace.load_model(
        flync_model,
        workspace_name,
        output_path,
        workspace_config=workspace_config,
    )
    ws.generate_configs()


def generate_external_node(
    node: str | type[FLYNCBaseModel],
    node_path: Path | str,
    workspace_config: WorkspaceConfiguration | None = None,
    **override_values,
):
    """
    Generate external node.
    """

    node = type_from_input(node)
    # generate object from type
    model_factory = Factory.get_factory(node)
    flync_obj = model_factory.build(
        **override_values,
    )
    # dump to output
    if not isinstance(node_path, Path):
        node_path = Path(node_path)
    FLYNCWorkspace.load_model(flync_obj, file_path=node_path, workspace_config=workspace_config).generate_configs()


def _get_flync_path(model: BaseModel | list | set, field_name: str) -> str:
    """Determine the flync path based on model type."""
    if isinstance(model, (list, set)):
        return f"{field_name}.[]"
    if isinstance(model, FLYNCBaseModel) and field_name in type(model).model_fields:
        return f"{field_name}"
    return ""


def __resolve_semantic_object(
    so: SemanticObject,
    field_name: str,
) -> tuple[str, type[FLYNCBaseModel] | None]:
    """
    Resolve the FLYNC path and root type from a semantic object for a given field.

    Args:
        so (SemanticObject): The semantic object containing the model.
        field_name (str): The name of the field to resolve.

    Returns:
        tuple[str, type[FLYNCBaseModel] | None]:
            - `str`: The resolved FLYNC path for the field (e.g. ``"ports.[]"`` for a list field).
            - `type[FLYNCBaseModel] | None`: The model type if the resolved value is a FLYNC model, ``None`` otherwise.
    """

    model = so.model
    if isinstance(model, dict) and field_name in model:
        model = model[field_name]
    root = type(model) if isinstance(model, FLYNCBaseModel) else None
    flync_path = _get_flync_path(model, field_name)
    return flync_path, root


def __resolve_path(valid_path: list[str], ws: FLYNCWorkspace) -> tuple[type[FLYNCBaseModel], str, str, FLYNCBaseModel | None]:
    """
    Walk ``valid_path`` segments and resolve the root type, the last field name, the FLYNC path, and the owner model.

    Args:
        valid_path (list[str]): A list of path segments (e.g. ``["ecus", "0", "ports", "override_port"]``).
        ws (FLYNCWorkspace): The workspace to resolve against.

    Returns:
        tuple[type[FLYNCBaseModel], str, str, FLYNCBaseModel | None]:
            - ``type[FLYNCBaseModel]``: The resolved root model type (defaults to :class:`FLYNCModel`).
            - ``str``: The last resolved field name (e.g. ``"ports"``).
            - ``str``: The FLYNC path for the field (e.g. ``"ports.[]"``).
            - ``FLYNCBaseModel | None``: The owner model instance that holds the target field, or ``None`` if not found.
    """

    root: type[FLYNCBaseModel] = FLYNCModel
    node_field_name: str = ""
    flync_path = ""
    parent_path: list[str] = []
    owner_model: FLYNCBaseModel | None = None
    for field_name in valid_path:
        parent_path.append(field_name)
        path = ObjectId(curdir.join(parent_path))
        if not ws.has_object(path):
            if owner_model is not None and hasattr(owner_model, field_name):
                root = FLYNCFactory._default_arg_type(type(owner_model).model_fields[field_name].annotation)
                node_field_name = field_name
            continue
        node_field_name = field_name
        so: SemanticObject = ws.get_object(path)
        flync_path, resolved_root = __resolve_semantic_object(so, field_name)
        if resolved_root is not None:
            root = resolved_root
        elif so.model is None and owner_model is not None and hasattr(owner_model, field_name):
            # The field is registered (e.g. an unset Optional External field) but has no value
            # yet, so fall back to the owner's field annotation for the type. Only applies when
            # so.model is genuinely None -- list fields resolve so.model to the (possibly empty)
            # list itself and must keep the owner as root to match its "field.[]" flync path.
            root = FLYNCFactory._default_arg_type(type(owner_model).model_fields[field_name].annotation)
        if isinstance(so.model, FLYNCBaseModel):
            owner_model = so.model

    return root, node_field_name, flync_path, owner_model


def __get_generated_node(
    root_node: Optional[str | type[FLYNCBaseModel]],
    flync_path: str,
    node_name: str,
    **override_values,
) -> FLYNCBaseModel | None:
    """
    Build a FLYNC node instance matching the given path.

    Searches the available node registry for a node type whose FLYNC paths match ``flync_path``,
    then builds a new instance with the provided overrides.

    Args:
        root_node (str | type[FLYNCBaseModel] | None): Optional root node type or name to constrain the search.
        flync_path (str): The FLYNC path to match (e.g. ``"ports.[]"``).
        **override_values: Keyword arguments forwarded to the model factory :meth:`build`.

    Returns:
        FLYNCBaseModel | None: The generated node instance, or ``None`` if no matching node type is found.
    """
    nodes = available_flync_nodes(root_node=root_node)
    for node_info in nodes.values():
        if flync_path in node_info.flync_paths or (not flync_path and not node_info.flync_paths):
            python_type = node_info.python_type
            if safe_issubclass(node_info.python_type, RootModel) and node_name:
                annotation = node_info.python_type.model_fields["root"].annotation
                if node_type := next(
                    (arg for arg in get_args(annotation) if getattr(arg, "__name__", None) == node_name and safe_issubclass(arg, FLYNCBaseModel)),
                    None,
                ):
                    python_type = node_type
            model_factory = Factory.get_factory(python_type)
            return model_factory.build(
                **override_values,
            )
    return None


def __try_append_to(collection, item) -> bool:
    """
    Append or add an item to a list or set collection in-place.

    Args:
        collection: The collection to modify (``list`` or ``set``). ``None`` and other types are silently ignored.
        item: The item to append or add.

    Returns:
        bool: ``True`` if the item is present in the collection after the operation, ``False`` otherwise.
    """
    if isinstance(collection, list):
        collection.append(item)
    elif isinstance(collection, set):
        collection.add(item)
    return item in (collection or [])


def __patch_owner(ws, owner_model, generated_node, override_values, owner_path=""):
    """
    Patch an owner model in place using the override values and the generated node.

    Complex fields that already exist as workspace objects are patched recursively,
    collections are appended to in-place, and everything else is assigned from the
    generated node.

    Args:
        ws (FLYNCWorkspace): The workspace object.
        owner_model: The model to patch.
        generated_node: The generated node providing replacement/derived values.
        override_values (dict): The override values used to build the generated node.
        owner_path (str): The semantic object path of the owner ("" for root es).

    Returns:
        None
    """
    for key, value in override_values.items():
        fname = get_field_name_from_alias(type(owner_model), key)
        if fname not in type(owner_model).model_fields:
            continue
        gen_value = getattr(generated_node, fname, None)
        field_path = f"{owner_path}.{fname}" if owner_path else fname
        if not __patch_existing(ws, owner_model, fname, gen_value, value, field_path):
            setattr(owner_model, fname, gen_value)


def __patch_existing(ws, owner_model, fname, gen_value, value, field_path) -> bool:
    """
    Handle the recursion and in-place collection append cases for :func:`__patch_owner`.

    Args:
        ws (FLYNCWorkspace): The workspace object.
        owner_model: The model to patch.
        fname (str): The resolved field name on the owner.
        gen_value: The generated value for the field.
        value: The raw override value (dict for complex fields, list for collections).
        field_path (str): The fully-qualified path of the field.

    Returns:
        bool: ``True`` if the field was patched here (recurse or append), ``False``
            otherwise so the caller falls back to a plain assignment.
    """
    if isinstance(value, dict) and ws.has_object(ObjectId(field_path)) and ws.get_object(ObjectId(field_path)).model is not None:
        __patch_owner(ws, ws.get_object(ObjectId(field_path)).model, gen_value, value, field_path)
        return True
    if isinstance(value, list) and isinstance(getattr(owner_model, fname, None), (list, set)):
        for item in gen_value or []:
            __try_append_to(getattr(owner_model, fname, None), item)
        return True
    return False


def __attach_to_owner(
    ws: FLYNCWorkspace,
    owner_model,
    node_field_name: str,
    generated_node,
    override_values: dict | None = None,
) -> tuple[bool, Path | None, Any]:
    """
    Attach a generated node to its owner model.

    When the generated node is of the same type as the owner, the owner is patched in-place using
    the override values: collections are appended to (e.g. union-list ``connections``), while empty
    or scalar fields are assigned. Otherwise the generated node is treated as a child and attached to
    the owner's ``node_field_name`` field (existing list/assignment behaviour).

    Args:
        ws (FLYNCWorkspace): The workspace object.
        owner_model: The parent model that owns the field.
        node_field_name (str): The name of the field on the owner model.
        generated_node: The node to attach.
        override_values (dict | None): The override values used to build the generated node.

    Returns:
        tuple[bool, Path | None, Any]:
            - success flag
            - resolved path for serialization
            - model to be reloaded
    """
    override_values = override_values or {}
    if ws.workspace_root is None:
        return False, None, None

    # Case B - the generated node is the same type as the owner: patch the owner in place.
    if type(owner_model) is type(generated_node):
        owner_path = str(objs[0].id) if (objs := ws.get_semantic_objects_from_model(owner_model)) else ""
        __patch_owner(ws, owner_model, generated_node, override_values, owner_path)

    # Case A - attach the generated child node to the owner's field.
    else:
        if not hasattr(owner_model, node_field_name):
            return False, None, None
        # Handle list element type conversion if needed
        if root_model := FLYNCFactory._list_element_flync_type(type(owner_model).model_fields[node_field_name].annotation, RootModel):
            generated_node = __get_generated_node(root_model.__name__, "", "", root=generated_node)

        if not __try_append_to(getattr(owner_model, node_field_name, None), generated_node):
            setattr(owner_model, node_field_name, generated_node)

    # Resolve path for serialization
    path: Path | None = None
    model = owner_model
    if objs := ws.get_semantic_objects_from_model(owner_model):
        uri = ws.get_source(objs[0].id).uri
        path = ws.workspace_root / uri.removesuffix(ws.configuration.flync_file_extension)

    return True, path, model


def generate_node(
    ws: FLYNCWorkspace,
    node_paths: list[str] = [],
    **override_values,
) -> bool:
    """
    Build and attach a FLYNC node in the workspace.

    Resolves the given path, creates the node using its factory with optional overrides,
    and attaches it to the workspace or parent.

    When the node belongs to a list field on an existing owner model, the node is appended
    to the owner's field and the owner is re-serialized.

    When the generated node is the root model, the workspace root model is replaced.

    Args:
        ws (FLYNCWorkspace): The workspace object in which the node will be generated and attached.
        node_paths (list[str]): A list of path components identifying the target node location.
        Each element may be a dot-separated segment (e.g., ``ecus.0``) or a plain segment (e.g., ``ecus``).
        Defaults to an empty list.
        override_values: Arbitrary keyword arguments used to override default values when building the node via its factory.

    Returns:
        bool: ``True`` if the node was successfully generated and attached,
        ``False`` if the path cannot be resolved or workspace root is ``None``.
    """

    if ws.workspace_root is None:
        return False

    valid_path = __get_valid_path(node_paths)
    root, node_field_name, flync_path, owner_model = __resolve_path(valid_path, ws)

    if generated_node := __get_generated_node(
        root_node=root, flync_path=flync_path, node_name=valid_path[-1] if valid_path else "", **override_values
    ):
        success = True
        model = generated_node
        path: Path | None = Path(sep.join(valid_path))

        if isinstance(generated_node, ws.configuration.root_model):
            ws.flync_model = generated_node
            path = ws.workspace_root
        elif owner_model is not None:
            success, path, model = __attach_to_owner(ws, owner_model, node_field_name, generated_node, override_values)

        if success and path:
            ws.load_flync_model(flync_model=model, file_path=path)
            return True

    return False
