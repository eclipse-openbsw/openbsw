add_executable(bspCommandInterpreterSimpleTest src/IncludeTest.cpp)

target_link_libraries(bspCommandInterpreterSimpleTest PRIVATE gtest_main)

gtest_discover_tests(bspCommandInterpreterSimpleTest
                     PROPERTIES LABELS "bspCommandInterpreterSimpleTest")
