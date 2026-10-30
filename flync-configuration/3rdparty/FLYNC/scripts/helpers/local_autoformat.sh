#!/bin/bash
# Autoformats src and tests using isort and black (the fixable checkers).
# mypy and flake8 report issues that require manual fixes and are skipped.
# Usage: bash local_autoformat.sh

LINE_LENGTH=149

echo "=== Formatting isort ==="
uv run isort --line-length $LINE_LENGTH src
uv run isort --line-length $LINE_LENGTH tests

echo "=== Formatting black ==="
uv run black --target-version py312 src
uv run black --target-version py312 tests

echo ""
echo "Done. Run local_checkers.sh to verify. Note: flake8 and mypy issues require manual fixes."
