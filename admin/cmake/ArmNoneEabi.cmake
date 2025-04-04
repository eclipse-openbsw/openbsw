set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(ARM_TARGET_TRIPLE arm-none-eabi)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_compile_definitions(REALTIME_OS=1)
add_compile_definitions(ESTL_NO_ASSERT_MESSAGE=1)

set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "s;S")

set(CPU_BUILD_FLAGS "-mcpu=cortex-m4 \
-mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0")
set(C_COMMON_FLAGS
    "${CPU_BUILD_FLAGS} -mthumb -fshort-enums \
-mno-unaligned-access -Wno-psabi \
-fno-asynchronous-unwind-tables -fno-builtin -fno-common \
-ffunction-sections -fdata-sections")

set(CMAKE_ASM_FLAGS "-g -mcpu=cortex-m4")
set(CMAKE_C_FLAGS "${C_COMMON_FLAGS} -ffreestanding")
set(CMAKE_CXX_FLAGS "${C_COMMON_FLAGS} -fno-rtti -fno-exceptions \
-fno-non-call-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_BUILD_FLAGS} -static \
-Wl,--gc-sections -Wl,-Map,application.map,--cref")
