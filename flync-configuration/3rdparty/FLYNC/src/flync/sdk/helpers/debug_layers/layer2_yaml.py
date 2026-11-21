"""Layer 2: YAML syntax validation.

Parses every .flync.yaml file under the target directory with ruamel.yaml
(the same library the workspace uses).  Reports any file that cannot be
parsed as valid YAML before the schema loader even tries to read it.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

FLYNC_EXT = ".flync.yaml"


@dataclass
class YAMLIssue(object):
    """A single YAML parsing/structure issue found in Layer 2 checks."""

    message: str
    path: str
    line: int | None = None
    col: int | None = None


def check_yaml_syntax(dir_path: Path) -> list[YAMLIssue]:
    """
    Parse every *.flync.yaml file under dir_path and return one YAMLIssue
    for each file that contains a syntax error.
    """
    issues: list[YAMLIssue] = []

    try:
        from ruamel.yaml import YAML
        from ruamel.yaml.parser import ParserError
        from ruamel.yaml.scanner import ScannerError

        _check_with_ruamel(dir_path, issues, YAML, scanner_error=ScannerError, parser_error=ParserError)
    except ImportError:
        # Fallback: plain PyYAML (also a project dependency)
        _check_with_pyyaml(dir_path, issues)

    return issues


def _check_with_ruamel(
    dir_path: Path,
    issues: list[YAMLIssue],
    yaml_cls,
    scanner_error,
    parser_error,
) -> None:
    """Parse every .flync.yaml file under dir_path with ruamel.yaml, collecting syntax issues."""
    yaml = yaml_cls()
    yaml.preserve_quotes = True

    for yaml_file in sorted(dir_path.rglob(f"*{FLYNC_EXT}")):
        try:
            with open(yaml_file, encoding="utf-8") as fh:
                yaml.load(fh)
        except (scanner_error, parser_error) as exc:
            mark = getattr(exc, "problem_mark", None)
            issues.append(
                YAMLIssue(
                    message=getattr(exc, "problem", str(exc)) or str(exc),
                    path=str(yaml_file.relative_to(dir_path)),
                    line=mark.line + 1 if mark else None,
                    col=mark.column + 1 if mark else None,
                )
            )
        except Exception as exc:
            issues.append(
                YAMLIssue(
                    message=str(exc),
                    path=str(yaml_file.relative_to(dir_path)),
                )
            )


def _check_with_pyyaml(dir_path: Path, issues: list[YAMLIssue]) -> None:
    """Parse every .flync.yaml file under dir_path with PyYAML, collecting syntax issues."""
    import yaml
    from yaml import YAMLError

    for yaml_file in sorted(dir_path.rglob(f"*{FLYNC_EXT}")):
        try:
            with open(yaml_file, encoding="utf-8") as fh:
                yaml.safe_load(fh)
        except YAMLError as exc:
            mark = getattr(exc, "problem_mark", None)
            issues.append(
                YAMLIssue(
                    message=str(exc),
                    path=str(yaml_file.relative_to(dir_path)),
                    line=mark.line + 1 if mark else None,
                    col=mark.column + 1 if mark else None,
                )
            )
