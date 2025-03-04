add_library(watchdogManager src/watchdogManager/watchdogManager.cpp)

target_include_directories(watchdogManager PUBLIC include)

target_link_libraries(
    watchdogManager
    PUBLIC bsp
           timer
           common
           bspMcu
           safeLifecycle)

if (PLATFORM_SUPPORT_WATCHDOG)
    target_link_libraries(watchdogManager PRIVATE bspMcuWatchdog)
endif ()
