set(PLATFORM_SUPPORT_CAN ON)
set(PLATFORM_SUPPORT_UDS ON)
set(PLATFORM_SUPPORT_IO ON)

# 3rdparty modules
add_subdirectory(3rdparty/freeRtosPosix)

# BSP modules
add_subdirectory(bsp)

# Other modules
add_subdirectory(lwipSysArch)
