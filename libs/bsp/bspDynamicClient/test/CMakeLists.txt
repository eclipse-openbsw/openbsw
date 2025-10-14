add_executable(bspDynamicClientTest src/DynamicClientTest.cpp)

target_link_libraries(bspDynamicClientTest PRIVATE bspDynamicClient gmock_main)

gtest_discover_tests(bspDynamicClientTest PROPERTIES LABELS
                                                     "bspDynamicClientTest")
