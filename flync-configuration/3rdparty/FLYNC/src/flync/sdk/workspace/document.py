"""Helper for working with YAML documents."""

from pathlib import Path
from typing import Any

from ruamel.yaml import YAML

from flync.sdk.utils.sdk_types import PathType

_yaml_cache: dict[str, YAML] = {}


def _get_yaml(typ: str):
    """
    Create a YAML parser instance appropriate for parsing.

    Args:
        typ (str): type of loader.

    Returns:
        YAML: Configured ruamel.yaml parser with preserved quotes.
    """
    yaml = _yaml_cache.get(typ)
    if yaml is None:
        yaml = YAML(typ=typ)
        yaml.preserve_quotes = True
        _yaml_cache[typ] = yaml
    return yaml


class Document(object):
    """
    Represents a YAML document with parsing capabilities.

    Attributes:
        uri (str): The unique identifier for the document.

        text (str): The raw YAML content.

        ast (Any | None): The parsed abstract syntax tree, or None if not parsed.

        compose_ast (Any | None): The composed ruamel.yaml AST used for source-position tracking, or None if not parsed.
        needs_compose (bool): Whether to produce a composed AST.
    """

    _yaml_safe = _get_yaml("safe")

    def __init__(self, uri: PathType, text: str, needs_compose: bool):
        """
        Initialize a Document instance.

        Args:
            uri (str): The document's URI.
            text (str): The raw YAML text.
            needs_compose (bool): Whether to produce a composed AST for source tracking.
        """

        self.uri: PathType = uri
        self.needs_compose = needs_compose
        self.ast: Any | None = None
        self.compose_ast = None
        self.text: str = text

    def parse(self):
        """
        Parse the YAML text into an abstract syntax tree.

        Sets :attr:`ast` via ``yaml.load`` and :attr:`compose_ast` via ``yaml.compose``, both derived from :attr:`text`.

        Returns: None
        """
        self.ast, self.compose_ast = self._parse_text(self.text, self.needs_compose)

    def update_text(self, text: str):
        """
        Update the document's text and re-parse it.

        Args:
            text (str): The new YAML content.

        Returns: None
        """

        self.text = text
        self.parse()

    def assign_ast(self, ast, compose_ast):
        """
        Assign parsed YAML structures to the Document instance.

        Args:
            ast: Parsed YAML object tree.
            compose_ast: Composed YAML node tree (optional, used for object maps).
        """
        self.ast = ast
        self.compose_ast = compose_ast

    @classmethod
    def _get_safe_yaml(cls):
        """
        Return a safe YAML parser instance.

        Returns:
            YAML: A ruamel.yaml parser configured for safe loading.
        """
        return cls._yaml_safe

    @classmethod
    def _parse_text(cls, text: str, needs_compose: bool):
        """
        Parse YAML text into AST and optionally a composed AST.

        Args:
            text (str): YAML source text.
            needs_compose (bool): Whether to also produce a composed AST.

        Returns:
            tuple: (ast, compose_ast) where compose_ast may be None.
        """
        compose_ast = None
        if needs_compose:
            # ruamel.yaml YAML instances are not thread-safe: they store
            # per-parse composer state on the instance itself.
            compose_ast = _get_yaml("rt").compose(text)
        ast = cls._get_safe_yaml().load(text)
        return ast, compose_ast

    @classmethod
    def normalize_uri(cls, uri: PathType, ws_root: Path | None) -> str:
        """
        Normalize a file URI relative to the workspace root.

        Args:
            uri (Path): File path to normalize.
            ws_root (Path): Workspace root path.

        Returns:
            str: Normalized URI as POSIX-style string.
        """
        if isinstance(uri, str):
            uri = Path(uri)
        if uri.is_absolute():
            uri = uri.relative_to(ws_root)  # type: ignore[arg-type]
        return uri.as_posix()


def read_file(path: PathType) -> str:
    """
    Read a file as UTF-8 text.

    Args:
        path (PathType): Path to the file.

    Returns:
        str: File contents if successful, empty string if file cannot be read.
    """
    try:
        with open(path, "r", encoding="utf-8") as direct_data:
            return direct_data.read()
    except (OSError, UnicodeDecodeError):
        return ""


def parse_documents(paths, ws_root, needs_compose):
    """
    Parse multiple YAML documents from given paths.

    The raw text is read inside each worker and NOT returned through IPC
    to avoid pickle overhead. The main process re-reads the file from the
    OS page cache for Document construction.

    Args:
        paths (list[PathType]): List of file paths to parse.
        ws_root (Path): Workspace root for URI normalization.
        needs_compose (bool): Whether to produce composed ASTs.

    Returns:
        list[tuple[str, Any, Any | None]]: Each tuple contains
            (normalized_uri, ast, compose_ast).
    """

    results = []

    for path in paths:
        text = read_file(path)
        ast, compose_ast = Document._parse_text(text, needs_compose)
        results.append(
            (
                Document.normalize_uri(path, ws_root),
                ast,
                compose_ast,
            )
        )
    return results
