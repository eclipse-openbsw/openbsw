"""
Model-to-disk serialization for the FLYNC workspace.

Turns a :class:`~flync.core.base_models.base_model.FLYNCBaseModel` into workspace documents,
routing externally annotated fields to their own files or folders.
"""

import logging
from pathlib import Path

import yaml

from flync.core.annotations import (
    External,
    Implied,
    ImpliedStrategy,
    NamingStrategy,
    OutputStrategy,
)
from flync.core.base_models.base_model import FLYNCBaseModel
from flync.sdk.context.workspace_config import CONFIG_RELPATH
from flync.sdk.utils.field_utils import (
    get_metadata,
    get_name,
)
from flync.sdk.utils.model_dumper import dump_model_with_discriminators
from flync.sdk.utils.sdk_types import PathType

from ._incremental import _WorkspaceIncremental
from .document import Document

logger = logging.getLogger(__name__)


class _WorkspaceSaving(_WorkspaceIncremental):
    """Serializes models into workspace documents."""

    def load_flync_model(self, flync_model: FLYNCBaseModel, file_path: PathType = ""):
        """
        Load a FLYNCModel into the workspace.

        This is a placeholder implementation that stores the model for later
        use.
        """

        if isinstance(file_path, str):
            file_path = Path(file_path)
        content = self.__get_model_content(flync_model, file_path)
        self.__save_content_to_file(file_path, content)

    def save_workspace_config(self) -> None:
        """
        Save the workspace configuration to .flync/config.yaml.

        Serializes the current :class:`~flync.sdk.context.workspace_config.WorkspaceConfiguration`
        to the workspace root as ``.flync/config.yaml``. This is typically called after
        modifying workspace configuration to persist the changes.

        Calling this writes the file unconditionally. ``persist_config`` only governs the
        implicit write that :meth:`generate_configs` performs, so asking for the file
        directly always produces it.

        Example:
            >>> workspace = FLYNCWorkspace.load_workspace("myproject", "/path/to/project")
            >>> # ... modify workspace.configuration if needed ...
            >>> workspace.save_workspace_config()
        """
        if not self.workspace_root:
            raise ValueError("Unable to save workspace configuration: workspace root is not defined.")
        config_path = self.workspace_root / CONFIG_RELPATH
        self.configuration.to_yaml_file(config_path)

    def __save_content_to_file(self, file_path: Path, content):
        """
        Persist serialized model content as a Document in the workspace.

        Resolves the full URI under the workspace root, creates a :class:`~flync.sdk.workspace.document.Document` for it, and calls
        :meth:`generate_configs` to write it to disk. Does nothing when ``content`` is empty (e.g. all fields were external).

        Args:
            file_path (Path): Relative path (without extension) for the file.
            content: The serialized content to write; may be a ``dict``, a ``list``, or a plain string.
        """

        if not content:
            # everything in the object was external,
            # no need to create a document
            return
        if not self.workspace_root:
            raise ValueError("Unable to save contents in a workspace, the workspace root is not defined.")
        uri = self.workspace_root / file_path.with_suffix(self.configuration.flync_file_extension)
        doc = Document(uri, content, self.configuration.map_objects)
        self.documents[str(uri)] = doc
        self.generate_configs(uri)

    def __get_model_content(self, flync_model: FLYNCBaseModel, file_path):
        """
        Serialize a model to a dict, routing external fields to separate documents.

        Iterates over the model's fields.
        Fields annotated with :class:`~flync.core.annotations.External` are excluded from the returned dict and handled recursively.
        Fields with :class:`~flync.core.annotations.
        Implied` ``FOLDER_NAME`` strategy are also excluded (their value is inferred from the directory name at load time).

        Args:
            flync_model (FLYNCBaseModel): The model instance to serialize.
            file_path (Path): The base file path used when routing external fields.

        Returns:
            dict: The serialized content with external and implied fields excluded.
        """

        exclude = set()
        for field_name, field_info in type(flync_model).model_fields.items():
            external: External | None = get_metadata(field_info.metadata, External)
            if external is not None:
                exclude.add(field_name)
                # field will need to be added to to a new separate document
                flync_attribute = getattr(flync_model, field_name)
                self.__handle_load_external_types(file_path, flync_attribute, external, field_name)
                continue
            implied: Implied | None = get_metadata(field_info.metadata, Implied)
            if implied is not None and implied.strategy in (
                ImpliedStrategy.FOLDER_NAME,
                ImpliedStrategy.FILE_NAME,
            ):
                exclude.add(field_name)

        content = dump_model_with_discriminators(flync_model, exclude=exclude, exclude_unset=self.configuration.exclude_unset)
        return content

    def __handle_load_external_types(
        self,
        file_path: Path,
        flync_attribute,
        external: External,
        field_name: str,
    ):
        """
        Dispatch an external field value to the correct save handler.

        Determines the output path from the :class:`~flync.core.annotations.External`
        naming strategy, then delegates to the appropriate handler based on
        whether the attribute is a list, dict, or a :class:`FLYNCBaseModel`.

        Args:
            file_path (Path): Base path of the parent document.
            flync_attribute: The field value to save externally.
            external (External): The ``External`` annotation controlling naming and output structure.
            field_name (str): The field name, used as the default path when ``FIELD_NAME`` strategy is active.

        Raises:
            ValueError: If no valid external path can be determined or the attribute type is not supported.
        """

        if flync_attribute is None or not flync_attribute:
            # none field, do nothing
            return
        if external.naming_strategy == NamingStrategy.FIXED_PATH and external.path is not None:
            external_path = external.path
        elif external.naming_strategy == NamingStrategy.FIELD_NAME:
            external_path = field_name
        else:
            raise ValueError("Unable to find an external path for {}", field_name)
        next_path = file_path / external_path
        if isinstance(flync_attribute, list):
            self.__handle_load_external_types_list(flync_attribute, external, next_path, field_name)
        elif isinstance(flync_attribute, dict):
            self.__handle_load_external_types_dict(flync_attribute, external, next_path)
        elif isinstance(flync_attribute, FLYNCBaseModel):
            if OutputStrategy.SINGLE_FILE in external.output_structure and OutputStrategy.OMMIT_ROOT not in external.output_structure:
                content = self.__get_model_content(flync_attribute, next_path)
                self.__save_content_to_file(next_path, {field_name: content})
            else:
                self.load_flync_model(flync_attribute, next_path)
        else:
            raise ValueError("Unable to load object {} from flync object", field_name)

    def __handle_load_external_types_list(
        self,
        flync_attribute: list,
        external: External,
        next_path: Path,
        field_name: str,
    ):
        """
        Save a list of external model instances to their output locations.

        When ``output_structure`` is ``SINGLE_FILE``, all items are serialized into a single file.
        Otherwise each item is written to its own file named after its ``name`` attribute (or the implied file-name field).

        Args:
            flync_attribute (list): The list of model instances to persist.
            external (External): The ``External`` annotation for this field.
            next_path (Path): The resolved output directory path.
            field_name (str): The field name, used as the key when writing a combined single-file output.
        """

        list_content = []
        for attr in flync_attribute:
            if OutputStrategy.SINGLE_FILE in external.output_structure:
                list_content.append(self.__get_model_content(attr, next_path))
            else:
                self.load_flync_model(
                    attr,
                    next_path / get_name(attr, self.__get_field_filename(attr)),
                )
        if len(list_content) != 0:
            self.__save_content_to_file(next_path, {field_name: list_content})

    def __handle_load_external_types_dict(self, flync_attribute: dict, external: External, next_path: Path):
        """
        Save a dict of external model instances to their output locations.

        When ``output_structure`` is ``SINGLE_FILE``, all values are aggregated into a single file keyed by their original dict keys.
        Otherwise each value is written to its own file named after its key.

        Args:
            flync_attribute (dict): The dict of model instances to persist.
            external (External): The ``External`` annotation for this field.
            next_path (Path): The resolved output directory path.
        """

        dict_content = {}
        for attr_name, attr_value in flync_attribute.items():
            if external.output_structure == OutputStrategy.SINGLE_FILE:
                dict_content[attr_name] = self.__get_model_content(attr_value, next_path)
            else:
                self.load_flync_model(attr_value, next_path / attr_name)

    @staticmethod
    def __get_field_filename(model: FLYNCBaseModel):
        """
        Return the field name whose value supplies the output filename.

        Searches the model's fields for one annotated with :class:`~flync.core.annotations.Implied` using the ``FILE_NAME`` strategy.

        Args:
            model (FLYNCBaseModel): The model instance to inspect.

        Returns:
            str | None: The field name to use as the file name, or ``None`` if no such field exists.
        """

        for field, info in type(model).model_fields.items():
            implied: Implied | None = get_metadata(info.metadata, Implied)
            if implied and implied.strategy == ImpliedStrategy.FILE_NAME:
                return field

        return None

    def generate_configs(self, uri: PathType | None = None, *, persist_config: bool | None = None):
        """
        Save the workspace to the given path.

        Creates the output directory (if it does not exist) and writes a simple representation of the workspace.
        If a FLYNCModel has been loaded via ``load_flync_model``, it attempts to serialize the model to JSON.
        Also persists the workspace configuration to .flync/config.yaml (only when saving the entire workspace).

        Args:
            uri (str | Path | None): Optional argument to save specific file instead of the entire workspace.
            persist_config (bool | None): One-off override for ``WorkspaceConfiguration.persist_config``.
            ``False`` suppresses the ``.flync/config.yaml`` write for this call - use it when writing to a directory that should hold nothing but
            the generated FLYNC files. ``None`` (the default) follows the workspace configuration. Ignored when ``uri`` is given, since a
            single-document save never writes the configuration.

        Returns: None
        """

        if uri is not None:
            uri = str(uri)
            if uri not in self.documents:
                raise ValueError(f"Document with URI {uri} not found in workspace.")
        docs = [self.documents[uri]] if uri else self.documents.values()
        for doc in docs:
            # create file
            path_from_uri: Path = Path(doc.uri)
            path_from_uri.parent.mkdir(parents=True, exist_ok=True)
            if isinstance(doc.text, str):
                path_from_uri.write_text(doc.text, encoding="utf-8")
            elif isinstance(doc.text, dict) or isinstance(doc.text, list):
                with open(path_from_uri, "w", encoding="utf-8") as f:
                    yaml.dump(
                        doc.text,
                        f,
                        sort_keys=False,
                        default_flow_style=False,
                        allow_unicode=True,
                    )

        # Persist workspace configuration when saving entire workspace, unless suppressed.
        should_persist = self.configuration.persist_config if persist_config is None else persist_config
        if uri is None and should_persist:
            self.save_workspace_config()
