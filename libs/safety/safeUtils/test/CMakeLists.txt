add_executable(safeUtilsTest src/safeUtils/include_test.cpp)

target_link_libraries(safeUtilsTest PRIVATE safeUtils gtest_main)

gtest_discover_tests(safeUtilsTest PROPERTIES LABELS "safeUtilsTest")
