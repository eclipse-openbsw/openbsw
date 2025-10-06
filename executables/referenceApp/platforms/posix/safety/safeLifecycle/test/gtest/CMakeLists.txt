add_executable(safeLifecycleTest src/safeLifecycle/include_test.cpp)

target_link_libraries(safeLifecycleTest PRIVATE safeLifecycle gtest_main)

gtest_discover_tests(safeLifecycleTest PROPERTIES LABELS "safeLifecycleTest")
