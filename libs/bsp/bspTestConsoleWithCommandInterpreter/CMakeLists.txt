add_library(bspTestConsoleWithCommandInterpreter src/console/Console.cpp
                                                 src/console/ConsoleMenu.cpp)

target_include_directories(bspTestConsoleWithCommandInterpreter PUBLIC include)

target_link_libraries(
    bspTestConsoleWithCommandInterpreter
    PUBLIC bsp bspCommandInterpreterSimple
    PRIVATE bspCharInputOutput)
