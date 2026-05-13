# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(
    bspConfiguration
    src/bsp/stdIo/stdIo.cpp
    src/bsp/uart/UartConfig.cpp)

target_include_directories(bspConfiguration PUBLIC include)

target_link_libraries(bspConfiguration PUBLIC bspUart PRIVATE bspMcu platform)
