# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# BSP modules for STM32 platform

add_subdirectory(bspMcu)
add_subdirectory(bspClock)
add_subdirectory(bspInterruptsImpl)
add_subdirectory(bspUart)
add_subdirectory(bspTimer)
add_subdirectory(bspIo)
add_subdirectory(bspAdc)
add_subdirectory(bspEepromDriver)

# Aggregated BSP target
add_library(socBsp INTERFACE)
target_link_libraries(
    socBsp
    INTERFACE bspClock
              bspInterruptsImpl
              bspIo
              bspMcu
              bspTimer
              bspUart)
