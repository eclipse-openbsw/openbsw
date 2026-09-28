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
# Usage:
#   regenerate.sh JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR
#
#   JSONL_FILE       Path to the routing .jsonl input file.
#   OUT_BLOB_DIR     Directory to write blob headers:
#                      configuration.h  (binary blob as uint8_t array)
#                      ConfigType.h     (ConfigType enum)
#   OUT_ROUTING_DIR  Directory to write routing headers:
#                      channelId.h      (per-channel ID constants)
#                      constants.h      (channel count constants)
#
# Prerequisites:
#   - Python 3 with the dependencies in tools/blob/requirements.txt installed

set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "Usage: $(basename "$0") JSONL_FILE OUT_BLOB_DIR OUT_ROUTING_DIR" >&2
    exit 1
fi

mkdir -p "$2" "$3"
JSONL="$(realpath "$1")"
OUT_BLOB="$(realpath "$2")"
OUT_ROUTING="$(realpath "$3")"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PYTHON="${PYTHON:-python3}"
export PYTHONPATH="${TOOLS_DIR}${PYTHONPATH:+:$PYTHONPATH}"

if [[ ! -f "${JSONL}" ]]; then
    echo "ERROR: JSONL_FILE not found: ${JSONL}" >&2
    exit 1
fi

echo "Input:             ${JSONL}"
echo "Output blob/:      ${OUT_BLOB}"
echo "Output routing/:   ${OUT_ROUTING}"

exec "${PYTHON}" -m blob.generate_routing "${JSONL}" "${OUT_BLOB}" "${OUT_ROUTING}"
