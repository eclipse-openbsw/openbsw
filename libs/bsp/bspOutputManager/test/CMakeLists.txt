add_executable(bspOutputManagerTest src/IncludeTest.cpp)
target_link_libraries(bspOutputManagerTest PUBLIC bspOutputManager gtest_main)
gtest_discover_tests(bspOutputManagerTest PROPERTIES LABELS
                                                     "bspOutputManagerTest")
