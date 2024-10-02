add_executable(bspTestConsoleWithCommandInterpreterTest src/IncludeTest.cpp)

target_include_directories(bspTestConsoleWithCommandInterpreterTest
                           PRIVATE ../../include)

target_link_libraries(bspTestConsoleWithCommandInterpreterTest
                      PRIVATE bspCommandInterpreterSimple gtest_main)

gtest_discover_tests(
    bspTestConsoleWithCommandInterpreterTest
    PROPERTIES LABELS "bspTestConsoleWithCommandInterpreterTest")
