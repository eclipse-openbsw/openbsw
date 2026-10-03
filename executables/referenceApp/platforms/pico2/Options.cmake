# *****************************************************************************
# Copyright (c) 2026 Roland Reichwein
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *****************************************************************************

set(OPENBSW_PLATFORM rp2xxx)
set(RP2XXX_CHIP RP2350)

# The vendored pico-sdk C headers use static_assert, which needs C11.
set(CMAKE_C_STANDARD 11)

set(_PICO2_ARCH_FLAGS "-mcpu=cortex-m33 -mthumb -mfloat-abi=hard \
-mfpu=fpv5-sp-d16")

set(CMAKE_ASM_FLAGS "${_PICO2_ARCH_FLAGS} -g -O3")

set(_PICO2_COMMON_FLAGS
    "${_PICO2_ARCH_FLAGS} -g -O3 -fshort-enums \
-mno-unaligned-access -Wno-psabi \
-fno-asynchronous-unwind-tables -fno-builtin -fno-common \
-ffunction-sections -fdata-sections -fstack-usage \
-DNDEBUG")

# -ffreestanding is only applied to C, as it is for the other ARM platforms. In
# C++ it makes newlib's <math.h> (pulled in by etl/limits.h) fail its
# hosted-only guard.
set(CMAKE_C_FLAGS "${_PICO2_COMMON_FLAGS} -ffreestanding")
set(CMAKE_CXX_FLAGS "${_PICO2_COMMON_FLAGS} -fno-rtti -fno-exceptions \
-fno-non-call-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit")
# Unlike the RP2040, the RP2350 bootrom exposes no v1 math shims and the
# Cortex-M33 has a hardware FPU, so no compiler runtime or math symbols are
# wrapped here. Only the pico-sdk stdio implementation is wrapped.
set(CMAKE_EXE_LINKER_FLAGS
    "${_PICO2_ARCH_FLAGS} -g -O3 \
-Wl,--gc-sections -Wl,-Map,application.map,--cref --specs=nosys.specs -nostartfiles -nolibc \
-Wl,--wrap=printf -Wl,--wrap=vprintf -Wl,--wrap=puts -Wl,--wrap=putchar -Wl,--wrap=getchar"
)

set(APPLICATION_LINK_ADD "boot_stage2_library")

# The pico-sdk crt0 owns the vector table and names the scheduler exceptions
# isr_pendsv / isr_systick, for which it only provides weak defaults. ThreadX
# names its handlers differently, so alias them here. Otherwise PendSV and
# SysTick are dispatched to the SDK default handler and the scheduler never
# runs. SVC is not aliased: the Cortex-M33 port runs in single mode and
# therefore defines no SVC_Handler. The FreeRTOS port does the equivalent
# renaming in its portmacro.h.
if (BUILD_TARGET_RTOS STREQUAL "THREADX")
    string(APPEND CMAKE_EXE_LINKER_FLAGS
           " -Wl,--defsym=isr_pendsv=PendSV_Handler"
           " -Wl,--defsym=isr_systick=SysTick_Handler")
endif ()

set(PLATFORM_SUPPORT_IO
    OFF
    CACHE BOOL "Turn IO support on or off" FORCE)
set(PLATFORM_SUPPORT_CAN
    OFF
    CACHE BOOL "Turn CAN support on or off" FORCE)
set(PLATFORM_SUPPORT_WATCHDOG
    ON
    CACHE BOOL "Turn ON Watchdog support" FORCE)
set(PLATFORM_SUPPORT_MPU
    OFF
    CACHE BOOL "Turn ON MPU support" FORCE)
set(PLATFORM_SUPPORT_ETHERNET
    OFF
    CACHE BOOL "Turn ethernet support on or off" FORCE)
