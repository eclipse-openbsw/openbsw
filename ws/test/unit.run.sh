# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set -e

PRESET=tests-posix-debug

echo "\nBuilding...\n"
cmake --preset $PRESET
cmake --build --preset $PRESET -j 5
# Coverage-instrumented test binaries can exit non-zero when stale .gcda files
# from a previous build are reused after object checksum changes.
find build/tests/posix/Debug -name '*.gcda' -delete
ctest --preset $PRESET --output-on-failure
