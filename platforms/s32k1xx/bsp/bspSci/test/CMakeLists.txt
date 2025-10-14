add_executable(bspSciTest src/IncludeTest.cpp)

target_include_directories(bspSciTest PRIVATE)

target_link_libraries(bspSciTest PRIVATE utbspSci utbspIo gtest_main)

gtest_discover_tests(bspSciTest PROPERTIES LABELS "bspSciTest")
