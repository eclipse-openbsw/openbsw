"""
Detect pre-0.11 single-file Controller configurations.

Up to FLYNC 0.10.x a Controller was defined in a single file ``<ecu>/controllers/<name>.flync.yaml``
whose top-level dict carried a ``meta`` block holding ``compatible_flync_version``.
Starting with FLYNC 0.11 the controller is a directory with the metadata moved into a ``controller_metadata`` field
(file ``<ecu>/controllers/<name>/controller_metadata.flync.yaml``).

When the workspace loader encounters a legacy single-file controller it merges the file's contents into the dict that reaches Controller
validation, so the legacy ``meta`` key is still visible at ``mode='before'``.
This module exposes helpers that recognize that shape and raise a fatal validation error, prompting the user to either update the configuration to
the new layout or downgrade FLYNC to the 0.10.x line.
"""

from typing import Any, Optional

from flync.core.utils.exceptions import Category, err_fatal

_LEGACY_META_KEY = "meta"
_VERSION_KEY = "compatible_flync_version"
_REQUIRED_FLYNC_VERSION = "0.11.0"


def detect_legacy_controller_version(data: Any) -> Optional[str]:
    """
    Return the legacy controller's ``compatible_flync_version`` when ``data`` matches the pre-0.11 single-file shape; ``None`` otherwise.

    A legacy payload has a top-level ``meta`` mapping whose ``compatible_flync_version`` mapping carries a ``version`` entry —
    none of these keys exist on a 0.11+ Controller dict, so their joint presence is a reliable signal.
    """

    if not isinstance(data, dict):
        return None

    legacy_meta = data.get(_LEGACY_META_KEY)
    compat = legacy_meta.get(_VERSION_KEY) if isinstance(legacy_meta, dict) else None
    version = compat.get("version") if isinstance(compat, dict) else None
    return None if version is None else str(version)


def reject_legacy_controller(data: Any) -> None:
    """
    Raise a fatal error when ``data`` is a legacy Controller payload.

    No-op for non-legacy payloads.  Designed to be invoked from a Pydantic ``mode='before'`` model validator on
    :class:`flync.model.flync_4_ecu.controller.Controller`.
    """

    legacy_version = detect_legacy_controller_version(data)
    if legacy_version is None:
        return
    raise err_fatal(
        "Incompatible Controller Config detected (compatible_flync_version={legacy_version}). "
        "FLYNC {required_version} requires every controller to live in its own directory containing 'controller_metadata.flync.yaml'. "
        "Update the configuration to the new layout or downgrade FLYNC to 0.10.x.",
        legacy_version=legacy_version,
        required_version=_REQUIRED_FLYNC_VERSION,
        category=Category.COMPATIBILITY,
        error_number="048",
    )
