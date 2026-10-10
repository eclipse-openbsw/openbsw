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
# own. It only contributes the Clang specific flags; the common settings are
# applied by ArmNoneEabi-common.cmake.

set(CMAKE_C_COMPILER_TARGET ${ARM_TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${ARM_TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${ARM_TARGET_TRIPLE})

set(_EXE_LINKER_FLAGS
    "-Wl,--start-group -ldummyhost -lclang_rt.builtins -Wl,--end-group")

# Clang defaults to -Os instead of GCC's -O2 for Release/RelWithDebInfo builds.
# These override the generic defaults from the base preset (which are tuned for
# GCC).
set(CMAKE_C_FLAGS_RELEASE
    "-Os -DNDEBUG"
    CACHE STRING "C Release flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE
    "-Os -DNDEBUG"
    CACHE STRING "C++ Release flags" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO
    "-g3 -Os -DNDEBUG"
    CACHE STRING "C RelWithDebInfo flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO
    "-g3 -Os -DNDEBUG"
    CACHE STRING "C++ RelWithDebInfo flags" FORCE)
