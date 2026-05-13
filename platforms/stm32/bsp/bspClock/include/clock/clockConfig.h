// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   clockConfig.h
 * \brief  Declaration of the platform-specific PLL configuration entry point.
 *
 * Each STM32 target provides its own implementation of configurePll()
 * in the corresponding clockConfig_*.cpp translation unit.
 */

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief Configure the system clock PLL for the target STM32 chip.
 *
 * - STM32F413ZH: HSE 8 MHz bypass -> PLL -> 96 MHz SYSCLK, APB1 48 MHz
 * - STM32G474RE: HSI 16 MHz       -> PLL -> 170 MHz SYSCLK (boost mode)
 *
 * \note Called from startup assembly before main(). Implementations must
 *       not use heap, RTOS, or BSW services.
 */
void configurePll(void);

#ifdef __cplusplus
}
#endif
