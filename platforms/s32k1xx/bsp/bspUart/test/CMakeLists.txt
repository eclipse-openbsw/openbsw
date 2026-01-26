add_executable(bspUartTest src/IncludeTest.cpp)

target_include_directories(bspUartTest PRIVATE mock/include)

target_link_libraries(bspUartTest PRIVATE bspMock bspIo gmock gtest_main)

gtest_discover_tests(bspUartTest PROPERTIES LABELS "bspUartTest")
