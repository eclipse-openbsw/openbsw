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
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

require_file() {
    if [[ ! -f "$1" ]]; then
        echo "Required file is missing: $1" >&2
        exit 1
    fi
}

check_configuration() {
    local name="$1"
    local jsonl="$2"
    local checked_blob="$3"
    local checked_routing="$4"
    local generated_blob="${TMP_DIR}/${name}/blob"
    local generated_routing="${TMP_DIR}/${name}/routing"
    local stale=0

    require_file "${PROJECT_ROOT}/${jsonl}"
    require_file "${checked_blob}/ConfigType.h"
    require_file "${checked_blob}/configuration.h"
    if [[ -n "${checked_routing}" ]]; then
        require_file "${checked_routing}/channelId.h"
        require_file "${checked_routing}/constants.h"
    fi

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

# Separated function as the Bazel generated output example is only on blobTest.
# One comparison between generated headers is enough for verifying the correctness of the Bazel output.
check_bazel_output() {
    local generated_blob="${TMP_DIR}/blobTest/blob"
    local generated_routing="${TMP_DIR}/blobTest/routing"
    local bazel_output="${PROJECT_ROOT}/bazel-bin/libs/bsw/blob/generated_routing_headers/generated"
    local include_directories

    bazel build --platforms=//bazel/platform:posix_unit_test \
        //libs/bsw/blob:generated_routing_headers

    include_directories="$(bazel cquery --platforms=//bazel/platform:posix_unit_test \
        --output=starlark \
        '--starlark:expr=providers(target)["CcInfo"].compilation_context.includes.to_list()' \
        //libs/bsw/blob:generated_routing_headers)"
    if ! grep -q 'libs/bsw/blob/generated_routing_headers/generated' <<<"${include_directories}"; then
        echo "Generated routing target does not export its include directory through CcInfo" >&2
        return 1
    fi

    for header in ConfigType.h configuration.h; do
        if ! cmp -s "${bazel_output}/blob/${header}" "${generated_blob}/${header}"; then
            echo "Bazel output differs from standalone generation: blob/${header}" >&2
            return 1
        fi
    done

    for header in channelId.h constants.h; do
        if ! cmp -s "${bazel_output}/routing/${header}" "${generated_routing}/${header}"; then
            echo "Bazel output differs from standalone generation: routing/${header}" >&2
            return 1
        fi
    done
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
check_bazel_output
check_configuration \
    routingTest \
    libs/bsw/routing/test/routing.jsonl \
    "${PROJECT_ROOT}/libs/bsw/routing/test/include/blob" \
    "${PROJECT_ROOT}/libs/bsw/routing/test/include/routing"
