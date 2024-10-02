add_executable(asyncConsoleTest src/console/include_test.cpp)

target_include_directories(asyncConsoleTest PRIVATE)

target_link_libraries(asyncConsoleTest PRIVATE asyncConsole asyncMock
                                               gmock_main)

gtest_discover_tests(asyncConsoleTest PROPERTIES LABELS "asyncConsoleTest")
