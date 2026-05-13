# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# Low-level CAN device drivers — chip-specific source selected by CAN_TYPE
if (CAN_TYPE STREQUAL "BXCAN")
    add_library(bspCan src/can/BxCanDevice.cpp)
    target_include_directories(bspCan PUBLIC include)
    target_link_libraries(bspCan PUBLIC bspMcu cpp2can etl PRIVATE platform)
elseif (CAN_TYPE STREQUAL "FDCAN")
    add_library(bspCan src/can/FdCanDevice.cpp)
    target_include_directories(bspCan PUBLIC include)
    target_link_libraries(bspCan PUBLIC bspMcu cpp2can etl PRIVATE platform)
endif ()
