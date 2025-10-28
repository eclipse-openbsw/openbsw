add_executable(bspCharInputOutputTest src/IncludeTest.cpp)

target_link_libraries(bspCharInputOutputTest PRIVATE bspCharInputOutput
                                                     gtest_main)

gtest_discover_tests(bspCharInputOutputTest PROPERTIES LABELS
                                                       "bspCharInputOutputTest")
