"""
Identifier types for the FLYNC SDK workspace.

Defines typed aliases used to uniquely address semantic objects within a workspace.
"""

from typing import NewType

ObjectId = NewType("ObjectId", str)
"""A string-based unique identifier for a semantic object in the workspace."""
