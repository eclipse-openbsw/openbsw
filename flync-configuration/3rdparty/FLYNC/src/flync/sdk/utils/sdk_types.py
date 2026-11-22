"""
Shared type variables for the FLYNC SDK.

Provides common type aliases used across SDK modules.
"""

from pathlib import Path

type PathType = Path | str
"""A type alias that accepts either a :class:`pathlib.Path` or a plain :class:`str`."""
