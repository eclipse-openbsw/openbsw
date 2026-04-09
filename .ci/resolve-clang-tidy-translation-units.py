"""
Resolve clang-tidy translation units from changed source/header files.

This helper is used by the branch-mode clang-tidy CI flow after the build step.
It takes a diff-derived file list and expands header-only changes into affected
translation units by reusing dependency metadata from the build directory.

Inputs:
- --build-directory: Build folder containing compile_commands.json and Ninja
    dependency metadata (.ninja_deps).
- --changed-files: Newline-separated list of files from git diff. This list may
    contain source files and headers.
- --output-file: Destination file for resolved translation units.

Behavior:
- Changed source files (.c/.cc/.cpp) are kept only when they are present in the
    current build's compile_commands.json.
- Changed headers (.h/.hh/.hpp/.hxx) are mapped to translation units by parsing
    `ninja -t deps` output and joining dependency targets with compile_commands
    output mappings.
- The result is written as repository-relative source file paths, one per line.

This script does not run clang-tidy itself; it only prepares the source-file
list consumed by .ci/clang-tidy.py (`--file-list`).
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path


SOURCE_SUFFIXES = (".c", ".cc", ".cpp")
HEADER_SUFFIXES = (".h", ".hh", ".hpp", ".hxx")


def get_repo_root() -> Path:
    """Return the repository root derived from this script's location."""
    return Path(__file__).resolve().parents[1]


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    """Parse CLI arguments for translation-unit resolution."""
    parser = argparse.ArgumentParser(
        description="Resolve changed headers to clang-tidy translation units"
    )
    parser.add_argument(
        "--build-directory",
        type=Path,
        required=True,
        help="Build directory containing compile_commands.json and Ninja metadata",
    )
    parser.add_argument(
        "--changed-files",
        type=Path,
        required=True,
        help="Path to newline-separated changed files from git diff",
    )
    parser.add_argument(
        "--output-file",
        type=Path,
        required=True,
        help="Output file with translation units to run clang-tidy on",
    )
    return parser.parse_args(argv)


def is_source(path: Path) -> bool:
    """Return True when the path has a supported source-file suffix."""
    return path.suffix in SOURCE_SUFFIXES


def is_header(path: Path) -> bool:
    """Return True when the path has a supported header-file suffix."""
    return path.suffix in HEADER_SUFFIXES


def read_changed_files(path: Path, repo_root: Path) -> tuple[set[Path], set[Path]]:
    """Read and classify changed files into source and header path sets."""
    sources: set[Path] = set()
    headers: set[Path] = set()

    for line in path.read_text(encoding="utf-8").splitlines():
        value = line.strip()
        if not value:
            continue

        candidate = (repo_root / value).resolve(strict=False)
        if is_source(candidate):
            sources.add(candidate)
        elif is_header(candidate):
            headers.add(candidate)

    return sources, headers


def build_output_to_source_map(build_directory: Path, repo_root: Path) -> dict[str, Path]:
    """Map possible Ninja object target names to their source files."""
    compile_commands = build_directory / "compile_commands.json"
    try:
        raw_compile_commands = compile_commands.read_text(encoding="utf-8")
    except FileNotFoundError as exc:
        raise RuntimeError(
            f"Missing {compile_commands}. Ensure the CI build step generated compile_commands.json."
        ) from exc

    try:
        data = json.loads(raw_compile_commands)
    except json.JSONDecodeError as exc:
        raise RuntimeError(
            f"Invalid JSON in {compile_commands}: {exc.msg} at line {exc.lineno} column {exc.colno}."
        ) from exc

    if not isinstance(data, list):
        raise RuntimeError(f"Expected {compile_commands} to contain a JSON list of compile commands.")

    mapping: dict[str, Path] = {}
    for index, entry in enumerate(data):
        if not isinstance(entry, dict):
            raise RuntimeError(
                f"Invalid compile command at index {index} in {compile_commands}: expected an object."
            )

        file_value = entry.get("file")
        if not file_value:
            raise RuntimeError(
                f"Invalid compile command at index {index} in {compile_commands}: missing 'file'."
            )

        source = Path(file_value).resolve(strict=False)
        output = entry.get("output")

        if output:
            mapping[output] = source

            output_path = Path(output)
            if not output_path.is_absolute():
                output_path = (repo_root / output_path).resolve(strict=False)
            else:
                output_path = output_path.resolve(strict=False)
            mapping[str(output_path)] = source

            # Ninja deps typically reports object paths relative to build dir.
            try:
                relative_to_build = output_path.relative_to(build_directory)
                mapping[str(relative_to_build)] = source
            except ValueError:
                pass

        object_guess = source.with_suffix(source.suffix + ".o")
        try:
            relative_guess = object_guess.relative_to(repo_root)
            mapping[str(relative_guess)] = source
        except ValueError:
            pass

    return mapping


