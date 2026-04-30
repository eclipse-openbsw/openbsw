# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(bspAdc src/adc/Adc.cpp)
target_include_directories(bspAdc PUBLIC include)
target_link_libraries(bspAdc PUBLIC bspMcu PRIVATE platform)
