set(consoleCommands_SOURCES)
list(
    APPEND
    consoleCommands_SOURCES
    src/lifecycle/console/LifecycleControlCommand.cpp
    src/lifecycle/console/StatisticsCommand.cpp
    src/can/console/CanCommand.cpp)

if (PLATFORM_SUPPORT_IO)
    list(APPEND consoleCommands_SOURCES src/safety/console/SafetyCommand.cpp)
endif ()

add_library(consoleCommands ${consoleCommands_SOURCES})

target_include_directories(consoleCommands PUBLIC include)

target_link_libraries(
    consoleCommands
    PUBLIC lifecycle
           asyncBinding
           asyncCoreConfiguration
           runtime
           configuration
           cpp2can
           safeUtils)

if (PLATFORM_SUPPORT_IO)
    target_link_libraries(consoleCommands PRIVATE safeIo bspIo)
endif ()
