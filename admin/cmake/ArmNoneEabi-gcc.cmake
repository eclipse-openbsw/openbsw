include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

set(_C_FLAGS "-funsigned-bitfields")

set(_CXX_FLAGS "-femit-class-debug-always \
    -funsigned-bitfields")

set(_EXE_LINKER_FLAGS "-specs=nano.specs \
    -specs=nosys.specs")

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-as)
set(CMAKE_LINKER arm-none-eabi-g++)
set(CMAKE_AR arm-none-eabi-ar)
