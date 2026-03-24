# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# ============================================================================
# NUCLEO-F413ZH platform main build
#
# Builds the platform entry point, CAN system (bxCAN via bxCanTransceiver),
# BSP lifecycle, and ISR overrides for the STM32F413ZH target.
# ============================================================================

# startUp is an INTERFACE library — it carries no compiled sources, only
# propagates link dependencies (bspMcu, common, hardFaultHandler) and the
# linker script to consumers via INTERFACE properties.
add_library(startUp INTERFACE)

target_link_libraries(
    startUp
    INTERFACE bspMcu
              common
              hardFaultHandler)

add_library(
    main
    src/os/isr/isr_can.cpp
    src/systems/CanSystem.cpp
    src/lifecycle/StaticBsp.cpp
    src/main.cpp)

target_include_directories(main PUBLIC include)

# bxCanTransceiver — the STM32F413ZH has a classic bxCAN peripheral (not FDCAN).
# This selects the bxCAN driver used by CanSystem for 500 kbit/s CAN.
target_link_libraries(
    main
    PUBLIC bspConfiguration
    PRIVATE bspClock
            bspTimer
            bspUart
            bxCanTransceiver
            configuration
            etl
            lifecycle
            safeSupervisor
            startUp)

# RTOS-specific linking
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    target_link_libraries(main PRIVATE freeRtosCm4SysTick)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    target_sources(startUp INTERFACE
        src/bsp/threadx/tx_initialize_low_level.S
        src/bsp/threadx/tx_execution_initialize.c)
    target_link_libraries(main PRIVATE threadXCortexM4)
endif ()

# --whole-archive forces the linker to include ALL object files from the main
# library, even if no other translation unit references them. This is required
# because isr_can.cpp provides strong ISR definitions (CAN1_RX0_IRQHandler,
# CAN1_TX_IRQHandler) that override the weak default handlers in the startup
# assembly. Without --whole-archive the linker would discard isr_can.o as
# "unreferenced" and the weak stubs would remain, silently swallowing CAN IRQs.
set_target_properties(main PROPERTIES INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE "")
target_link_options(main INTERFACE
    "LINKER:--whole-archive" "$<TARGET_FILE:main>" "LINKER:--no-whole-archive")

set_target_properties(
    startUp
    PROPERTIES PROP_LINKER_SCRIPT
               "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/application.ld")
set_target_properties(
    startUp
    PROPERTIES LINK_DEPENDS
               "${CMAKE_CURRENT_SOURCE_DIR}/linkerscript/application.ld")

# RTOS-specific osHooks
if (BUILD_TARGET_RTOS STREQUAL "FREERTOS")
    add_library(osHooks src/osHooks/freertos/osHooks.cpp)
    target_link_libraries(osHooks PRIVATE common freeRtosCm4SysTick)
elseif (BUILD_TARGET_RTOS STREQUAL "THREADX")
    add_library(osHooks src/osHooks/threadx/osHooks.cpp)
    target_link_libraries(osHooks PRIVATE common threadXCortexM4 asyncThreadX
                                          threadXConfiguration)
endif ()
target_include_directories(osHooks PRIVATE include)
