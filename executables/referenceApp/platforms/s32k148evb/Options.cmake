# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM s32k1xx)

set(PLATFORM_SUPPORT_IO
    ON
    CACHE BOOL "Turn IO support on or off" FORCE)
set(PLATFORM_SUPPORT_CAN
    ON
    CACHE BOOL "Turn CAN support on or off" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    ON
    CACHE BOOL "Turn ethernet support on or off" FORCE)
set(PLATFORM_SUPPORT_TRANSPORT
    ON
    CACHE BOOL "Turn TRANSPORT support on or off" FORCE)
set(PLATFORM_SUPPORT_UDS
    ON
    CACHE BOOL "Turn UDS support on or off" FORCE)
set(PLATFORM_SUPPORT_WATCHDOG
    ON
    CACHE BOOL "Turn ON Watchdog support" FORCE)
set(PLATFORM_SUPPORT_MPU
    ON
    CACHE BOOL "Turn ON MPU support" FORCE)
set(PLATFORM_SUPPORT_STORAGE
    ON
    CACHE BOOL "Turn persistent storage on or off" FORCE)
set(PLATFORM_SUPPORT_ROM_CHECK
    OFF
    CACHE BOOL "Turn ON ROM check support" FORCE)
