# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(bspIo src/io/Gpio.cpp)
target_include_directories(bspIo PUBLIC include)
target_link_libraries(bspIo PUBLIC bspMcu PRIVATE platform)
