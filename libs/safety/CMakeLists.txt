option(PLATFORM_SUPPORT_WATCHDOG "Turn WATCHDOG support on or off" OFF)
option(PLATFORM_SUPPORT_MPU "Turn MPU support on or off" OFF)

add_subdirectory(safeLifecycle)
add_subdirectory(safeMemory)
add_subdirectory(safeMonitor)
add_subdirectory(safeWatchdog)
add_subdirectory(watchdogManager)
