# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_executable(BxCanTransceiverTest src/can/BxCanTransceiverTest.cpp)

target_link_libraries(
    BxCanTransceiverTest
    PRIVATE asyncMockImpl
            bsp
            bxCanTransceiver
            cpp2can
            bspIo
            lifecycle
            bspMock
            gmock)

gtest_discover_tests(BxCanTransceiverTest
                     PROPERTIES LABELS "BxCanTransceiverTest")
