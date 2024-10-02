add_executable(
    cpp2canTest
    src/can/canframes/CANFrameTest.cpp
    src/can/canframes/CanIdTest.cpp
    src/can/filter/BitFieldFilterTest.cpp
    src/can/filter/IntervalFilterTest.cpp
    src/can/framemgmt/CANFrameListenerProxyTest.cpp
    src/can/transceiver/AbstractCANTransceiverTest.cpp)

target_include_directories(cpp2canTest PRIVATE)

target_link_libraries(cpp2canTest PRIVATE bspMock cpp2can cpp2canMock)

gtest_discover_tests(cpp2canTest PROPERTIES LABELS "cpp2canTest")
