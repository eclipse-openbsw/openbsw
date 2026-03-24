# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# BSP modules for STM32 platform

if (BUILD_EXECUTABLE STREQUAL "unitTest")
    # Unit test builds: only compile transceivers (with mock devices).
    # Skip hardware-dependent modules (bspMcu, bspClock, bspCan, etc.).
    add_subdirectory(bxCanTransceiver)
    add_subdirectory(fdCanTransceiver)
    add_subdirectory(bspUart)
else ()
    add_subdirectory(bspMcu)
    add_subdirectory(bspClock)
    add_subdirectory(bspInterruptsImpl)
    add_subdirectory(bspUart)
    add_subdirectory(bspCan)
    add_subdirectory(bspTimer)

    # CAN transceiver — selected by chip family
    if (CAN_TYPE STREQUAL "BXCAN")
        add_subdirectory(bxCanTransceiver)
    elseif (CAN_TYPE STREQUAL "FDCAN")
        add_subdirectory(fdCanTransceiver)
    endif ()

    # Aggregated BSP target
    add_library(socBsp INTERFACE)
    target_link_libraries(
        socBsp
        INTERFACE bspClock
                  bspInterruptsImpl
                  bspMcu
                  bspTimer
                  bspUart)
endif ()
