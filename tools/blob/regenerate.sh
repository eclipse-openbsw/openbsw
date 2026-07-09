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
#                    If crc package is not available, a venv will be
#                    automatically created at ~/.cache/openbsw/blob-venv
#
# Prerequisites:
#   - python3 and python3-venv (or equivalent for your distro)
#   - clang-format (optional, for automatic formatting)
#
# The script will automatically:
#   1. Check if the crc package is available
#   2. If not, create a venv in ~/.cache/openbsw/blob-venv
#   3. Install dependencies from tools/blob/requirements.txt
#   4. Run the blob generator
#   5. Format the output with clang-format (if available)
#
# Example (referenceApp, run from the project root):
#   tools/blob/regenerate.sh \
#       executables/referenceApp/configuration/routing.jsonl \
#       executables/referenceApp/configuration/include/blob \
#       executables/referenceApp/configuration/include/routing
#
# Or with a custom Python interpreter:
#   PYTHON=/path/to/python3 tools/blob/regenerate.sh ...

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
REQUIREMENTS="${TOOLS_DIR}/blob/requirements.txt"

# Set up venv automatically if crc is not available
if ! "${PYTHON}" -c "import crc" >/dev/null 2>&1; then
    VENV_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/openbsw/blob-venv"
    if [[ ! -x "${VENV_DIR}/bin/python" ]]; then
        echo "Setting up Python environment in ${VENV_DIR} ..."
        
        # Try to create venv, capture stderr for error detection
        VENV_ERROR=$("${PYTHON}" -m venv "${VENV_DIR}" 2>&1 || echo "FAILED")
        if echo "${VENV_ERROR}" | grep -q "ensurepip"; then
            # Venv creation failed due to missing ensurepip; try to install python3-venv
            PYTHON_VERSION=$("${PYTHON}" -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
            VENV_PKG="python${PYTHON_VERSION}-venv"
            echo "Installing ${VENV_PKG} ..."
            if sudo apt-get install -y "${VENV_PKG}"; then
                # Clean up the partial venv and retry
                echo "Retrying venv creation..."
                rm -rf "${VENV_DIR}"
                "${PYTHON}" -m venv "${VENV_DIR}" || {
                    echo "ERROR: Failed to create venv after installing ${VENV_PKG}." >&2
                    exit 1
                }
            else
                echo "ERROR: Failed to install ${VENV_PKG}. Please install it manually and try again." >&2
                exit 1
            fi
        elif echo "${VENV_ERROR}" | grep -q "FAILED"; then
            echo "ERROR: Failed to create venv: ${VENV_ERROR}" >&2
            exit 1
        fi
    fi
    PYTHON="${VENV_DIR}/bin/python"
    "${PYTHON}" -m pip install --quiet --disable-pip-version-check -r "${REQUIREMENTS}"
fi
export PYTHONPATH="${TOOLS_DIR}${PYTHONPATH:+:$PYTHONPATH}"

# ---------------------------------------------------------------------------
# Preflight checks
# ---------------------------------------------------------------------------
if [[ ! -f "${JSONL}" ]]; then
    echo "ERROR: JSONL_FILE not found: ${JSONL}" >&2
    exit 1
fi

mkdir -p "${OUT_BLOB}" "${OUT_ROUTING}"

echo "Using Python:      $("${PYTHON}" --version)"
echo "Input:             ${JSONL}"
echo "Output blob/:      ${OUT_BLOB}"
echo "Output routing/:   ${OUT_ROUTING}"

# ---------------------------------------------------------------------------
# Generation — import blob module via PYTHONPATH
# ---------------------------------------------------------------------------

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
