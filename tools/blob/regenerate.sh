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
# Regenerate blob and routing C++ headers from a routing JSON Lines file.
# This script is build-system independent and can be used by CMake or other
# build systems that consume checked-in generated headers.
#
# Usage:
#   regenerate.sh JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR
#
# The blob output directory receives configuration.h and ConfigType.h. The
# routing output directory receives channelId.h and constants.h. Directories
# are created if needed; existing generated files are overwritten.
#
# Requirements:
#   - Python 3 and venv support
#   - Dependencies are installed from requirements.lock into
#     ${XDG_CACHE_HOME:-$HOME/.cache}/openbsw/blob-venv when crc is unavailable.
#
# Set PYTHON to select a Python 3 interpreter. For project-specific inputs and
# output locations, see the adjacent regenerate.sh wrappers.

set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "Usage: $(basename "$0") JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR" >&2
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PROJECT_ROOT="$(cd "${TOOLS_DIR}/.." && pwd)"
PYTHON="${PYTHON:-python3}"

if ! "${PYTHON}" -c "import crc" >/dev/null 2>&1; then
    VENV_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/openbsw/blob-venv"
    if [[ ! -x "${VENV_DIR}/bin/python" ]]; then
        "${PYTHON}" -m venv "${VENV_DIR}"
    fi
    PYTHON="${VENV_DIR}/bin/python"
    "${PYTHON}" -m pip install --quiet --disable-pip-version-check \
        --require-hashes -r "${SCRIPT_DIR}/requirements.lock"
fi

export PYTHONPATH="${TOOLS_DIR}${PYTHONPATH:+:$PYTHONPATH}"

"${PYTHON}" "${SCRIPT_DIR}/generate_routing.py" "$@"

if [[ -f "${PROJECT_ROOT}/.ci/format.py" ]]; then
    OUT_BLOB="$(realpath -m "$2")"
    OUT_ROUTING="$(realpath -m "$3")"
    FORMAT_PATHS=()

    for header in \
        "${OUT_BLOB}/configuration.h" \
        "${OUT_BLOB}/ConfigType.h" \
        "${OUT_ROUTING}/channelId.h" \
        "${OUT_ROUTING}/constants.h"; do
        if [[ "${header}" == "${PROJECT_ROOT}/"* ]]; then
            FORMAT_PATHS+=("${header#"${PROJECT_ROOT}/"}")
        fi
    done

    if ((${#FORMAT_PATHS[@]} > 0)); then
        if command -v docker >/dev/null 2>&1 && docker compose version >/dev/null 2>&1; then
            (
                cd "${PROJECT_ROOT}"
                DOCKER_UID="$(id -u)" DOCKER_GID="$(id -g)" DOCKER_HISTORY=/dev/null \
                    docker compose run --rm development treefmt --no-cache "${FORMAT_PATHS[@]}"
            )
        elif command -v treefmt >/dev/null 2>&1; then
            (
                cd "${PROJECT_ROOT}"
                treefmt --no-cache "${FORMAT_PATHS[@]}"
            )
        fi
    fi
fi