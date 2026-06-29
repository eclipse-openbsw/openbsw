add_executable(timeTest src/time/TimestampProviderTest.cpp)

target_link_libraries(timeTest PRIVATE time bspMock gmock_main)

gtest_discover_tests(timeTest PROPERTIES LABELS "timeTest")
