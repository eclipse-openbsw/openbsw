add_executable(lwipSocketTest src/lwipSocket/LwipSocketTest.cpp)

target_link_libraries(lwipSocketTest PRIVATE lwipSocket gtest_main lwipcore)

gtest_discover_tests(lwipSocketTest PROPERTIES LABELS "lwipSocketTest")
