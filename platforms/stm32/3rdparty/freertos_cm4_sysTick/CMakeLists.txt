add_library(freeRtosCm4SysTickPort INTERFACE)

target_include_directories(freeRtosCm4SysTickPort INTERFACE .)

add_library(freeRtosCm4SysTick port.c)

target_include_directories(freeRtosCm4SysTick PUBLIC .)

target_link_libraries(
    freeRtosCm4SysTick
    PUBLIC freeRtos
    PRIVATE bspInterrupts)

target_compile_options(freeRtosCm4SysTick PRIVATE -Wno-unused-but-set-variable)
