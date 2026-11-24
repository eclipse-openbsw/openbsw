"""
Semantic object layer the FLYNC SDK exposes to its clients.

:class:`SemanticObject` wraps a FLYNC model object together with its :class:`ObjectMetadata`, so tooling can
navigate objects by :class:`ObjectId` instead of by Python reference. The :class:`FieldMetadata` hierarchy -
:class:`ScalarFieldMetadata`, :class:`ListFieldMetadata` and :class:`DictFieldMetadata` - describes how a
field references other objects and serializes that information to JSON-safe dictionaries.
"""

from abc import ABC, abstractmethod
from typing import TYPE_CHECKING, Optional

from pydantic import BaseModel

from flync.core.annotations.external import External, OutputStrategy
from flync.core.annotations.reference import Reference
from flync.sdk.utils.field_utils import get_metadata, get_name

from .ids import ObjectId

if TYPE_CHECKING:
    from .flync_workspace import FLYNCWorkspace


class FieldMetadata(ABC):
    """Base class for field reference metadata."""

    @abstractmethod
    def to_dict(self) -> dict:
        """Serialize this field reference to a JSON-safe dictionary."""
        ...


class ScalarFieldMetadata(FieldMetadata):
    """Single object reference."""

    def __init__(self, ids: list[ObjectId]):
        self.ids = ids

    def to_dict(self) -> dict:
        return {"type": "object", "ids": [str(oid) for oid in self.ids]}


class ListFieldMetadata(FieldMetadata):
    """List of field references (items can be scalar, list, or dict)."""

    def __init__(self, items: list["FieldMetadata"]):
        self.items = items

    def to_dict(self) -> dict:
        return {"type": "list", "items": [item.to_dict() for item in self.items]}


class DictFieldMetadata(FieldMetadata):
    """Dict of field references (values can be scalar, list, or dict)."""

    def __init__(self, items: dict[str, "FieldMetadata"]):
        self.items = items

    def to_dict(self) -> dict:
        return {"type": "dict", "items": {key: metadata.to_dict() for key, metadata in self.items.items()}}


class SemanticObject(object):
    """
    Wrapper around a validated semantic model.

    Attributes:
        id (ObjectId): Identifier of the semantic object.
        model (BaseModel): The validated Pydantic model.
    """

    __slots__ = ("id", "model")

    def __init__(self, id: ObjectId, model: BaseModel):
        """
        Initialize a SemanticObject.

        Args:
            id (ObjectId): Identifier of the semantic object.
            model (BaseModel): The validated Pydantic model.
        """

        self.id = id
        self.model = model


class ObjectMetadata(object):
    """
    Metadata about a semantic object for JSON serialization.

    Provides type information, field details, relationships, and source location
    without exposing the full model data (which can be large).

    Attributes:
        id (ObjectId): Identifier of the semantic object.
        type_name (str): Class name of the model.
        source (dict): Source file reference {uri, range}.
    """

    def __init__(self, semantic_obj: SemanticObject, workspace: "FLYNCWorkspace"):
        """
        Initialize ObjectMetadata from a SemanticObject.

        Args:
            semantic_obj (SemanticObject): The semantic object to extract metadata from.
            workspace (FLYNCWorkspace): The workspace for resolving relationships.
        """

        self.id = semantic_obj.id
        self._model = semantic_obj.model
        self._workspace = workspace

    @property
    def type_name(self) -> str:
        """Class name of the model."""
        return type(self._model).__name__

    @property
    def name(self) -> str:
        """Display name: model's 'name' attribute, or last segment of the ObjectId."""
        fallback = str(self.id).split(".")[-1]
        return get_name(self._model, "name", fallback)

    @property
    def parent_id(self) -> Optional[str]:
        """Parent ObjectId (immediate container), or None if root."""
        id_str = str(self.id)
        parts = id_str.rsplit(".", 1)
        if len(parts) > 1 and parts[0]:
            parent_id = parts[0]
            parent_parts = parent_id.rsplit(".", 1)
            if len(parent_parts) > 1:
                grandparent_last = parent_parts[0].rsplit(".", 1)[-1]
                if parent_parts[1] == grandparent_last:
                    return parent_parts[0]
            return parent_id
        return None

    @property
    def child_ids(self) -> list[str]:
        """Direct child ObjectIds, collapsing SINGLE_FILE wrapper levels."""
        raw = self._workspace.get_child_ids(self.id)
        if len(raw) == 1:
            only = raw[0]
            child_last = only.rsplit(".", 1)[-1]
            my_last = str(self.id).rsplit(".", 1)[-1]
            if child_last == my_last and self._parent_field_is_single_file(my_last):
                return self._workspace.get_child_ids(ObjectId(only))
        return raw

    def _parent_field_is_single_file(self, field_name: str) -> bool:
        """Check if *field_name* on the parent model is a SINGLE_FILE (no OMMIT_ROOT) external field."""
        try:
            pid = self.parent_id
            if pid is not None:
                parent_model = self._workspace.get_object(ObjectId(pid)).model
                model_fields = getattr(type(parent_model), "model_fields", {})
                if field_name in model_fields:
                    ext = get_metadata(model_fields[field_name].metadata, External)
                    return (
                        ext is not None
                        and OutputStrategy.SINGLE_FILE in ext.output_structure
                        and OutputStrategy.OMMIT_ROOT not in ext.output_structure
                    )
        except (KeyError, AttributeError):
            return False

        return False

    @property
    def source(self) -> dict:
        """Source file and location {uri, range}."""
        src = self._workspace.get_source(self.id)
        return {
            "uri": src.uri,
            "range": {
                "start": {"line": src.range.start.line, "character": src.range.start.character},
                "end": {"line": src.range.end.line, "character": src.range.end.character},
            },
        }

    @property
    def fields(self) -> dict[str, FieldMetadata]:
        """Maps reference fields to the ObjectId(s) they resolve to.

        Only fields explicitly annotated with :class:`Reference` are treated as
        references, and each target is resolved through the workspace's reference
        machinery (the private ``source`` attribute). A field's target is never
        inferred from Python object identity: interned scalar values (e.g. a
        discriminator ``mode: "base_t1"`` shared across many ports) would otherwise
        collide, making a plain scalar masquerade as a reference to every other
        field that happens to hold the same literal.
        """
        result: dict[str, FieldMetadata] = {}
        model_fields = getattr(type(self._model), "model_fields", {})

        for field_name, field_info in model_fields.items():
            if get_metadata(field_info.metadata, Reference) is None:
                continue
            target = self._workspace.get_definition(self.id, field_name)
            if target is not None:
                result[field_name] = ScalarFieldMetadata([target])

        return result

    def to_dict(self) -> dict:
        """
        Serialize metadata to a dictionary (for REST JSON responses).

        The model is kept private and not included in the output.
        Clients should use a separate /data endpoint if they need the full model.

        Returns:
            dict: Metadata only, without the model.
        """
        fields_dict = {name: metadata.to_dict() for name, metadata in self.fields.items()}
        return {
            "id": str(self.id),
            "name": self.name,
            "type": self.type_name,
            "parent_id": self.parent_id,
            "child_ids": self.child_ids,
            "fields": fields_dict,
            "source": self.source,
        }
