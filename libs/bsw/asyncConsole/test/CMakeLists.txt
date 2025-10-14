add_executable(
    asyncConsoleTest src/console/IncludeTest.cpp src/console/DemoTest.cpp
                     src/demo/DemoCommand.cpp)

target_include_directories(asyncConsoleTest PRIVATE include)

target_link_libraries(asyncConsoleTest PRIVATE asyncFreeRtos asyncConsole
                                               asyncMockImpl gmock_main)

gtest_discover_tests(asyncConsoleTest PROPERTIES LABELS "asyncConsoleTest")
