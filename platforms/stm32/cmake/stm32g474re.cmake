# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# STM32G474RE chip configuration (NUCLEO-G474RE)
# Cortex-M4F, 170 MHz, 512 KB Flash, 128 KB SRAM, FDCAN x3, USART2 VCP

set(STM32_FAMILY "G4" CACHE STRING "STM32 family" FORCE)
set(STM32_DEVICE "stm32g474xx" CACHE STRING "STM32 device define" FORCE)
set(STM32_DEVICE_UPPER "STM32G474xx" CACHE STRING "STM32 device define (upper)" FORCE)
set(CAN_TYPE "FDCAN" CACHE STRING "CAN peripheral type" FORCE)

add_compile_definitions(STM32G474xx)
add_compile_definitions(STM32_FAMILY_G4)
add_compile_definitions(CAN_TYPE_FDCAN)

set(STM32_STARTUP_ASM
    "${CMAKE_CURRENT_LIST_DIR}/../bsp/bspMcu/startup/startup_stm32g474xx.s"
    CACHE STRING "Startup assembly" FORCE)
