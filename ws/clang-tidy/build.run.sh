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

: "${PRESET:?PRESET must name the CMake preset}"
: "${BUILD_DIRECTORY:?BUILD_DIRECTORY must locate compile_commands.json}"

export CC=clang-17
export CXX=clang++-17
cd source

printf '\nConfiguring and building %s...\n\n' "${PRESET}"
cmake --preset "${PRESET}" -DCMAKE_CXX_STANDARD=17
cmake --build --preset "${PRESET}" --config Release --verbose -j 5

cp -a "${BUILD_DIRECTORY}/." /out
