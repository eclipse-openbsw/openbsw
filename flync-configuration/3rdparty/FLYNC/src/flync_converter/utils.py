"""Shared utilities for the CLI and TUI."""

from __future__ import annotations

import logging
import typing
from typing import Type, Union, get_type_hints

from flync_converter.base import ConverterConfig
from flync_converter.registry import registry

logger = logging.getLogger(__name__)


def _unwrap_optional(hint):
    """Unwrap Optional[X] / Union[X, None] to X; return hint unchanged."""
    origin = getattr(hint, "__origin__", None)
    if origin is Union:
        args = [a for a in typing.get_args(hint) if a is not type(None)]
        if args:
            return args[0]
    return hint


def _is_concrete_config(hint) -> bool:
    """Return True if hint is a concrete ConverterConfig subclass."""
    return isinstance(hint, type) and issubclass(hint, ConverterConfig) and hint is not ConverterConfig


def get_config_model(converter_type: str) -> Type[ConverterConfig]:  # NOSONAR
    """Return the config model class for a registered converter.

    Resolution order (most-specific first):

    1. Class-level ``config`` annotation in the converter's MRO.
    2. ``config`` parameter annotation on ``__init__``.
    3. ``Config`` or ``config_model`` class attributes.

    Falls back to base :class:`ConverterConfig` when nothing more specific is
    found.

    Args:
        converter_type: Converter key as registered in the registry.

    Returns:
        A ConverterConfig subclass to use for configuration.
    """
    logger.debug("Resolving config model for converter: %s", converter_type)
    try:
        conv = registry[converter_type]
        cls = type(conv)

        # 1. Walk MRO for a class-level 'config' annotation that is more
        #    specific than the base ConverterConfig.
        for klass in cls.__mro__:
            raw = klass.__dict__.get("__annotations__", {}).get("config")
            if raw is None:
                continue
            # Resolve string annotations using the class's module globals.
            try:
                resolved_hints = get_type_hints(klass)
                raw = resolved_hints.get("config", raw)
            except Exception:
                pass
            hint = _unwrap_optional(raw)
            if _is_concrete_config(hint):
                logger.debug(
                    "Found config model via class annotation on %s: %s",
                    klass.__name__,
                    hint.__name__,
                )
                return hint  # type: ignore[return-value]

        # 2. Inspect __init__ type annotation for 'config' parameter.
        try:
            hints = get_type_hints(cls.__init__)
            config_hint = hints.get("config")
            if config_hint is not None:
                hint = _unwrap_optional(config_hint)
                if _is_concrete_config(hint):
                    logger.debug(
                        "Found config model via __init__ annotation: %s",
                        hint.__name__,
                    )
                    return hint  # type: ignore[return-value]
                # Accept base ConverterConfig only as a last resort later.
                if isinstance(hint, type) and issubclass(hint, ConverterConfig):
                    logger.debug(
                        "Found config model via __init__ annotation: %s",
                        hint.__name__,
                    )
                    return hint  # type: ignore[return-value]
        except Exception:
            pass

        # 3. Fall back to explicit class attributes.
        for attr in ("Config", "config_model"):
            m = getattr(conv, attr, None)
            if m is not None and isinstance(m, type) and issubclass(m, ConverterConfig):
                logger.debug(
                    "Found config model via class attribute '%s': %s",
                    attr,
                    m.__name__,
                )
                return m  # type: ignore[return-value]

    except Exception:
        pass

    logger.debug(
        "No specific config model found for %s, using ConverterConfig",
        converter_type,
    )
    return ConverterConfig


def cast_value(value: str, target_type):  # NOSONAR
    """Cast a string prompt value to a basic Python type.

    Performs lightweight casting for common primitive types before a Pydantic
    model is instantiated.  Returns the original string on failure so that
    Pydantic validation can surface a meaningful error.

    Args:
        value: Raw string input from the user.
        target_type: Target Python type expected by the field.

    Returns:
        The cast value (``int`` / ``float`` / ``bool`` / ``str``) or the
        original string when casting is not possible.
    """
    if value is None:
        return None
    logger.debug("Casting %r to type %s", value, target_type)
    try:
        if target_type is bool:
            lower = value.strip().lower()
            if lower in ("1", "true", "yes", "y"):
                return True
            if lower in ("0", "false", "no", "n"):
                return False
            return bool(value)
        if target_type is int:
            return int(value)
        if target_type is float:
            return float(value)
    except Exception:
        logger.debug("Could not cast %r to %s, keeping as string", value, target_type)
    return value
