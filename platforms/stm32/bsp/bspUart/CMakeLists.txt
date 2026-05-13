# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    # Header-only for unit tests (no hardware-dependent Uart.cpp)
    add_library(bspUart INTERFACE)
    target_include_directories(bspUart INTERFACE include)
    target_link_libraries(bspUart INTERFACE bsp)
else ()
    add_library(bspUart src/Uart.cpp)
    target_include_directories(bspUart PUBLIC include)
    target_link_libraries(bspUart PUBLIC bsp PRIVATE bspMcu)
endif ()
