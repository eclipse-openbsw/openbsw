add_executable(
    runtimeTest
    src/FunctionExecutionMonitorTest.cpp
    src/FunctionRuntimeStatisticsTest.cpp
    src/NestedRuntimeEntryTest.cpp
    src/RuntimeMonitorTest.cpp
    src/RuntimeStackEntryTest.cpp
    src/RuntimeStackTest.cpp
    src/RuntimeStatisticsTest.cpp
    src/SharedStatisticsContainerTest.cpp
    src/SimpleRuntimeEntryTest.cpp
    src/StatisticsContainerTest.cpp
    src/StatisticsIteratorTest.cpp
    src/StatisticsWriterTest.cpp)

target_link_libraries(
    runtimeTest
    PRIVATE runtime
            asyncMockImpl
            bspMock
            utilMock
            gtest_main)

gtest_discover_tests(runtimeTest PROPERTIES LABELS "runtimeTest")
