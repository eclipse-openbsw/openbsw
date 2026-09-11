# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set -euo pipefail

: "${BUILD_DIRECTORY:?BUILD_DIRECTORY must locate compile_commands.json}"

source_root=/worktree/source
build_directory="${source_root}/${BUILD_DIRECTORY}"

mkdir -p "$(dirname "${build_directory}")"
ln -s /clang-tidy-build "${build_directory}"

# The compilation database may repeat translation units; emit each repository source once.
# Exclude third-party, test, and mock sources while retaining their headers in the source tree.
jq --join-output 'map(.file) | unique[] | ., "\u0000"' "${build_directory}/compile_commands.json" \
    | while IFS= read -r -d '' source; do
        if [[ "${source}" != "${source_root}/"* ]]; then
            continue
        fi
        relative_path="${source#${source_root}/}"
        case "/${relative_path}" in
            */3rdparty/* | */test/* | */mock/*) continue ;;
        esac
        printf '%s\0' "${source}"
    done \
    | xargs --null --no-run-if-empty --max-args=1 --max-procs="$(nproc)" \
        sh -c 'clang-tidy-17 --quiet --warnings-as-errors="*" -p "$0" "$1"' "${build_directory}"
