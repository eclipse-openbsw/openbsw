#!/usr/bin/env bash
# *******************************************************************************
# Copyright (c) 2026 BMW AG
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************
#
# Wrapper script to regenerate blob headers for the routing test.
#
# This script locates the project root and the tools/blob/regenerate.sh script,
# then calls it with the correct paths for the routing test configuration.
#
# Usage:
#   regenerate.sh [PYTHON]
#
#   PYTHON  (optional) Python interpreter to use. If not provided, defaults to
#           the PYTHON environment variable or python3.
#
# Examples:
#   # From libs/bsw/routing/test/
#   ./regenerate.sh
#
#   # From project root
#   libs/bsw/routing/test/regenerate.sh
#
#   # With a custom Python interpreter
#   PYTHON=/custom/python3 ./regenerate.sh

set -euo pipefail

# Find the project root by locating tools/blob/regenerate.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}"
while [[ "${PROJECT_ROOT}" != "/" ]]; do
    if [[ -f "${PROJECT_ROOT}/tools/blob/regenerate.sh" ]]; then
        break
    fi
    PROJECT_ROOT="$(cd "${PROJECT_ROOT}/.." && pwd)"
done

if [[ ! -f "${PROJECT_ROOT}/tools/blob/regenerate.sh" ]]; then
    echo "ERROR: Could not locate tools/blob/regenerate.sh. Are you in an openbsw project?" >&2
    exit 1
fi

# Set paths relative to the project root
JSONL="${PROJECT_ROOT}/libs/bsw/routing/test/routing.jsonl"
OUT_BLOB="${PROJECT_ROOT}/libs/bsw/routing/test/include/blob"
OUT_ROUTING="${PROJECT_ROOT}/libs/bsw/routing/test/include/routing"

# Pass through PYTHON environment variable if set
export PYTHON="${PYTHON:-${1:-python3}}"

# Call the main regenerate script
exec "${PROJECT_ROOT}/tools/blob/regenerate.sh" "${JSONL}" "${OUT_BLOB}" "${OUT_ROUTING}"
