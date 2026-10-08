add_executable(canTransceiverTest src/can/CanTransceiverTest.cpp)

target_link_libraries(
    canTransceiverTest
    PRIVATE asyncMockImpl
            bsp
            canTransceiver
            cpp2can
            bspIo
            lifecycle
            bspMock
            gmock)

gtest_discover_tests(canTransceiverTest PROPERTIES LABELS "canTransceiverTest")
