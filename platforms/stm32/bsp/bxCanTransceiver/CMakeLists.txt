# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(bxCanTransceiver src/can/transceiver/bxcan/BxCanTransceiver.cpp)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    target_include_directories(bxCanTransceiver PUBLIC include
                                                       test/mock/include)

    target_link_libraries(
        bxCanTransceiver
        PUBLIC asyncMockImpl
               bsp
               cpp2can
               etl
               bspIo
               lifecycle
               gmock)
else ()
    target_include_directories(bxCanTransceiver PUBLIC include)

    target_link_libraries(bxCanTransceiver PUBLIC async bspCan lifecycle cpp2can)
endif ()
