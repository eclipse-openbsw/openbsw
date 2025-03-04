add_library(safeWatchdog src/safeWatchdog/SafeWatchdog.cpp)

target_include_directories(safeWatchdog PUBLIC include)

target_link_libraries(safeWatchdog PUBLIC safeLifecycle common util)

if (PLATFORM_SUPPORT_WATCHDOG)
    target_link_libraries(safeWatchdog PRIVATE bspMcu bspMcuWatchdog)
endif ()
