# *******************************************************************************
# Copyright (c) 2026 Accenture
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM posix)

if (NOT CMAKE_SYSTEM_NAME OR NOT CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(PLATFORM_SUPPORT_IO
        OFF
        CACHE BOOL "Turn IO support on or off" FORCE)
endif ()
