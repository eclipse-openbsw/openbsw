# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(fdCanTransceiver src/can/transceiver/fdcan/FdCanTransceiver.cpp)

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    target_include_directories(fdCanTransceiver PUBLIC include
                                                       test/mock/include)

    target_link_libraries(
        fdCanTransceiver
        PUBLIC asyncMockImpl
               bsp
               cpp2can
               etl
               bspIo
               lifecycle
               gmock)
else ()
    target_include_directories(fdCanTransceiver PUBLIC include)

    target_link_libraries(fdCanTransceiver PUBLIC async bspCan lifecycle cpp2can)
endif ()
