# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# NUCLEO-G474RE platform for referenceApp

add_subdirectory(bspConfiguration)
add_subdirectory(safety)
add_subdirectory(main)

if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_subdirectory(freeRtosCoreConfiguration)
    add_library(freeRtosPort ALIAS freeRtosCm4SysTickPort)
    add_library(freeRtosPortImpl ALIAS freeRtosCm4SysTick)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_subdirectory(threadXCoreConfiguration)
    add_library(threadXPort ALIAS threadXCortexM4Port)
    add_library(threadXPortImpl ALIAS threadXCortexM4)
endif ()
