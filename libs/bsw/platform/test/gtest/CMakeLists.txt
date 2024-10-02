add_executable(platformTest src/platform/config.cpp src/platform/estdint.cpp
                            src/platform/estdint_c.c)

target_link_libraries(platformTest PRIVATE platform gtest_main)

gtest_discover_tests(platformTest PROPERTIES LABELS "platformTest")
