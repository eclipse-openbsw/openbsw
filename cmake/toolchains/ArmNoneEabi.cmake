# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

#
# Single entry-point ARM none-eabi toolchain file. It automatically selects the
# GCC- or Clang-specific toolchain settings based on the compiler in use, so
# presets no longer need to encode "-gcc"/"-clang" in their name or point at a
# different toolchain file. The compiler is chosen the usual CMake way (the
# CC/CXX environment variables or -DCMAKE_C_COMPILER=/-DCMAKE_CXX_COMPILER=...);
# this file inspects that choice to decide which set of compiler-specific flags
# to add, and also sets CMAKE_C_COMPILER/CMAKE_CXX_COMPILER explicitly so that
# both languages consistently resolve to the same toolchain (GCC defaults to
# "arm-none-eabi-gcc"/"arm-none-eabi-g++" if nothing is set).

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

# NOTE: we only check the C compiler, since the C++ compiler is derived from it
# below (respecting the order in the project call). If neither is set, GCC is
# used as the default.

if (DEFINED CMAKE_C_COMPILER)
    set(_ARM_C_COMPILER "${CMAKE_C_COMPILER}")
elseif (DEFINED ENV{CC})
    set(_ARM_C_COMPILER "$ENV{CC}")
else ()
    set(_ARM_C_COMPILER "arm-none-eabi-gcc")
endif ()

if (_ARM_C_COMPILER MATCHES "clang")
    set(_ARM_CXX_COMPILER_DEFAULT "arm-none-eabi-clang++")
    include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-clang.cmake")
else ()
    set(_ARM_CXX_COMPILER_DEFAULT "arm-none-eabi-g++")
    include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-gcc.cmake")
endif ()

set(CMAKE_C_COMPILER "${_ARM_C_COMPILER}")

if (DEFINED CMAKE_CXX_COMPILER)
    # already set explicitly, leave as-is
elseif (DEFINED ENV{CXX})
    set(CMAKE_CXX_COMPILER "$ENV{CXX}")
else ()
    set(CMAKE_CXX_COMPILER "${_ARM_CXX_COMPILER_DEFAULT}")
endif ()

unset(_ARM_CXX_COMPILER_DEFAULT)
unset(_ARM_C_COMPILER)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-common.cmake")
