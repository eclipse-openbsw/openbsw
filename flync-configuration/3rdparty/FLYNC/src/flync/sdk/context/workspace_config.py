"""
Configuration module for FLYNC SDK.

Provides :class:`WorkspaceConfiguration` and :class:`ListObjectsMode`, which control how a
:class:`~flync.sdk.workspace.flync_workspace.FLYNCWorkspace` is loaded, validated, and serialized.
"""

import importlib.metadata
import logging
from enum import IntFlag
from pathlib import Path
from typing import Any, Type

import yaml
from packaging.version import Version
from pydantic import BaseModel, ConfigDict, Field, field_serializer, field_validator

from flync.model import FLYNCBaseModel, FLYNCModel
from flync.model.flync_4_metadata.metadata import BaseVersion

logger = logging.getLogger(__name__)

DEFAULT_EXTENSION = ".flync.yaml"

#: Per-workspace directory holding everything FLYNC tooling persists into a
#: workspace: the configuration file, and later views, templates and conversion
#: artifacts. One directory rather than a spread of root-level dotfiles.
CONFIG_DIRNAME = ".flync"

#: Basename of the workspace configuration file inside :data:`CONFIG_DIRNAME`.
CONFIG_FILENAME = "config.yaml"

#: Workspace configuration path, relative to a workspace root. Join this
#: against the root; never join :data:`CONFIG_FILENAME` on its own, which would
#: resolve to a stray file beside the workspace contents.
CONFIG_RELPATH = Path(CONFIG_DIRNAME) / CONFIG_FILENAME

#: Version reported when the ``flync`` distribution metadata cannot be read
#: (vendored copies, frozen or bundled deployments).
UNKNOWN_VERSION = "0.0.0"

#: Fields that are never read from, nor written to, ``.flync/config.yaml``.
#:
#: ``root_model`` is deliberately programmatic-only. Persisting it would mean naming a
#: model class in a workspace file, and loading such a workspace would then have to import
#: that class - i.e. execute code shipped inside the workspace. Workspaces are exchanged
#: between suppliers and OEMs, so they are treated as data, never as a source of importable
#: code. The root model is always supplied by the host application.
NON_PERSISTED_FIELDS = frozenset({"root_model"})


def _get_current_flync_version() -> BaseVersion:
    """
    Get the current FLYNC version as a BaseVersion.

    The version is derived from git tags at build time. A checkout that carries no release
    tag (a mirror that does not replicate them, or a shallow clone) instead yields a
    development version such as ``0.0.0.post20+07e3a84``. That names no release, and its
    local segment changes with every commit, so it is normalized to :data:`UNKNOWN_VERSION`.
    Since ``0.0.0`` sorts below every real release, a build that cannot determine its own
    version can never advance a stamp recorded by one that could.

    Returns:
        BaseVersion: Current FLYNC version in PEP 440 format, or :data:`UNKNOWN_VERSION`
        when it cannot be determined.
    """
    try:
        version_str = importlib.metadata.version("flync")
    except importlib.metadata.PackageNotFoundError:
        logger.warning(
            "Cannot determine FLYNC version: distribution 'flync' not found. Falling back to %s.",
            UNKNOWN_VERSION,
        )
        version_str = UNKNOWN_VERSION

    if Version(version_str).base_version == UNKNOWN_VERSION:
        logger.debug(
            "FLYNC version %s is not built from a release tag; recording %s instead.",
            version_str,
            UNKNOWN_VERSION,
        )
        version_str = UNKNOWN_VERSION

    return BaseVersion(version_schema="pep440", version=version_str)


class ListObjectsMode(IntFlag):
    """
    Flags controlling how list items are keyed in the workspace object map.

    Flags can be combined with ``|``. The default is ``INDEX | NAME``.

    Attributes:
        INDEX: Register each list item under its zero-based integer index (e.g. ``controllers.0``).
        NAME: Register each list item under its name — the file/directory stem for folder-based lists, or the model's ``name`` attribute for \
              inline YAML lists. Items without a name are skipped.
    """

    INDEX = 1
    NAME = 2


