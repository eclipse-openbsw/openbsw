set(PLATFORM_SUPPORT_CAN TRUE)
set(PLATFORM_SUPPORT_UDS TRUE)
set(PLATFORM_SUPPORT_IO TRUE)

# 3rdparty modules
add_subdirectory(3rdparty/freeRtosPosix)

# BSP modules
add_subdirectory(bsp)

# Other modules
add_subdirectory(lwipSysArch)
