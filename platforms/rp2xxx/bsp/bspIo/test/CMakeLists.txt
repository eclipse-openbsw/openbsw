add_executable(bspIoTest src/IncludeTest.cpp)

target_link_libraries(bspIoTest PRIVATE gtest_main bspIo utbspMcu)

gtest_discover_tests(bspIoTest PROPERTIES LABELS "bspIoTest")
