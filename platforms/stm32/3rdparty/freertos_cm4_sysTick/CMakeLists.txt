# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

add_library(freeRtosCm4SysTickPort INTERFACE)

target_include_directories(freeRtosCm4SysTickPort INTERFACE include)

add_library(freeRtosCm4SysTick src/port.c)

target_include_directories(freeRtosCm4SysTick PUBLIC include)

target_link_libraries(
    freeRtosCm4SysTick
    PUBLIC freeRtos
    PRIVATE bspInterrupts)

target_compile_options(freeRtosCm4SysTick PRIVATE -Wno-unused-but-set-variable)
