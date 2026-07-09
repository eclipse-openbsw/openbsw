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
# Regenerate the C++ headers produced by the blob generator tool.
#
# This script is designed to be reusable: it lives in tools/blob and can be
# called from any project that vendors or includes this repository.
#
# Usage:
#   regenerate.sh JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR
#
#   JSONL_FILE       Path to the routing .jsonl input file.
#   OUT_BLOB_DIR     Directory to write blob headers:
#                      configuration.h  (binary blob as uint8_t array)
#                      ConfigType.h     (ConfigType enum)
#   OUT_ROUTING_DIR  Directory to write routing headers:
#                      channelId.h      (per-channel ID constants)
#
# Environment variables:
#   PYTHON           Python interpreter to use (default: python3).
#                    Must have the 'crc' package installed (see requirements.txt).
#
# Prerequisites (one of):
#   a) Run inside the openbsw-development Docker container — all dependencies
#      are already available via /opt/venv.
#   b) Create a venv on the host (once):
#        python3 -m venv tools/.venv
#        tools/.venv/bin/pip install -r tools/blob/requirements.txt
#      Then pass the interpreter:
#        PYTHON=tools/.venv/bin/python3 tools/blob/regenerate.sh ...
#
# Example (referenceApp, run from the project root):
#   tools/blob/regenerate.sh \
#       executables/referenceApp/configuration/routing.jsonl \
#       executables/referenceApp/configuration/include/blob \
#       executables/referenceApp/configuration/include/routing

set -euo pipefail

# ---------------------------------------------------------------------------
# Argument parsing
# ---------------------------------------------------------------------------
if [[ $# -ne 3 ]]; then
    echo "Usage: $(basename "$0") JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR" >&2
    echo "See the file header for details." >&2
    exit 1
fi

JSONL="$(realpath "$1")"
OUT_BLOB="$(realpath "$2")"
OUT_ROUTING="$(realpath "$3")"

# The blob Python package must be imported from the tools directory.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

PYTHON="${PYTHON:-python3}"
# Resolve to absolute path now, before we cd into TOOLS_DIR later.
# Bare names (no slash) are looked up in PATH; relative paths are made absolute
# without following symlinks (following symlinks through a venv breaks site-packages).
if [[ "${PYTHON}" != */* ]]; then
    PYTHON="$(command -v "${PYTHON}")"
elif [[ "${PYTHON}" != /* ]]; then
    PYTHON="$(cd "$(dirname "${PYTHON}")" && pwd)/$(basename "${PYTHON}")"
fi

# ---------------------------------------------------------------------------
# Preflight checks
# ---------------------------------------------------------------------------
if [[ ! -f "${JSONL}" ]]; then
    echo "ERROR: JSONL_FILE not found: ${JSONL}" >&2
    exit 1
fi

if ! "${PYTHON}" -c "import crc" 2>/dev/null; then
    echo "ERROR: 'crc' package not found for '${PYTHON}'." >&2
    echo "  Option A: run inside the openbsw-development Docker container." >&2
    echo "  Option B: set up a venv:" >&2
    echo "    python3 -m venv tools/.venv" >&2
    echo "    tools/.venv/bin/pip install -r tools/blob/requirements.txt" >&2
    echo "    PYTHON=tools/.venv/bin/python3 $0 ..." >&2
    exit 1
fi

mkdir -p "${OUT_BLOB}" "${OUT_ROUTING}"

echo "Using Python:      $("${PYTHON}" --version)"
echo "Input:             ${JSONL}"
echo "Output blob/:      ${OUT_BLOB}"
echo "Output routing/:   ${OUT_ROUTING}"

# ---------------------------------------------------------------------------
# Generation — must run from TOOLS_DIR so 'import blob' resolves correctly
# ---------------------------------------------------------------------------
cd "${TOOLS_DIR}"

# include/blob/configuration.h — binary blob embedded as a uint8_t array
"${PYTHON}" -m blob binary \
    -i "${JSONL}" \
    -c blob.routing.table \
    | "${PYTHON}" -m blob header data \
        -n CONFIGURATION_BLOB \
        -o "${OUT_BLOB}/configuration.h"
echo "Written: ${OUT_BLOB}/configuration.h"

# include/blob/ConfigType.h — ConfigType enum
"${PYTHON}" -m blob header config-type \
    -o "${OUT_BLOB}/ConfigType.h"
echo "Written: ${OUT_BLOB}/ConfigType.h"

# include/routing/channelId.h — per-channel ID constants
"${PYTHON}" -m blob.routing header \
    "${JSONL}" \
    -o "${OUT_ROUTING}/channelId.h"
echo "Written: ${OUT_ROUTING}/channelId.h"

# ---------------------------------------------------------------------------
# Format the generated headers with clang-format
# ---------------------------------------------------------------------------
if command -v clang-format &> /dev/null; then
    clang-format -i "${OUT_BLOB}/configuration.h" \
                     "${OUT_BLOB}/ConfigType.h" \
                     "${OUT_ROUTING}/channelId.h"
    echo "Formatted with clang-format."
fi

echo "Done. Review ${OUT_ROUTING}/constants.h if channel IDs changed."
