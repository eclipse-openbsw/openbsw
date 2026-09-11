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

: "${COVERAGE_NAME:?COVERAGE_NAME must name the coverage input}"
: "${PRESET:?PRESET must name the CMake preset}"

GCC_VERSION=11
BUILD_DIRECTORY=build

require_tool() {
    local tool="$1"
    if ! command -v "${tool}" >/dev/null; then
        printf 'ERROR: Required command %s not found!\n' "${tool}" >&2
        exit 1
    fi
}

require_tool "gcc-${GCC_VERSION}"
require_tool "g++-${GCC_VERSION}"

export CC
CC="$(command -v "gcc-${GCC_VERSION}")"
export CXX
CXX="$(command -v "g++-${GCC_VERSION}")"

threads="$(nproc)"
if ((threads > 1)); then
    ((threads -= 1))
fi
export CMAKE_BUILD_PARALLEL_LEVEL="${threads}"
export CTEST_PARALLEL_LEVEL="${threads}"

cd source
cmake --preset "${PRESET}" -B "${BUILD_DIRECTORY}"
cmake --build "${BUILD_DIRECTORY}" --config Debug
ctest --test-dir "${BUILD_DIRECTORY}" --output-on-failure

unfiltered="coverage_${COVERAGE_NAME}_unfiltered.info"
filtered="coverage_${COVERAGE_NAME}.info"

lcov \
    --gcov-tool "gcov-${GCC_VERSION}" \
    --capture \
    --directory "${BUILD_DIRECTORY}" \
    --no-external \
    --base-directory . \
    --output-file "${unfiltered}" \
    --ignore-errors mismatch \
    --rc geninfo_unexecuted_blocks=1

lcov \
    --remove "${unfiltered}" \
    '*/mock/*' \
    '*/gmock/*' \
    '*/gtest/*' \
    '*/test/*' \
    '*/3rdparty/*' \
    --output-file "${filtered}" \
    --ignore-errors mismatch

cp "${filtered}" "/out/${filtered}"
