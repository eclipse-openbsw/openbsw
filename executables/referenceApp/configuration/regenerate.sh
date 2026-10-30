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
# Regenerate the reference application's checked-in blob and routing headers.
#
# Usage, from any working directory:
#   executables/referenceApp/configuration/regenerate.sh [PYTHON]
#
# PYTHON may also be set in the environment. The shared script provisions its
# locked Python dependency when needed; see tools/blob/regenerate.sh.

set -euo pipefail

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

export PYTHON="${PYTHON:-${1:-python3}}"

exec "${PROJECT_ROOT}/tools/blob/regenerate.sh" \
    "${PROJECT_ROOT}/executables/referenceApp/configuration/routing.jsonl" \
    "${PROJECT_ROOT}/executables/referenceApp/configuration/include/blob" \
    "${PROJECT_ROOT}/executables/referenceApp/configuration/include/routing"
