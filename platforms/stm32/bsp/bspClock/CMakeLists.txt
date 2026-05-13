# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# Clock configuration — chip-specific source selected by STM32_FAMILY
if (STM32_FAMILY STREQUAL "F4")
    add_library(bspClock src/clockConfig_f4.cpp)
elseif (STM32_FAMILY STREQUAL "G4")
    add_library(bspClock src/clockConfig_g4.cpp)
endif ()

target_include_directories(bspClock PUBLIC include)

target_link_libraries(bspClock PRIVATE bspMcu)
