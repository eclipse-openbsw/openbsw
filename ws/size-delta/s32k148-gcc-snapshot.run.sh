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

export CC=/opt/arm-gnu-toolchain/bin/arm-none-eabi-gcc
cd source

printf '\nBuilding size snapshot...\n\n'
cmake --preset s32k148-freertos-gcc
cmake --build --preset s32k148-freertos-gcc -j 5

python3 ../analyze.py snapshot \
    build/s32k148-freertos-gcc/executables/referenceApp/application/RelWithDebInfo/app.referenceApp.elf \
    | tee /out/snapshot.txt
