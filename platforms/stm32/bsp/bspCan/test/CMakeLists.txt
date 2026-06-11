add_executable(BxCanDeviceTest src/can/BxCanDeviceTest.cpp)

target_include_directories(BxCanDeviceTest PRIVATE include ../include ../src)

target_link_libraries(BxCanDeviceTest PRIVATE cpp2can etl gmock gtest_main)

gtest_discover_tests(BxCanDeviceTest PROPERTIES LABELS "BxCanDeviceTest")

add_executable(FdCanDeviceTest src/can/FdCanDeviceTest.cpp)

target_include_directories(FdCanDeviceTest PRIVATE include ../include ../src)

target_link_libraries(FdCanDeviceTest PRIVATE cpp2can etl gmock gtest_main)

gtest_discover_tests(FdCanDeviceTest PROPERTIES LABELS "FdCanDeviceTest")
