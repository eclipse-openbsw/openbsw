add_library(safeBspMcuWatchdog src/watchdog/Watchdog.cpp)

target_include_directories(safeBspMcuWatchdog PUBLIC include)

target_link_libraries(
    safeBspMcuWatchdog
    PUBLIC platform
    PRIVATE bsp bspInterrupts bspMcu)
