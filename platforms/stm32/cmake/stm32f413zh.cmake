# *******************************************************************************
# Copyright (c) 2026 An Dao
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(STM32_FAMILY "F4")
set(STM32_DEVICE_UPPER "STM32F413xx")
set(CAN_TYPE "BXCAN")
set(STM32_STARTUP_ASM
    "${CMAKE_CURRENT_LIST_DIR}/../bsp/bspMcu/startup/startup_stm32f413xx.s")
