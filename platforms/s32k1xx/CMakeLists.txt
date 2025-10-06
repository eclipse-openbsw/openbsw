# 3rdparty modules
add_subdirectory(3rdparty/freertos_cm4_sysTick)

# BSP modules
add_subdirectory(bsp)

# Safety modules
add_subdirectory(safety)

# Other modules
add_subdirectory(hardFaultHandler)
add_subdirectory(lwipSysArch)
