add_executable(shedTest multi_list_test.cpp table_test.cpp)

target_link_libraries(shedTest PRIVATE shed gmock_main)

gtest_discover_tests(shedTest PROPERTIES LABELS "shedTest")