def parse_ninja_deps(build_directory: Path) -> dict[str, set[Path]]:
    """Parse `ninja -t deps` output into target-to-dependency mappings."""
    cmd = ["ninja", "-C", str(build_directory), "-t", "deps"]
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
    except FileNotFoundError as exc:
        raise RuntimeError(
            "The 'ninja' executable is not available in the CI environment."
        ) from exc
    except subprocess.CalledProcessError as exc:
        details = (exc.stderr or exc.stdout or "no additional output").strip()
        raise RuntimeError(
            f"Failed to read Ninja dependency data from {build_directory}: {details}"
        ) from exc

    deps_by_target: dict[str, set[Path]] = {}
    current_target: str | None = None

    for raw_line in result.stdout.splitlines():
        line = raw_line.rstrip()
        if not line:
            continue

        if not raw_line.startswith(" ") and ":" in line:
            current_target = line.split(":", 1)[0].strip()
            deps_by_target[current_target] = set()
            continue

        if current_target is None:
            continue

        dep = line.strip()
        if dep:
            dep_path = Path(dep)
            if not dep_path.is_absolute():
                dep_path = (build_directory / dep_path).resolve(strict=False)
            else:
                dep_path = dep_path.resolve(strict=False)
            deps_by_target[current_target].add(dep_path)

    return deps_by_target


def filter_built_sources(
    changed_sources: set[Path], output_to_source: dict[str, Path]
) -> tuple[set[Path], set[Path]]:
    """Split changed sources into built and skipped sets for this build."""
    built_sources = set(output_to_source.values())
    return changed_sources.intersection(built_sources), changed_sources.difference(built_sources)


def resolve_translation_units(
    changed_sources: set[Path],
    changed_headers: set[Path],
    output_to_source: dict[str, Path],
    deps_by_target: dict[str, set[Path]],
) -> set[Path]:
    """Resolve final source files by expanding changed headers to owning TUs."""
    resolved_sources, _ = filter_built_sources(changed_sources, output_to_source)
    if not changed_headers:
        return resolved_sources

    for target, deps in deps_by_target.items():
        source = output_to_source.get(target)
        if source is None:
            continue

        if deps.intersection(changed_headers):
            resolved_sources.add(source)

    return resolved_sources


def write_sources(output_file: Path, repo_root: Path, sources: set[Path]) -> None:
    """Write resolved sources as sorted repository-relative paths."""
    lines = []
    for source in sorted(sources):
        try:
            lines.append(str(source.relative_to(repo_root)))
        except ValueError:
            continue

    output_file.parent.mkdir(parents=True, exist_ok=True)
    output_file.write_text("\n".join(lines) + ("\n" if lines else ""), encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    """Run the resolver workflow and write translation-unit output."""
    args = parse_args(argv)
    repo_root = get_repo_root()

    changed_sources, changed_headers = read_changed_files(args.changed_files, repo_root)

    if not changed_sources and not changed_headers:
        args.output_file.write_text("")
        print("No changed source/header files selected for clang-tidy.")
        return 0

    try:
        output_to_source = build_output_to_source_map(args.build_directory, repo_root)
        deps_by_target = parse_ninja_deps(args.build_directory) if changed_headers else {}
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        return 1

    built_changed_sources, skipped_changed_sources = filter_built_sources(
        changed_sources, output_to_source
    )
    if skipped_changed_sources:
        print(
            f"Warning: changed source files not present in compile_commands for {args.build_directory} were skipped:",
            file=sys.stderr,
        )
        for source in sorted(skipped_changed_sources):
            try:
                source_display = source.relative_to(repo_root)
            except ValueError:
                source_display = source
            print(f"  {source_display}", file=sys.stderr)

    if changed_headers and not output_to_source:
        print(
            "No compile command entries were available to map changed headers to translation units.",
            file=sys.stderr,
        )
        return 1

    if changed_headers and not deps_by_target:
        print(
            f"No Ninja dependency data was available in {args.build_directory} for changed header resolution.",
            file=sys.stderr,
        )
        return 1

    resolved_sources = resolve_translation_units(
        built_changed_sources, changed_headers, output_to_source, deps_by_target
    )

    write_sources(args.output_file, repo_root, resolved_sources)
    if resolved_sources:
        print("Resolved translation units:")
        for source in sorted(resolved_sources):
            print(source)
    else:
        if changed_headers:
            print(
                "Warning: changed headers did not resolve to any translation units.",
                file=sys.stderr,
            )
        print("No translation units resolved from changed source/header files.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
