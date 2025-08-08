add_executable(storageTest src/StorageTest.cpp)

target_link_libraries(storageTest PRIVATE storage storageMock bspMock asyncMockImpl gmock_main)

gtest_discover_tests(storageTest PROPERTIES LABELS "storageTest")
