add_executable(safeMemory src/safeMemory/include_test.cpp)

target_link_libraries(safeMemory PRIVATE safeLifecycle gtest_main)

gtest_discover_tests(safeMemory PROPERTIES LABELS "safeMemoryTest")
