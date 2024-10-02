add_library(
    bspCommandInterpreterSimple
    src/commandInterpreterSimple/CommandInterpreter.cpp
    src/commandInterpreterSimple/CommandLine.cpp)

target_include_directories(bspCommandInterpreterSimple PUBLIC include)

target_link_libraries(bspCommandInterpreterSimple PUBLIC bsp)
