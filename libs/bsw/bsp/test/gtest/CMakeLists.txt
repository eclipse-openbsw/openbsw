add_executable(
    bspTest
    src/bsp/can/canTransceiver/CanPhyTest.cpp
    src/bsp/timer/IsEqualAfterTimeoutTest.cpp
    src/bsp/IncludeTest.cpp
    )

target_include_directories(bspTest PRIVATE)

target_link_libraries(bspTest PRIVATE bsp bspMock util)

gtest_discover_tests(bspTest PROPERTIES LABELS "bspTest")
