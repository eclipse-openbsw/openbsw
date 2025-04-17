option(PLATFORM_SUPPORT_WATCHDOG "Turn WATCHDOG support on or off" OFF)
option(PLATFORM_SUPPORT_MPU "Turn MPU support on or off" OFF)

add_subdirectory(safeLifecycle EXCLUDE_FROM_ALL)
add_subdirectory(safeMemory EXCLUDE_FROM_ALL)
add_subdirectory(safeMonitor EXCLUDE_FROM_ALL)
add_subdirectory(safeWatchdog EXCLUDE_FROM_ALL)
add_subdirectory(watchdogManager EXCLUDE_FROM_ALL)

if (BUILD_UNIT_TESTS)
    add_subdirectory(safeLifecycle/test/gtest)
    add_subdirectory(safeMonitor/test)
endif ()
