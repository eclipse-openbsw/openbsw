# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

#set -e

CMAKE_PRESET=posix-freertos

echo "\nBuilding...\n"
cmake --preset $CMAKE_PRESET
cmake --build --preset $CMAKE_PRESET -j 5 # -- -cache

#cd test/pyTest && pytest -s -v --target=posix --app=freertos

cram --shell=/bin/bash -ivy test/*.t

cp --parents test/*.t /out


