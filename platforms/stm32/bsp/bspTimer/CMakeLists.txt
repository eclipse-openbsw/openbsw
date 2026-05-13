# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(bspTimer src/bsp/SystemTimer/SystemTimer.cpp)

target_link_libraries(bspTimer PUBLIC bsp bspMcu bspInterrupts)