class WorkspaceConfiguration(BaseModel):
    """
    Configuration object for the FLYNC SDK workspace.

    A workspace can store this configuration on disk as ``.flync/config.yaml`` in its root
    (see :meth:`from_workspace` and :meth:`to_yaml_file`), so the workspace is self-describing
    and the same settings do not have to be rebuilt in a script every time it is opened.

    The persisted file only ever carries plain data. Fields listed in
    :data:`NON_PERSISTED_FIELDS` (currently ``root_model``) are set by the host application
    in code and are rejected when present in a configuration file.

    Attributes:
        flync_file_extension (str): The primary file extension used when writing FLYNC configuration files. Defaults to ``".flync.yaml"``.
        allowed_extensions (set[str]): Set of file extensions recognized as FLYNC files. Defaults to ``{".flync.yaml", ".flync.yml"}``.
        exclude_unset (bool): When ``True``, fields that were not explicitly set on a model are omitted from serialized output.
        root_model (Type[FLYNCBaseModel]): The root Pydantic model class used to validate workspace contents.
        Programmatic-only, never serialized. Defaults to :class:`~flync.model.flync_model.FLYNCModel`.
        map_objects (bool): tells the workspace if it should map all objects in the workspace (reduces performance).
        list_objects_mode (ListObjectsMode): Controls how objects are keyed when listed. Defaults to ``INDEX | NAME``.
        Accepts int, list of flag names, or pipe-separated string.
        persist_config (bool): When ``True`` (the default), saving the whole workspace also writes this
        configuration to ``.flync/config.yaml``. Set it to ``False`` for workspaces whose output directory
        should stay free of FLYNC tooling files - generated or converted output, scratch directories, or a
        workspace whose configuration file is maintained by hand. Only the implicit write is suppressed;
        :meth:`to_yaml_file` still writes when called directly.
        version (BaseVersion): FLYNC release that last wrote this configuration. Auto-detects the current version by default.
        Always serialized, and moved forward (never backwards) when a newer FLYNC rewrites the file.
        Tracking only: it is recorded to support future migrations and is not enforced on load.
    """

    model_config = ConfigDict(frozen=True, arbitrary_types_allowed=True, extra="forbid")

    flync_file_extension: str = DEFAULT_EXTENSION
    allowed_extensions: set[str] = {DEFAULT_EXTENSION, ".flync.yml"}
    exclude_unset: bool = True
    root_model: Type[FLYNCBaseModel] = FLYNCModel
    map_objects: bool = False
    list_objects_mode: ListObjectsMode = ListObjectsMode.INDEX | ListObjectsMode.NAME
    persist_config: bool = True
    version: BaseVersion = Field(default_factory=_get_current_flync_version)

    @field_validator("root_model", mode="before")
    @classmethod
    def validate_root_model(cls, v: Any) -> Any:
        """Reject model class *paths*; only a real class is accepted."""
        if isinstance(v, str):
            raise ValueError(
                "root_model must be a FLYNC model class, not a module path. Resolving a class by name is not supported: "
                "it would import code named by workspace data. Pass the class itself from the host application."
            )
        return v

    @field_validator("list_objects_mode", mode="before")
    @classmethod
    def validate_list_objects_mode(cls, v: Any) -> ListObjectsMode:
        """Convert list/string formats to ListObjectsMode IntFlag."""
        if isinstance(v, ListObjectsMode):
            return v
        if isinstance(v, int):
            return ListObjectsMode(v)

        mode_val = ListObjectsMode(0)

        if isinstance(v, list):
            # Support list format: ['INDEX', 'NAME']
            for flag_name in v:
                mode_val |= ListObjectsMode[flag_name]
        elif isinstance(v, str):
            # Support string format: "INDEX|NAME" or "INDEX"
            for flag_name in v.split("|"):
                flag_name = flag_name.strip()
                if flag_name.startswith("ListObjectsMode."):
                    flag_name = flag_name.replace("ListObjectsMode.", "")
                mode_val |= ListObjectsMode[flag_name]
        else:
            raise ValueError(f"Invalid list_objects_mode format: {type(v).__name__}")

        return mode_val

    @field_serializer("list_objects_mode")
    def serialize_list_objects_mode(self, v: ListObjectsMode) -> list[str]:
        """Serialize ListObjectsMode IntFlag to list of flag names for readability."""
        flags = []
        if ListObjectsMode.INDEX in v:
            flags.append("INDEX")
        if ListObjectsMode.NAME in v:
            flags.append("NAME")
        return flags

    @classmethod
    def from_yaml_file(cls, path: str | Path) -> "WorkspaceConfiguration":
        """
        Load WorkspaceConfiguration from a YAML file.

        Args:
            path: Path to YAML file (e.g., .flync/config.yaml).

        Returns:
            WorkspaceConfiguration instance with values from file.

        Raises:
            FileNotFoundError: If file doesn't exist.
            ValueError: If YAML is invalid, contains an unknown key, or sets a
            programmatic-only field (see :data:`NON_PERSISTED_FIELDS`).

        Example:
            >>> config = WorkspaceConfiguration.from_yaml_file(".flync/config.yaml")
        """
        file_path = Path(path)
        if not file_path.exists():
            raise FileNotFoundError(f"Configuration file not found: {file_path}")

        with open(file_path, "r") as f:
            data = yaml.safe_load(f) or {}

        if not isinstance(data, dict):
            raise ValueError(f"YAML file must contain a mapping, got {type(data).__name__}")

        rejected = sorted(NON_PERSISTED_FIELDS.intersection(data))
        if rejected:
            raise ValueError(
                f"{file_path}: {', '.join(rejected)} cannot be set from a configuration file. "
                "It is supplied by the host application in code so that opening a workspace never imports code it names."
            )

        # Pydantic validators handle conversion of list_objects_mode and version automatically.
        # Unknown keys are rejected (extra="forbid") so typos in a hand-edited file are not silently ignored.
        return cls(**data)

    @classmethod
    def from_workspace(cls, workspace_path: str | Path) -> "WorkspaceConfiguration":
        """
        Resolve the base configuration for a workspace directory.

        Loads ``CONFIG_RELPATH`` under the workspace root if it exists, otherwise returns defaults.
        This is the disk-backed base that server/runtime overrides layer on top of.

        The file is read as plain data only: nothing in it is imported or executed, and
        the workspace directory is never added to ``sys.path``.
        """
        config_path = Path(workspace_path) / CONFIG_RELPATH
        if config_path.exists():
            return cls.from_yaml_file(config_path)
        return cls()

    def _version_to_persist(self) -> BaseVersion:
        """
        Return the version stamp to write to disk.

        This is the newer of the running FLYNC release and the version this configuration
        carries. A file rewritten by a newer FLYNC is in that newer release's format, so
        leaving the old stamp in place would point a future migration at the wrong baseline.

        The stamp is never moved backwards: an older FLYNC rewriting a file that a newer one
        produced must not claim the workspace downgraded. Versions recorded under a different
        ``version_schema`` are not comparable, so those are left untouched.

        Returns:
            BaseVersion: The version to serialize.
        """
        current = _get_current_flync_version()
        if self.version.version_schema != current.version_schema:
            return self.version
        return current if current.version > self.version.version else self.version

    def to_yaml_file(self, path: str | Path) -> None:
        """
        Save WorkspaceConfiguration to a YAML file.

        Only non-default values are written, making configs concise and readable.
        The ``version`` field is always serialized to track FLYNC compatibility.
        Programmatic-only fields (see :data:`NON_PERSISTED_FIELDS`) are never written.

        Args:
            path: Path to write YAML file (e.g., .flync/config.yaml).

        Example:
            >>> config = WorkspaceConfiguration(map_objects=True)
            >>> config.to_yaml_file(".flync/config.yaml")
        """
        file_path = Path(path)
        file_path.parent.mkdir(parents=True, exist_ok=True)

        if self.root_model is not FLYNCModel:
            logger.debug(
                "root_model %s is not persisted to %s; it must be supplied in code when the workspace is reopened.",
                getattr(self.root_model, "__name__", self.root_model),
                file_path.name,
            )

        # Derive the payload from the model itself so it cannot drift from the field defaults.
        data: dict[str, Any] = self.model_dump(exclude_defaults=True, exclude={"version", *NON_PERSISTED_FIELDS})

        # ``exclude_defaults`` compares serialized output, so a field with a custom
        # ``field_serializer`` (list_objects_mode) is kept even at its default. Drop those
        # by comparing the attribute against the declared default instead.
        #
        # Creating a list is required to delete while we iterate!
        for name in list(data.keys()):
            field = type(self).model_fields.get(name)
            if field is not None and getattr(self, name) == field.default:
                del data[name]

        # Sets have no stable YAML representation; write a sorted sequence instead.
        if "allowed_extensions" in data:
            data["allowed_extensions"] = sorted(data["allowed_extensions"])

        # Always serialize version (even if default), so the file records which FLYNC
        # release last wrote it and a later release can migrate it.
        version = self._version_to_persist()
        data["version"] = {
            "version_schema": version.version_schema,
            "version": str(version.version),
        }

        # Write YAML
        with open(file_path, "w") as f:
            yaml.dump(data, f, default_flow_style=False, sort_keys=False)

    @classmethod
    def create_from_config(cls, existing_config: "WorkspaceConfiguration", **configs) -> "WorkspaceConfiguration":
        """
        Create a new configuration by overriding fields on an existing one.

        Converts ``existing_config`` to a dict, applies ``configs`` on top, then constructs and returns a new :class:`WorkspaceConfiguration`.

        Args:
            existing_config (WorkspaceConfiguration): The base configuration to copy from.
            configs: Field names and new values to override.

        Returns:
            WorkspaceConfiguration: A new instance with the overrides applied.
        """

        existing_config_values = existing_config.model_dump()
        existing_config_values.update(**configs)
        return WorkspaceConfiguration(**existing_config_values)
