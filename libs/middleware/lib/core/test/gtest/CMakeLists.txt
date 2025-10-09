add_executable(coreTest src/middleware_message_unittest.cpp)

target_link_libraries(coreTest PRIVATE core_headers gtest_main)

gtest_discover_tests(coreTest PROPERTIES LABELS "coreTest")
