add_executable(bspSciTest src/IncludeTest.cpp)

target_include_directories(bspSciTest PRIVATE ../include)

target_link_libraries(bspSciTest PRIVATE gtest_main bspIo utbspMcu)

gtest_discover_tests(bspSciTest PROPERTIES LABELS "bspSciTest")
