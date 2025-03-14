set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CLANG_TARGET_TRIPLE arm-none-eabi)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if (NOT DEFINED CMAKE_C_COMPILER)
    if (NOT DEFINED ENV{CC})
        message(FATAL_ERROR "C compiler unspecified")
    endif ()

    set(CMAKE_C_COMPILER $ENV{CC})
endif ()

set(CXX_FLAGS
    "-Wno-psabi -fno-asynchronous-unwind-tables -fno-builtin \
-fshort-enums -mcpu=cortex-m4 -fmessage-length=0 -fno-common -ffunction-sections \
-fdata-sections -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mno-unaligned-access"
)

set(CMAKE_C_FLAGS "${CXX_FLAGS} -ffreestanding")
set(CMAKE_CXX_FLAGS "${CXX_FLAGS} -fno-rtti -fno-exceptions \
-fno-non-call-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit")
set(CMAKE_ASM_FLAGS "-g -mcpu=cortex-m4")
set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "s;S")

set(CMAKE_EXE_LINKER_FLAGS
    "-mcpu=cortex-m4 -static \
-mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 \
-Wl,--gc-sections -Wl,-Map,application.map,--cref")

cmake_path(GET CMAKE_C_COMPILER PARENT_PATH TOOLCHAIN_BIN_DIR)
cmake_path(GET TOOLCHAIN_BIN_DIR PARENT_PATH TOOLCHAIN_PREFIX)

set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/clang++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/clang")
set(CMAKE_LINKER "${TOOLCHAIN_BIN_DIR}/clang")
set(CMAKE_NM "${TOOLCHAIN_BIN_DIR}/llvm-nm")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/llvm-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/llvm-objdump")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN_DIR}/llvm-ranlib")
set(CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/llvm-size")
set(CMAKE_STRIP "${TOOLCHAIN_BIN_DIR}/llvm-strip")
set(CMAKE_AR "${TOOLCHAIN_BIN_DIR}/llvm-ar")
set(CMAKE_C_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_SYSROOT
    "${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${CLANG_TARGET_TRIPLE}/armv7m_soft_fpv4_sp_d16"
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_compile_definitions(REALTIME_OS=1)
add_compile_definitions(ESTL_NO_ASSERT_MESSAGE=1)
