option(PLATFORM_SUPPORT_WATCHDOG "Turn WATCHDOG support on or off" OFF)
option(PLATFORM_SUPPORT_MPU "Turn MPU support on or off" OFF)

if (BUILD_TARGET_PLATFORM STREQUAL "S32K148EVB")
    set(PLATFORM_SUPPORT_WATCHDOG
        ON
        CACHE BOOL "Turn ON Watchdog support" FORCE)
    set(PLATFORM_SUPPORT_MPU
        ON
        CACHE BOOL "Turn ON MPU support" FORCE)
else ()
    set(PLATFORM_SUPPORT_WATCHDOG
        OFF
        CACHE BOOL "Turn OFF Watchdog support" FORCE)
    set(PLATFORM_SUPPORT_MPU
        OFF
        CACHE BOOL "Turn OFF MPU support" FORCE)
endif ()

if (PLATFORM_SUPPORT_WATCHDOG)
    add_compile_definitions(PLATFORM_SUPPORT_WATCHDOG=1)
endif ()
if (PLATFORM_SUPPORT_MPU)
    add_compile_definitions(PLATFORM_SUPPORT_MPU=1)
endif ()

add_subdirectory(safeLifecycle EXCLUDE_FROM_ALL)
add_subdirectory(safeMemory EXCLUDE_FROM_ALL)
add_subdirectory(safeMonitor EXCLUDE_FROM_ALL)
add_subdirectory(safeWatchdog EXCLUDE_FROM_ALL)
add_subdirectory(watchdogManager EXCLUDE_FROM_ALL)

if (BUILD_UNIT_TESTS)
    add_subdirectory(safeLifecycle/test/gtest)
    add_subdirectory(safeMonitor/test)
endif ()
