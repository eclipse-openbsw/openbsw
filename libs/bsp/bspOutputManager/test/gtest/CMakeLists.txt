add_executable(bspOutputManagerTest src/IncludeTest.cpp)
target_include_directories(bspOutputManagerTest PRIVATE ../../include
                                                        ../include)
target_link_libraries(bspOutputManagerTest PRIVATE bspDynamicClient utbspIo
                                                   util gtest_main)
gtest_discover_tests(bspOutputManagerTest PROPERTIES LABELS
                                                     "bspOutputManagerTest")
