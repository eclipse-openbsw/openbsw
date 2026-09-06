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

GCC_VERSION=11
BUILD_DIRECTORY=code_coverage

mkdir -p "${BUILD_DIRECTORY}"

copy_tracefile() {
    local input="$1"
    local name="$2"
    local output="${BUILD_DIRECTORY}/coverage_${name}.info"

    sed 's#^SF:/worktree/source/#SF:/worktree/#' \
        "/${input}/coverage_${name}.info" >"${output}"
}

copy_tracefile s32k1xx s32k1xx
copy_tracefile posix posix

lcov \
    --ignore-errors mismatch \
    --add-tracefile "${BUILD_DIRECTORY}/coverage_s32k1xx.info" \
    --add-tracefile "${BUILD_DIRECTORY}/coverage_posix.info" \
    --output-file "${BUILD_DIRECTORY}/coverage.info"

genhtml \
    "${BUILD_DIRECTORY}/coverage.info" \
    --prefix "${PWD}" \
    --output-directory "${BUILD_DIRECTORY}/coverage"

summary="$(
    lcov \
        --gcov-tool "gcov-${GCC_VERSION}" \
        --summary "${BUILD_DIRECTORY}/coverage.info" \
        --ignore-errors mismatch
)"

badge_directory="${BUILD_DIRECTORY}/coverage_badges"
mkdir -p "${badge_directory}"

generate_badge() {
    local metric="$1"
    local title="$2"
    local output="$3"
    local pattern="${metric}\\.*:[[:space:]]+([0-9]+\\.[0-9]+)%"

    if [[ "${summary}" =~ ${pattern} ]]; then
        local percentage="${BASH_REMATCH[1]}"
        printf '%s Percentage: %s%%\n' "${title}" "${percentage}"
        wget \
            "https://img.shields.io/badge/coverage-${percentage}%25-brightgreen.svg" \
            -O "${badge_directory}/${output}"
    fi
}

generate_badge lines Line line_coverage_badge.svg
generate_badge functions Function function_coverage_badge.svg

cp -a . /out
rm /out/run.sh
