# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

# Private fragment of ArmNoneEabi.cmake, not usable as a toolchain file on its
# own. It only contributes the GCC specific flags; the common settings are
# applied by ArmNoneEabi-common.cmake.

set(_C_FLAGS "-funsigned-bitfields")

set(_CXX_FLAGS "-femit-class-debug-always -funsigned-bitfields")

set(_EXE_LINKER_FLAGS "-specs=nano.specs -specs=nosys.specs")
