add_executable(
    bspTest
    src/bsp/background/AbstractBackgroundJobTest.cpp
    src/bsp/can/canTransceiver/CanPhyTest.cpp
    src/bsp/can/ICanPhyTest.cpp
    src/bsp/lin/linTransceiver/LinPhyTest.cpp
    src/bsp/power/AbstractPowerStateListenerTest.cpp
    src/bsp/timer/IsEqualAfterTimeoutTest.cpp
    src/bsp/IncludeTest.cpp
    src/bsp/IncludeTestCommandInterpreterSimple.cpp)

target_include_directories(bspTest PRIVATE)

target_link_libraries(bspTest PRIVATE bsp bspMock util)

gtest_discover_tests(bspTest PROPERTIES LABELS "bspTest")
