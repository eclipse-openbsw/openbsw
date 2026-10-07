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

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TMP_DIR="$(mktemp -d "${PROJECT_ROOT}/.routing-generation-check.XXXXXX")"
trap 'rm -rf "${TMP_DIR}"' EXIT

check_configuration() {
    local name="$1"
    local jsonl="$2"
    local checked_blob="$3"
    local checked_routing="$4"
    local generated_blob="${TMP_DIR}/${name}/blob"
    local generated_routing="${TMP_DIR}/${name}/routing"
    local stale=0

    "${SCRIPT_DIR}/regenerate.sh" \
        "${PROJECT_ROOT}/${jsonl}" \
        "${generated_blob}" \
        "${generated_routing}"

    for header in ConfigType.h configuration.h; do
        if ! cmp -s "${checked_blob}/${header}" "${generated_blob}/${header}"; then
            echo "Generated header is stale: ${checked_blob}/${header}" >&2
            stale=1
        fi
    done

    if [[ -n "${checked_routing}" ]]; then
        for header in channelId.h constants.h; do
            if ! cmp -s "${checked_routing}/${header}" "${generated_routing}/${header}"; then
                echo "Generated header is stale: ${checked_routing}/${header}" >&2
                stale=1
            fi
        done
    fi

    return "${stale}"
}

check_configuration \
    referenceApp \
    executables/referenceApp/configuration/routing.jsonl \
    "${PROJECT_ROOT}/executables/referenceApp/configuration/include/blob" \
    "${PROJECT_ROOT}/executables/referenceApp/configuration/include/routing"
check_configuration \
    blobTest \
    libs/bsw/blob/test/routing.jsonl \
    "${PROJECT_ROOT}/libs/bsw/blob/test/include/blob" \
    ""
check_configuration \
    routingTest \
    libs/bsw/routing/test/routing.jsonl \
    "${PROJECT_ROOT}/libs/bsw/routing/test/include/blob" \
    "${PROJECT_ROOT}/libs/bsw/routing/test/include/routing"