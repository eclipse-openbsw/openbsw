# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

if (NOT BUILD_EXECUTABLE STREQUAL "unitTest")
    set(bspInterruptsImplName "bspInterruptsImpl")
else ()
    set(bspInterruptsImplName "bspInterruptsImplUt")
endif ()

add_library(${bspInterruptsImplName} src/interrupt_manager.c)

target_include_directories(${bspInterruptsImplName} PUBLIC include)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_include_directories(${bspInterruptsImplName} PUBLIC freertos/include)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_include_directories(${bspInterruptsImplName} PUBLIC threadx/include)
    target_link_libraries(${bspInterruptsImplName} PUBLIC threadX)
endif ()

target_link_libraries(${bspInterruptsImplName} PRIVATE bspMcu platform)
