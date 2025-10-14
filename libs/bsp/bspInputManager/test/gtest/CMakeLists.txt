add_executable(bspInputManagerTest src/IncludeTest.cpp)

target_include_directories(bspInputManagerTest PRIVATE ../../include ../include)

target_link_libraries(bspInputManagerTest PRIVATE bspDynamicClient utbspIo util
                                                  gtest_main)

gtest_discover_tests(bspInputManagerTest PROPERTIES LABELS
                                                    "bspInputManagerTest")
