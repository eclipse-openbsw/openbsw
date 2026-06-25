add_executable(WatchdogTest src/WatchdogTest.cpp)

target_include_directories(WatchdogTest PRIVATE include ../include ../src)

target_link_libraries(WatchdogTest PRIVATE platform gmock gtest_main)

gtest_discover_tests(WatchdogTest PROPERTIES LABELS "WatchdogTest")
