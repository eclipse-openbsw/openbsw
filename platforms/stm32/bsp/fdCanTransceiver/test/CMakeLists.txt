# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_executable(FdCanTransceiverTest src/can/FdCanTransceiverTest.cpp)

target_link_libraries(
    FdCanTransceiverTest
    PRIVATE asyncMockImpl
            bsp
            fdCanTransceiver
            cpp2can
            bspIo
            lifecycle
            bspMock
            gmock)

gtest_discover_tests(FdCanTransceiverTest
                     PROPERTIES LABELS "FdCanTransceiverTest")
