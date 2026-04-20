# 3rdparty modules Configure

# Rtos libs depending on target rtos.
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(3rdparty/freertos_cm4_sysTick)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_subdirectory(3rdparty/threadx)
endif ()

# BSP modules
add_subdirectory(bsp)

# Safety modules
add_subdirectory(safety)

# ETL implementation
add_subdirectory(etlImpl)

# Other modules
if (BUILD_EXECUTABLE STREQUAL "referenceApp")
    add_subdirectory(hardFaultHandler)
endif ()
add_subdirectory(lwipSysArch)
