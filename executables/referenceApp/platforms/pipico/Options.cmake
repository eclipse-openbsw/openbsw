# *******************************************************************************
# Copyright (c) 2026 Roland Reichwein
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

set(OPENBSW_PLATFORM rp2xxx)
set(RP2XXX_CHIP RP2040)

# The vendored pico-sdk C headers use static_assert, which needs C11.
set(CMAKE_C_STANDARD 11)

set(CMAKE_ASM_FLAGS "-g -O3 -mcpu=cortex-m0plus -mthumb")

set(_PIPICO_COMMON_FLAGS
    "-mcpu=cortex-m0plus -mthumb -g -O3 -fshort-enums \
-mno-unaligned-access -Wno-psabi \
-fno-asynchronous-unwind-tables -fno-builtin -fno-common \
-ffunction-sections -fdata-sections -fstack-usage \
-DNDEBUG")

# -ffreestanding is only applied to C, as it is for the other ARM platforms. In
# C++ it makes newlib's <math.h> (pulled in by etl/limits.h) fail its
# hosted-only guard.
set(CMAKE_C_FLAGS "${_PIPICO_COMMON_FLAGS} -ffreestanding")
set(CMAKE_CXX_FLAGS "${_PIPICO_COMMON_FLAGS} -fno-rtti -fno-exceptions \
-fno-non-call-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit")
# Only functions for which the vendored pico-sdk actually provides a __wrap_
# implementation are wrapped. Everything else (floor, pow, log10, round, malloc,
# ...) resolves to newlib, because the pico_float/pico_double math sources are
# not part of the vendored subset.
set(CMAKE_EXE_LINKER_FLAGS
    "-mcpu=cortex-m0plus -mthumb -g -O3 \
-Wl,--gc-sections -Wl,-Map,application.map,--cref --specs=nosys.specs -nostartfiles -nolibc \
-Wl,--wrap=__clzsi2 -Wl,--wrap=__clzdi2 -Wl,--wrap=__ctzsi2 -Wl,--wrap=__popcountsi2 -Wl,--wrap=__popcountdi2 -Wl,--wrap=__clz \
-Wl,--wrap=__clzl -Wl,--wrap=__clzll -Wl,--wrap=__ctzdi2 -Wl,--wrap=__aeabi_idiv -Wl,--wrap=__aeabi_idivmod -Wl,--wrap=__aeabi_ldivmod \
-Wl,--wrap=__aeabi_uidiv -Wl,--wrap=__aeabi_uidivmod -Wl,--wrap=__aeabi_uldivmod -Wl,--wrap=__aeabi_dadd -Wl,--wrap=__aeabi_ddiv -Wl,--wrap=__aeabi_dmul \
-Wl,--wrap=__aeabi_drsub -Wl,--wrap=__aeabi_dsub -Wl,--wrap=__aeabi_cdcmpeq -Wl,--wrap=__aeabi_cdrcmple -Wl,--wrap=__aeabi_cdcmple -Wl,--wrap=__aeabi_dcmpeq \
-Wl,--wrap=__aeabi_dcmplt -Wl,--wrap=__aeabi_dcmple -Wl,--wrap=__aeabi_dcmpge -Wl,--wrap=__aeabi_dcmpgt -Wl,--wrap=__aeabi_dcmpun -Wl,--wrap=__aeabi_i2d \
-Wl,--wrap=__aeabi_l2d -Wl,--wrap=__aeabi_ui2d -Wl,--wrap=__aeabi_ul2d -Wl,--wrap=__aeabi_d2iz -Wl,--wrap=__aeabi_d2lz -Wl,--wrap=__aeabi_d2uiz \
-Wl,--wrap=__aeabi_d2ulz -Wl,--wrap=__aeabi_d2f -Wl,--wrap=sqrt -Wl,--wrap=cos -Wl,--wrap=sin -Wl,--wrap=tan \
-Wl,--wrap=atan2 -Wl,--wrap=exp -Wl,--wrap=log -Wl,--wrap=sincos -Wl,--wrap=__aeabi_lmul -Wl,--wrap=__aeabi_fadd \
-Wl,--wrap=__aeabi_fdiv -Wl,--wrap=__aeabi_fmul -Wl,--wrap=__aeabi_frsub -Wl,--wrap=__aeabi_fsub -Wl,--wrap=__aeabi_cfcmpeq -Wl,--wrap=__aeabi_cfrcmple \
-Wl,--wrap=__aeabi_cfcmple -Wl,--wrap=__aeabi_fcmpeq -Wl,--wrap=__aeabi_fcmplt -Wl,--wrap=__aeabi_fcmple -Wl,--wrap=__aeabi_fcmpge -Wl,--wrap=__aeabi_fcmpgt \
-Wl,--wrap=__aeabi_fcmpun -Wl,--wrap=__aeabi_i2f -Wl,--wrap=__aeabi_ui2f -Wl,--wrap=__aeabi_f2iz -Wl,--wrap=__aeabi_f2uiz -Wl,--wrap=__aeabi_l2f \
-Wl,--wrap=__aeabi_ul2f -Wl,--wrap=__aeabi_f2lz -Wl,--wrap=__aeabi_f2ulz -Wl,--wrap=__aeabi_f2d -Wl,--wrap=sqrtf -Wl,--wrap=cosf \
-Wl,--wrap=sinf -Wl,--wrap=tanf -Wl,--wrap=atan2f -Wl,--wrap=expf -Wl,--wrap=logf -Wl,--wrap=sincosf \
-Wl,--wrap=__aeabi_memcpy -Wl,--wrap=__aeabi_memset -Wl,--wrap=__aeabi_memcpy4 -Wl,--wrap=__aeabi_memset4 -Wl,--wrap=__aeabi_memcpy8 -Wl,--wrap=__aeabi_memset8 \
-Wl,--wrap=printf -Wl,--wrap=vprintf -Wl,--wrap=puts -Wl,--wrap=putchar -Wl,--wrap=getchar -Wl,--wrap=memcpy \
-Wl,--wrap=memset")

set(APPLICATION_LINK_ADD "boot_stage2_library")

# The pico-sdk crt0 owns the vector table and names the scheduler exceptions
# isr_svcall / isr_pendsv / isr_systick, for which it only provides weak
# defaults. ThreadX names its handlers differently, so alias them here.
# Otherwise SVC, PendSV and SysTick are dispatched to the SDK default handler
# and the scheduler never runs. The FreeRTOS port does the equivalent renaming
# in its portmacro.h.
if (BUILD_TARGET_RTOS STREQUAL "THREADX")
    string(
        APPEND
        CMAKE_EXE_LINKER_FLAGS
        " -Wl,--defsym=isr_svcall=__tx_SVCallHandler"
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
