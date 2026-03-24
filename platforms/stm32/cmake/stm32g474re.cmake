# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# STM32G474RE chip configuration (NUCLEO-G474RE)
# Cortex-M4F, 170 MHz, 512 KB Flash, 128 KB SRAM, FDCAN x3, USART2 VCP

set(STM32_FAMILY "G4" CACHE STRING "STM32 family" FORCE)
set(STM32_DEVICE "stm32g474xx" CACHE STRING "STM32 device define" FORCE)
set(STM32_DEVICE_UPPER "STM32G474xx" CACHE STRING "STM32 device define (upper)" FORCE)
set(CAN_TYPE "FDCAN" CACHE STRING "CAN peripheral type" FORCE)

set(STM32_FLASH_SIZE  "0x80000" CACHE STRING "Flash size in bytes (512 KB)" FORCE)
set(STM32_SRAM_SIZE   "0x20000" CACHE STRING "SRAM size in bytes (128 KB)" FORCE)
set(STM32_CPU_CLOCK_HZ "170000000" CACHE STRING "CPU clock frequency" FORCE)

add_compile_definitions(STM32G474xx)
add_compile_definitions(STM32_FAMILY_G4)
add_compile_definitions(CAN_TYPE_FDCAN)

set(STM32_LINKER_SCRIPT
    "${CMAKE_CURRENT_LIST_DIR}/../bsp/bspMcu/linker/STM32G474RExx_FLASH.ld"
    CACHE STRING "Linker script" FORCE)

set(STM32_STARTUP_ASM
    "${CMAKE_CURRENT_LIST_DIR}/../bsp/bspMcu/startup/startup_stm32g474xx.s"
    CACHE STRING "Startup assembly" FORCE)
