add_executable(bspCoreTest src/IncludeTest.cpp)

target_include_directories(bspCoreTest PRIVATE ../include)

target_link_libraries(bspCoreTest PRIVATE platform gtest_main)

gtest_discover_tests(bspCoreTest PROPERTIES LABELS "bspCoreTest")
