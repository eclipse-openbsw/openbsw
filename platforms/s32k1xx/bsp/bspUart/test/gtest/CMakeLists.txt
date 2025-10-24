add_executable(bspUartTest src/IncludeTest.cpp)

target_include_directories(bspUartTest PRIVATE)

target_link_libraries(bspUartTest PRIVATE utbspUart utbspIo gtest_main)

gtest_discover_tests(bspUartTest PROPERTIES LABELS "bspUartTest")
