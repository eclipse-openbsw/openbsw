add_executable(asyncTest src/async/util/CallTest.cpp
                         src/async/TestContextTest.cpp src/async/TypesTest.cpp)

target_link_libraries(asyncTest PRIVATE asyncMockImpl async gmock_main)

gtest_discover_tests(asyncTest PROPERTIES LABELS "asyncTest")
