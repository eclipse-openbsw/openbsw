// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   clockConfig_f4.cpp
 * \brief  PLL configuration for STM32F413ZH (96 MHz from HSI).
 *
 * Clock tree: HSE 8 MHz bypass (ST-LINK MCO) -> PLL -> 96 MHz SYSCLK.
 * - APB1 = SYSCLK / 2 = 48 MHz (max 50 MHz per datasheet)
 * - APB2 = SYSCLK     = 96 MHz
 * - Flash latency = 3 WS @ 96 MHz / 3.3 V
 */

#include <mcu/mcu.h>

uint32_t SystemCoreClock = 16000000U; // Default HSI, updated by configurePll()

/**
 * \brief  Configure the PLL and switch SYSCLK for the STM32F413ZH.
 *
 * Clock tree: HSE 8 MHz bypass -> PLL (M=8, N=384, P=4) -> 96 MHz SYSCLK.
 * Sets flash wait states, AHB/APB prescalers, and updates SystemCoreClock.
 *
 * \note Called from the startup code before main(). Must not use any
 *       RTOS primitives or BSW services.
 */
extern "C" void configurePll()
{
    // Enable HSE bypass (8 MHz from ST-LINK)
    RCC->CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {}

    // Configure PLL: HSE / M=8 * N=384 / P=4 = 96 MHz
    RCC->PLLCFGR = (8U << RCC_PLLCFGR_PLLM_Pos)     // M = 8 -> VCO input = 1 MHz
                   | (384U << RCC_PLLCFGR_PLLN_Pos) // N = 384 -> VCO = 384 MHz
                   | (1U << RCC_PLLCFGR_PLLP_Pos)   // P = 4 (register value 1) -> 96 MHz
                   | RCC_PLLCFGR_PLLSRC_HSE;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {}

    // Flash latency 3 WS for 96 MHz @ 3.3V
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    // AHB = SYSCLK, APB1 = SYSCLK/2, APB2 = SYSCLK
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}

    SystemCoreClock = 96000000U;
}
