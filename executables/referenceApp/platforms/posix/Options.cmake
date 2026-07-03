# *******************************************************************************
# Copyright (c) 2024 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM posix)

if (NOT CMAKE_SYSTEM_NAME OR NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(PLATFORM_SUPPORT_CAN
        ON
        CACHE BOOL "Turn CAN support on or off" FORCE)

    set(PLATFORM_SUPPORT_IO
        OFF
        CACHE BOOL "Turn IO support on or off" FORCE)
endif ()

set(PLATFORM_SUPPORT_WATCHDOG
    OFF
    CACHE BOOL "Turn OFF Watchdog support" FORCE)
set(PLATFORM_SUPPORT_MPU
    OFF
    CACHE BOOL "Turn OFF MPU support" FORCE)
set(PLATFORM_SUPPORT_ROM_CHECK
    OFF
    CACHE BOOL "Turn ROM check support off" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    ON
    CACHE BOOL "Turn ethernet support on or off" FORCE)
set(PLATFORM_SUPPORT_STORAGE
    ON
    CACHE BOOL "Turn persistent storage on or off" FORCE)
set(PLATFORM_SUPPORT_TRANSPORT
    ON
    CACHE BOOL "Turn TRANSPORT support on or off" FORCE)
set(PLATFORM_SUPPORT_UDS
    ON
    CACHE BOOL "Turn UDS support on or off" FORCE)
set(PLATFORM_SUPPORT_MIDDLEWARE
    ON
    CACHE BOOL "Turn middleware service demo support on or off" FORCE)
