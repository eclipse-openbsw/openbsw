"""
Node metadata types for the FLYNC SDK context.

Provides :class:`NodeInfo`, which captures type and path information for nodes in the FLYNC model dependency graph.
"""

from typing import Type

from pydantic import Field, dataclasses, field_serializer


@dataclasses.dataclass
class NodeInfo:
    """
    Metadata for a node in the FLYNC model dependency graph.

    Attributes:
        name (str): Human-readable name of the node (typically the class name).
        python_type (Type): The Python type (usually a Pydantic model class) that this node represents.
        flync_paths (list[str]): Dot-separated paths from the root model to this node through the dependency graph.
        discriminator_fields (set[str]): Field names that are Literal discriminators for this model.
    """

    name: str
    python_type: Type
    flync_paths: list[str] = Field(default_factory=list)
    discriminator_fields: set[str] = Field(default_factory=set)

    @field_serializer("python_type")
    def serialize_type(self, value: Type) -> str:
        """
        Serialize the Python type to its string representation.

        Args:
            value (Type): The type to serialize.

        Returns:
            str: The string form of the type.
        """

        return str(value)
