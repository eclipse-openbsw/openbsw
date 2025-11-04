include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

# Note: When using a toolchain file, it normally specifies the compiler itself
# and it is unexpected that somebody tries to overwrite it using e.g. the CC
# environment variable. However, thats exactly what we are doing at the moment,
# therefore, we only set a compiler when the CC environment variable is not set.
# Further note, that we only set the C compiler, since CMake deduces the others
# automatically from it (respecting the order in the project call).

if (NOT DEFINED CMAKE_C_COMPILER AND NOT DEFINED ENV{CC})
    set(CMAKE_C_COMPILER "/usr/bin/gcc-arm-none-eabi/bin/arm-none-eabi-gcc")
endif ()

set(_C_FLAGS "-funsigned-bitfields")

set(_CXX_FLAGS "-femit-class-debug-always -funsigned-bitfields")

set(_EXE_LINKER_FLAGS "-specs=nano.specs -specs=nosys.specs")

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")
