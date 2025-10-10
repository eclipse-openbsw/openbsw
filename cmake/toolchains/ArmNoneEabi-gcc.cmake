include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

set(_C_FLAGS "-funsigned-bitfields")

set(_CXX_FLAGS "-femit-class-debug-always \
    -funsigned-bitfields")

set(_EXE_LINKER_FLAGS "-specs=nano.specs \
    -specs=nosys.specs")

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")

if (NOT DEFINED CMAKE_C_COMPILER)
    if (DEFINED ENV{CC})
        set(CMAKE_C_COMPILER $ENV{CC})
    else ()
        find_program(CMAKE_C_COMPILER arm-none-eabi-gcc REQUIRED)
    endif ()
endif ()

cmake_path(GET CMAKE_C_COMPILER PARENT_PATH TOOLCHAIN_BIN_DIR)

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-gcc)
set(CMAKE_LINKER ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-g++)
set(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/arm-none-eabi-ar)
