add_library(
    consoleCommands
    src/lifecycle/console/LifecycleControlCommand.cpp
    src/lifecycle/console/StatisticsCommand.cpp src/can/console/CanCommand.cpp)

target_include_directories(consoleCommands PUBLIC include)

target_link_libraries(
    consoleCommands
    PUBLIC lifecycle
           asyncBinding
           asyncCoreConfiguration
           runtime
           configuration
           cpp2can)
