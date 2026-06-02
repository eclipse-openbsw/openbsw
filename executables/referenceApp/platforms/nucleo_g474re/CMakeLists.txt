# NUCLEO-G474RE platform for referenceApp

add_subdirectory(bspConfiguration)
add_subdirectory(safety)
add_subdirectory(main)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(freeRtosCoreConfiguration)
    add_library(freeRtosPort ALIAS freeRtosCm4SysTickPort)
    add_library(freeRtosPortImpl ALIAS freeRtosCm4SysTick)
endif ()
