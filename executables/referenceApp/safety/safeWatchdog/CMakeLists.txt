add_library(safeWatchdog src/safeWatchdog/SafeWatchdog.cpp)

target_include_directories(safeWatchdog PUBLIC include)

target_link_libraries(
    safeWatchdog
    PUBLIC platform
    PRIVATE safeSupervisor safeUtils)

if (PLATFORM_SUPPORT_WATCHDOG)
    target_link_libraries(safeWatchdog PRIVATE bspMcu safeBspMcuWatchdog)
endif ()
