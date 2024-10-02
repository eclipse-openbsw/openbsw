add_library(
    asyncConsole
    src/console/AsyncCommandWrapper.cpp src/console/AsyncConsole.cpp
    src/console/SyncCommandWrapper.cpp src/logger/ConsoleLogger.cpp)

target_include_directories(asyncConsole PUBLIC include)

target_link_libraries(asyncConsole PUBLIC async util)
