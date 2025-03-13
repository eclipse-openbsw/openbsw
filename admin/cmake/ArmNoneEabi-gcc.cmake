# Toolchain for gcc-arm-none-eabi-10.3-2021.10
include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -funsigned-bitfields")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -funsigned-bitfields -femit-class-debug-always")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
--specs=nano.specs -specs=nosys.specs")

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-as)
set(CMAKE_LINKER arm-none-eabi-g++)
set(CMAKE_AR arm-none-eabi-ar)
