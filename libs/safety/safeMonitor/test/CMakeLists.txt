add_executable(
    safeMonitorTest
    src/RegisterTest.cpp
    src/SequenceTest.cpp
    src/TriggerTest.cpp
    src/ValueTest.cpp
    src/WatchdogTest.cpp
    src/common.cpp)

target_include_directories(safeMonitorTest PRIVATE include)

target_link_libraries(safeMonitorTest PRIVATE safeMonitor gmock_main)

gtest_discover_tests(safeMonitorTest PROPERTIES LABELS "safeMonitorTest")
