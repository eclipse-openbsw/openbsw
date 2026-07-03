#!/bin/env python3
# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

"""Generate C++ files from YAML properties using predefined template mappings.

Usage:
    python jinja2cpp.py --input <base_dir> --output <base_dir> [--deployment-yaml <yaml_file>] [--clang-format-config <config_file>]

The script will generate all configured file types:
- Queue definitions (queue_definitions.h/cpp)
- Allocator definitions (allocator_selector_definitions.cpp)
- Cluster ID (cluster_id.h)
- Cluster connections (cluster_*.h/cpp)
- Services (service_*.h/cpp)

Arguments:
    --input                 Base input directory containing templates/ subdirectory
    --output                Base output directory where generated code will be written
    --deployment-yaml       Optional path to a deployment YAML file overriding
                           templates/properties/deployment.yaml
    --clang-format-config   Optional path to clang-format configuration file
                           If not provided, searches for .clang-format in repository root

Input structure (relative to input base dir):
    templates/properties/    # Property YAML files
    templates/schemas/       # Validation schema files
    templates/jinja/         # Jinja2 template files

Output structure (relative to output base dir):
    include/generated_code/  # Generated header files
    src/generated_code/      # Generated source files
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

import yaml
from jinja2 import Environment, FileSystemLoader
from jsonschema import validate, ValidationError

_SCRIPT_DIR = Path(os.path.dirname(os.path.abspath(__file__)))


class _DotDict(dict):
    """dict subclass that allows attribute-style access (nested)."""

    def __getattr__(self, name):
        try:
            return self[name]
        except KeyError:
            raise AttributeError(f"No attribute '{name}'") from None

    def __setattr__(self, name, value):
        self[name] = value


def _to_dot_dict(obj):
    """Recursively convert dicts to _DotDict so Jinja dot-notation works."""
    if isinstance(obj, dict):
        return _DotDict({k: _to_dot_dict(v) for k, v in obj.items()})
    if isinstance(obj, list):
        return [_to_dot_dict(item) for item in obj]
    return obj


def load_yaml(yaml_path: Path) -> dict:
    """Load and return the YAML file as a nested _DotDict structure."""
    with open(yaml_path, "r") as fh:
        data = yaml.safe_load(fh)
    if not isinstance(data, dict):
        raise ValueError(f"Expected a YAML mapping at the top level, got {type(data).__name__}")
    return _to_dot_dict(data)


def validate_data(data: dict, schema_path: Path) -> list[str]:
    """Validate *data* against the JSON Schema at *schema_path*.

    Returns a list of error messages (empty on success).
    """
    with open(schema_path, "r") as fh:
        schema = yaml.safe_load(fh)

    errors: list[str] = []
    try:
        validate(instance=dict(data), schema=schema)
        print(f" ✓ Schema validation passed: {schema_path.name}")
    except ValidationError as exc:
        errors.append(f"{schema_path.name}: {exc.message} (path: {list(exc.absolute_path)})")
    return errors


def load_input_data(input_base: Path, deployment_yaml: Path | None = None) -> dict:
    """Load and return the deployment.yaml data as the single source of truth."""
    deployment_file = deployment_yaml

    if not deployment_file.exists():
        raise FileNotFoundError(f"deployment.yaml not found: {deployment_file}")

    def _safe_len(value: object) -> int:
        return len(value) if value is not None else 0

    try:
        data = load_yaml(deployment_file)
        print(
            f"✓ Loaded deployment data: {_safe_len(getattr(data, 'services', []))} services, "
            f"{_safe_len(getattr(data, 'clusters', []))} clusters, "
            f"{_safe_len(getattr(data, 'connections', []))} connections, "
            f"{_safe_len(getattr(data, 'allocators', []))} allocators"
        )
        return data
    except Exception as e:
        raise RuntimeError(f"Error loading deployment.yaml: {e}")


def render_template_with_context(template_path: Path, context: dict, output_path: Path) -> None:
    """Render a single template with the given context and write to output path."""
    env = Environment(
        loader=FileSystemLoader(searchpath=str(template_path.parent)),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )

    # Set the context as globals
    for key, value in context.items():
        env.globals[key] = value

    try:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        template = env.get_template(template_path.name)
        template.stream().dump(str(output_path))
        print(f"  Generated: {output_path}")
    except Exception as e:
        raise RuntimeError(f"Error rendering template {template_path.name}: {e}")


# Template generation configuration
TEMPLATE_CONFIG = {
    "global": [
        ("allocator_selector_definitions.cpp.jinja", "AllocatorSelectorDefinitions.cpp", "src/generated_code"),
        ("cluster_id.h.jinja", "ClusterId.h", "include/generated_code/middleware"),
        ("shm/allocators_definitions.h.jinja", "AllocatorsDefinitions.h", "include/generated_code/shm"),
        ("shm/config.h.jinja", "Config.h", "include/generated_code/middleware/shm"),
        ("shm/config.cpp.jinja", "Config.cpp", "src/generated_code/shm"),
        ("shm/queue_definitions.h.jinja", "QueueDefinitions.h", "include/generated_code/shm"),
    ],
    "per_cluster": [
        ("cluster_srcCluster.h.jinja", "Cluster{}.h", "include/generated_code/middleware"),
        ("cluster_srcCluster.cpp.jinja", "Cluster{}.cpp", "src/generated_code"),
    ],
    "per_connection": [
        (
            "cluster_connections_srcCluster_to_tgtCluster.h.jinja",
            "ClusterConnections{}{}.h",
            "include/generated_code/middleware",
        ),
    ],
    "per_service": [
        ("service_common.h.jinja", "{}Common.h", "include/generated_code/{}"),
        ("service_proxy.h.jinja", "{}Proxy.h", "include/generated_code/{}"),
        ("service_skeleton.h.jinja", "{}Skeleton.h", "include/generated_code/{}"),
        ("service_common.cpp.jinja", "{}Common.cpp", "src/generated_code/{}"),
        ("service_proxy.cpp.jinja", "{}Proxy.cpp", "src/generated_code/{}"),
        ("service_skeleton.cpp.jinja", "{}Skeleton.cpp", "src/generated_code/{}"),
    ],
}


def build_context(input_data: dict, **additional) -> dict:
    """Build the standard template context with optional additional variables."""
    context = {
        "clusters": input_data.clusters,
        "services": input_data.services,
        "connections": input_data.connections,
        "cores": getattr(input_data, "cores", []),
        "allocators": getattr(input_data, "allocators", []),
    }
    context.update(additional)
    return context


def generate_files(
    template_type: str, input_data: dict, input_base: Path, output_base: Path, items=None, item_key: str = None
) -> None:
    """Unified file generation function for all template types."""
    print(f"Generating {template_type.replace('_', '-')} files...")

    template_base = input_base / "templates/jinja"
    mappings = TEMPLATE_CONFIG[template_type]

    # Determine what to iterate over
    if items is None:
        items = [None]  # Single iteration for global files

    for item in items:
        context = build_context(input_data)
        if item_key and item:
            context[item_key] = item
            if hasattr(item, "name"):
                print(f"  Processing {item_key}: {item.name}")

        for template_name, output_pattern, output_dir_pattern in mappings:
            template_path = template_base / template_name

            if not template_path.exists():
                print(f"  Skipping template (not found): {template_name}")
                continue

            # Generate output paths based on template type
            if template_type == "global":
                output_filename = output_pattern
                output_dir = output_base / output_dir_pattern
            elif template_type == "per_cluster":
                output_filename = output_pattern.format(item.name)
                output_dir = output_base / output_dir_pattern
            elif template_type == "per_connection":
                src_name = item.source_cluster.name
                tgt_name = item.target_cluster.name
                output_filename = output_pattern.format(src_name, tgt_name)
                output_dir = output_base / output_dir_pattern
            elif template_type == "per_service":
                service_name = getattr(item, "filename", item.name)
                namespace_path = item.namespace.replace("::", "/").lower() if hasattr(item, "namespace") else ""
                output_filename = output_pattern.format(service_name)
                output_dir = output_base / output_dir_pattern.format(namespace_path)

            output_path = output_dir / output_filename
            render_template_with_context(template_path, context, output_path)


# These functions are now replaced by the unified generate_files() function above


def clean_previous_generated_files(output_base: Path) -> None:
    """Delete previously generated output directories before a new generation run."""
    generated_dirs = [
        output_base / "include/generated_code",
        output_base / "src/generated_code",
    ]

    print("Cleaning previously generated files...")
    for generated_dir in generated_dirs:
        if generated_dir.exists():
            shutil.rmtree(generated_dir)
            print(f"  Removed: {generated_dir}")


def format_cpp_files(output_base: Path, clang_format_config: Path = None) -> None:
    """Run clang-format on all generated C++ files (.h, .hpp, .cpp)."""
    cpp_extensions = {".h", ".hpp", ".cpp"}
    cpp_files = []

    # Find all C++ files in the generated directories
    for root, dirs, files in os.walk(output_base):
        for file in files:
            if Path(file).suffix in cpp_extensions:
                cpp_files.append(Path(root) / file)

    if not cpp_files:
        print("No C++ files found to format")
        return

    print(f"\nFormatting {len(cpp_files)} C++ files with clang-format...")

    # Run clang-format in-place on all found files
    try:
        # Use provided config or search for .clang-format in repo root
        if clang_format_config and clang_format_config.exists():
            style_arg = f"--style=file:{clang_format_config}"
        else:
            # Find the repository root (where .clang-format should be located)
            repo_root = Path(__file__).resolve()
            while repo_root.parent != repo_root and not (repo_root / ".clang-format").exists():
                repo_root = repo_root.parent

            clang_format_path = repo_root / ".clang-format"
            if clang_format_path.exists():
                style_arg = f"--style=file:{clang_format_path}"
            else:
                style_arg = "--style=file"  # fallback to default search

        cmd = ["clang-format", "-i", style_arg] + [str(f) for f in cpp_files]
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("✓ Successfully formatted all C++ files")
    except subprocess.CalledProcessError as e:
        print(f"Warning: clang-format failed: {e}", file=sys.stderr)
        if e.stderr:
            print(f"  Error output: {e.stderr.strip()}", file=sys.stderr)
    except FileNotFoundError:
        print("Warning: clang-format not found in PATH, skipping formatting", file=sys.stderr)
        print("  Install with: sudo apt install clang-format", file=sys.stderr)
        print("  Or ensure clang-format is available in your PATH", file=sys.stderr)


def parse_args(argv=None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate C++ files from YAML properties using predefined template mappings.",
    )
    parser.add_argument(
        "--input",
        type=Path,
        required=True,
        help="Base input directory containing templates/ subdirectory with properties/, schemas/, and jinja/ folders.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Base output directory where include/generated_code/ and src/generated_code/ will be created.",
    )
    parser.add_argument(
        "--deployment-yaml",
        type=Path,
        required=True,
        help="Deployment YAML file",
    )
    parser.add_argument(
        "--clang-format-config",
        type=Path,
        help="Path to clang-format configuration file. If not provided, searches for .clang-format in repository root.",
    )
    parser.add_argument(
        "--no-clean",
        action="store_true",
        help="Preserve existing generated files and skip deleting include/generated_code and src/generated_code before generation.",
    )
    return parser.parse_args(argv)


def main(argv=None) -> int:
    args = parse_args(argv)

    input_base = args.input.resolve()
    output_base = args.output.resolve()

    if not input_base.is_dir():
        print(f"error: Input directory not found: {input_base}", file=sys.stderr)
        return 1

    print(f"Input base directory: {input_base}")
    print(f"Output base directory: {output_base}")
    print()

    # Load deployment.yaml as the single source of truth
    try:
        deployment_yaml = args.deployment_yaml.resolve()
        input_data = load_input_data(input_base, deployment_yaml)
    except Exception as e:
        print(f"error: {e}", file=sys.stderr)
        return 1

    # Validate the input data against schema
    schema_path = input_base / "templates/schemas/deployment.yaml"
    if schema_path.exists():
        validation_errors = validate_data(input_data, schema_path)
        if validation_errors:
            print("\nValidation errors:", file=sys.stderr)
            for error in validation_errors:
                print(f"  - {error}", file=sys.stderr)
            return 1
    else:
        print(f"Warning: Schema not found at {schema_path}, skipping validation")

    # Generate all file types using the unified clusters.yaml data
    try:
        if args.no_clean:
            print("Skipping cleanup of previously generated files (--no-clean).")
        else:
            clean_previous_generated_files(output_base)

        generate_files("global", input_data, input_base, output_base)
        generate_files(
            "per_cluster", input_data, input_base, output_base, items=input_data.clusters, item_key="cluster"
        )
        generate_files(
            "per_connection", input_data, input_base, output_base, items=input_data.connections, item_key="connection"
        )
        generate_files(
            "per_service", input_data, input_base, output_base, items=input_data.services, item_key="service"
        )

        # Format all generated C++ files with clang-format
        format_cpp_files(output_base, args.clang_format_config)

        print("\nAll file generation completed successfully!")
        return 0

    except Exception as e:
        print(f"\nError during generation: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
