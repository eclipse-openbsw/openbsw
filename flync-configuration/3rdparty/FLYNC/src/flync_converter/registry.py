"""Plugin registry that discovers and instantiates FLYNC converter factories."""

import logging

import pluggy

from .base.base_converter import BaseConverter
from .hookspec import hookspec  # noqa

logger = logging.getLogger(__name__)

hookimpl = pluggy.HookimplMarker("flync_converter")

pm = pluggy.PluginManager("flync_converter")
pm.add_hookspecs(__import__("flync_converter.hookspec").hookspec)


class ConverterFactoryRegistry(dict[str, BaseConverter]):
    """Registry mapping converter names to their BaseConverter instances."""

    def load_builtin(self):
        """Register the built-in json, yaml, flync, and dbc converters via pluggy."""
        from .converters import dbc, flync_converter, json_converter, yaml_converter

        for mod in (json_converter, yaml_converter, flync_converter, dbc):
            if not pm.is_registered(mod):
                logger.debug("Registering built-in plugin: %s", mod.__name__)
                pm.register(mod)
            else:
                logger.debug(
                    "Built-in plugin already registered, skipping: %s",
                    mod.__name__,
                )

    def load_plugins(self):
        """Load built-in converters and discover entry-point plugins, populating the registry."""
        logger.debug("Loading built-in converters")
        self.load_builtin()
        logger.debug("Loading setuptools entry-point plugins")
        pm.load_setuptools_entrypoints("flync_converter")
        for plugin in pm.get_plugins():
            for conv in plugin.register_converters():
                logger.debug(
                    "Registering converter: %s (from %s)",
                    conv.name,
                    plugin.__name__,
                )
                self[conv.name] = conv
        logger.debug(
            "Plugin loading complete. Registered converters: %s",
            list(self.keys()),
        )

    def pick(self, _obj) -> BaseConverter:
        """Return the first registered converter that reports it can decode."""
        for conv in self.values():
            if conv.can_decode():
                return conv
        raise ValueError("No converter found")


registry = ConverterFactoryRegistry()
