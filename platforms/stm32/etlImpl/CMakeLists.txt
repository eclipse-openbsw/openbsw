# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(etlImpl src/print.cpp src/clocks.cpp)

target_link_libraries(etlImpl PRIVATE etl bsp util)
