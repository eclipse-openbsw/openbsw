# Copyright 2024 Contributors to the Eclipse Foundation
#
# SPDX-License-Identifier: EPL-2.0

# STM32F413ZH chip configuration (NUCLEO-F413ZH)
# Cortex-M4F, 96 MHz, 1.5 MB Flash, 320 KB SRAM, bxCAN x3, USART3 VCP

set(STM32_FAMILY "F4" CACHE STRING "STM32 family" FORCE)
set(STM32_DEVICE "stm32f413xx" CACHE STRING "STM32 device define" FORCE)
set(STM32_DEVICE_UPPER "STM32F413xx" CACHE STRING "STM32 device define (upper)" FORCE)
set(CAN_TYPE "BXCAN" CACHE STRING "CAN peripheral type" FORCE)

add_compile_definitions(STM32F413xx)
add_compile_definitions(STM32_FAMILY_F4)
add_compile_definitions(CAN_TYPE_BXCAN)

set(STM32_STARTUP_ASM
    "${CMAKE_CURRENT_LIST_DIR}/../bsp/bspMcu/startup/startup_stm32f413xx.s"
    CACHE STRING "Startup assembly" FORCE)
