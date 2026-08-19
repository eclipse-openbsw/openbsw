# *******************************************************************************
# Copyright (c) 2026 An Dao
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM
    "stm32"
    CACHE STRING "" FORCE)
set(STM32_CHIP
    "STM32G474RE"
    CACHE STRING "" FORCE)
set(BUILD_TARGET_RTOS
    "FREERTOS"
    CACHE STRING "")
set(PLATFORM_SUPPORT_CAN
    ON
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_IO
    OFF
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    OFF
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_TRANSPORT
    ON
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_UDS
    ON
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_OBD_UDS_ADDRESSING
    ON
    CACHE BOOL "Turn OBD-style UDS addressing (0x7E0/0x7E8) on or off" FORCE)
set(PLATFORM_SUPPORT_PROGRAMMING_SESSION
    ON
    CACHE BOOL "Turn application-level UDS programming session on or off" FORCE)
set(PLATFORM_SUPPORT_UDS_DEMO_SERVICES
    ON
    CACHE BOOL "Turn the demo UDS services on or off" FORCE)
set(PLATFORM_SUPPORT_WATCHDOG
    OFF
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_MPU
    OFF
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_STORAGE
    OFF
    CACHE BOOL "" FORCE)
set(PLATFORM_SUPPORT_ROM_CHECK
    OFF
    CACHE BOOL "" FORCE)
