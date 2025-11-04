include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

# Note: When using a toolchain file, it normally specifies the compiler itself
# and it is unexpected that somebody tries to overwrite it using e.g. the CC
# environment variable. However, thats exactly what we are doing at the moment,
# therefore, we only set a compiler when the CC environment variable is not set.
# Further note, that we only set the C compiler, since CMake deduces the others
# automatically from it (respecting the order in the project call).

if (NOT DEFINED CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
    set(CMAKE_C_COMPILER "/usr/bin/llvm-arm/bin/clang")
endif ()

set(CMAKE_C_COMPILER_TARGET ${ARM_TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${ARM_TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${ARM_TARGET_TRIPLE})

set(_EXE_LINKER_FLAGS
    "-Wl,--start-group -ldummyhost -lclang_rt.builtins -Wl,--end-group")

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")
