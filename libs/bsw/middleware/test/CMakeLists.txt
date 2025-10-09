add_executable(middlewareTest src/core/middleware_message_unittest.cpp
                              src/queue/middleware_queue_unittest.cpp)

target_link_libraries(middlewareTest PRIVATE middlewareHeaders gmock gtest_main)

gtest_discover_tests(middlewareTest PROPERTIES LABELS "middlewareTest")
