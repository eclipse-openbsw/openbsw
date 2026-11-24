"""Utilities for dumping FLYNC models with discriminator fields included."""

from typing import TYPE_CHECKING, Any, Iterator

from pydantic import BaseModel

if TYPE_CHECKING:
    from flync.sdk.utils.model_dependencies import ModelDependencyGraph


def _models_in(value: Any) -> Iterator[BaseModel]:
    """
    Yield every model held directly by a field value.

    Handles the three shapes a FLYNC field can take: a plain model, a list of them, or a dict of
    them. Anything else yields nothing.

    Args:
        value: The field value to inspect.

    Yields:
        BaseModel: Each model found in ``value``.
    """

    if isinstance(value, BaseModel):
        yield value
    elif isinstance(value, list):
        yield from (item for item in value if isinstance(item, BaseModel))
    elif isinstance(value, dict):
        yield from (item for item in value.values() if isinstance(item, BaseModel))


def _nested_models(model: BaseModel) -> Iterator[BaseModel]:
    """
    Yield the models reachable from ``model`` through one field access.

    A field whose value cannot be read (e.g. a property that raises) is skipped rather than
    aborting the traversal.

    Args:
        model (BaseModel): The model whose fields are walked.

    Yields:
        BaseModel: Each directly nested model.
    """

    for field_name in type(model).model_fields:
        try:
            value = getattr(model, field_name, None)
        except Exception:
            continue
        yield from _models_in(value)


def _mark_discriminators(model: BaseModel, graph: "ModelDependencyGraph") -> None:
    """
    Mark the discriminator fields of ``model`` and every nested model as explicitly set.

    Args:
        model (BaseModel): The model to start from.
        graph (ModelDependencyGraph): Pre-built graph providing per-class discriminator info.
    """

    node_info = graph.fields_info.get(type(model).__name__)
    if node_info and node_info.discriminator_fields:
        model.model_fields_set.update(node_info.discriminator_fields)
    for nested in _nested_models(model):
        _mark_discriminators(nested, graph)


def dump_model_with_discriminators(model: BaseModel, **kwargs) -> dict:
    """
    Dump a model to a dictionary, ensuring Literal discriminator fields are included.

    When a model has Literal-typed discriminator fields with default values, Pydantic
    may exclude them from the dump if they weren't explicitly set during model creation.
    This function ensures they are included in the output by looking up discriminator info
    from the ModelDependencyGraph (built once at startup) and marking them as set before
    dumping. Recursively applies this to all nested models.

    Uses the pre-built ModelDependencyGraph for O(1) lookup efficiency. Falls back to no-op
    if the graph is not available (e.g., during early initialization).

    Args:
        model: The Pydantic model instance to dump.
        **kwargs: Additional arguments to pass to model.model_dump().

    Returns:
        dict: The dumped model data with discriminators included.
    """

    try:
        # Imported lazily: the graph is unavailable during early initialization, and the import
        # itself would be circular at module load time.
        from flync.model import FLYNCModel
        from flync.sdk.utils.model_dependencies import get_model_dependency_graph

        _mark_discriminators(model, get_model_dependency_graph(FLYNCModel))
    except Exception:
        # Graph not available or model not in graph, skip marking
        pass
    kwargs.setdefault("mode", "json")
    return model.model_dump(**kwargs)
