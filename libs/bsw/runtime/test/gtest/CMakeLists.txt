add_executable(
    runtimeTest
    runtime/FunctionExecutionMonitorTest.cpp
    runtime/FunctionRuntimeStatisticsTest.cpp
    runtime/NestedRuntimeEntryTest.cpp
    runtime/RuntimeMonitorTest.cpp
    runtime/RuntimeStackEntryTest.cpp
    runtime/RuntimeStackTest.cpp
    runtime/RuntimeStatisticsTest.cpp
    runtime/SharedStatisticsContainerTest.cpp
    runtime/SimpleRuntimeEntryTest.cpp
    runtime/StatisticsContainerTest.cpp
    runtime/StatisticsIteratorTest.cpp
    runtime/StatisticsWriterTest.cpp)

target_link_libraries(
    runtimeTest
    PRIVATE runtime
            asyncMockImpl
            bspMock
            utilMock
            gtest_main)

gtest_discover_tests(runtimeTest PROPERTIES LABELS "runtimeTest")
