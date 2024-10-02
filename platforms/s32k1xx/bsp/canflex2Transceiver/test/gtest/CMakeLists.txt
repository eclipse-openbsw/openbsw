add_executable(CanFlex2TransceiverTest src/can/CanFlex2TransceiverTest.cpp)

target_include_directories(CanFlex2TransceiverTest PRIVATE ../../include
                                                           ../mock/include)

target_link_libraries(
    CanFlex2TransceiverTest
    PRIVATE asyncMockImpl
            bsp
            canflex2Transceiver
            cpp2can
            utbspIo
            lifecycle
            bspMock
            gmock)

gtest_discover_tests(CanFlex2TransceiverTest
                     PROPERTIES LABELS "CanFlex2TransceiverTest")
