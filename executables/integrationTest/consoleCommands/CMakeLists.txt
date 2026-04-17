set(consoleCommands_SOURCES)
list(APPEND consoleCommands_SOURCES
     src/lifecycle/console/LifecycleControlCommand.cpp
     src/lifecycle/console/StatisticsCommand.cpp)

add_library(consoleCommands ${consoleCommands_SOURCES})

target_include_directories(consoleCommands PUBLIC include)

target_link_libraries(
    consoleCommands
    PUBLIC lifecycle
           asyncBinding
           asyncCoreConfiguration
           runtime
           cpp2can)
