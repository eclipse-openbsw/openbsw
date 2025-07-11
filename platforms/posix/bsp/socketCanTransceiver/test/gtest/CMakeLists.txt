add_executable(
    socketCanTransceiverTest
    socketCanTransceiver/src/can/IncludeTest.cpp
    socketCanTransceiver/src/can/SocketCanTransceiverTest.cpp)

target_link_libraries(
    socketCanTransceiverTest
    PRIVATE "-Wl,--whole-archive \"$<TARGET_FILE:bspMock>\" -Wl,--no-whole-archive"
            socketCanTransceiver bspMock gtest_main)

gtest_discover_tests(socketCanTransceiverTest
                     PROPERTIES LABELS "socketCanTransceiverTest")
