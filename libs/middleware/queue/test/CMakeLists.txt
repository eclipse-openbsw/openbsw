add_executable(queueTest src/middleware_queue_unittest.cpp)

target_link_libraries(queueTest PRIVATE queue gtest_main)

gtest_discover_tests(queueTest PROPERTIES LABELS "queueTest")
