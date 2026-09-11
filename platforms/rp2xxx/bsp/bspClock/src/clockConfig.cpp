/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "clock/clockConfig.h"

#include <mcu/mcu.h>
// clang-format off
// needs to be included after mcu.h
#include <bsp/clock/boardClock.h>
// clang-format on

#include <etl/infinite_loop.h>
#include <platform/estdint.h>

#ifdef SCG_SPLLCSR_SPLLEN_MASK
#define PLL_AVAILABLE 1
#else
#define PLL_AVAILABLE 0
#endif

extern "C"
{
void configurPll() {}

void fircOff() {}

void extOscOff() {}

void pllOff() {}

void systemClock2Sirc() {}

void systemClock2Firc() {}

void systemClock2Pll() {}
} // extern "C"
