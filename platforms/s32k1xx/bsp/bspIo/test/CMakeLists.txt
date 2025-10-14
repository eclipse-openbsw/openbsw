add_executable(bspIoTest src/IncludeTest.cpp)

target_include_directories(bspIoTest PRIVATE)

target_link_libraries(bspIoTest PRIVATE gtest_main utbspIo utbspMcu)
gtest_discover_tests(bspIoTest PROPERTIES LABELS "bspIoTest")
