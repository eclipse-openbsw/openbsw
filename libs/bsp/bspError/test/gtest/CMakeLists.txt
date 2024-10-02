add_executable(bspErrorTest src/IncludeTest.cpp)

target_link_libraries(bspErrorTest PRIVATE bspError util gtest_main)

gtest_discover_tests(bspErrorTest PROPERTIES LABELS "bspErrorTest")
