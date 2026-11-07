"""Pluggy hook specifications for FLYNC converter plugins."""

import pluggy

hookspec = pluggy.HookspecMarker("flync_converter")


@hookspec
def register_converters():
    """Return list[Converter]"""
