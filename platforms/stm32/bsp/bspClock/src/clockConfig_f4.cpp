/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include <mcu/mcu.h>

uint32_t SystemCoreClock = 16000000U; // Default HSI, updated by configurePll()

// Clock stabilization timeout (~100 ms at 16 MHz HSI = 1.6M cycles).
static constexpr uint32_t CLK_TIMEOUT = 1600000U;

extern "C" void configurePll()
{
    // Enable HSE bypass (8 MHz from ST-LINK)
    RCC->CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CR & RCC_CR_HSERDY) == 0U)
        {
            if (--t == 0U)
            {
                return;
            }
        }
    }

    // Configure PLL: HSE / M=8 * N=384 / P=4 = 96 MHz
    RCC->PLLCFGR = (8U << RCC_PLLCFGR_PLLM_Pos)     // M = 8 -> VCO input = 1 MHz
                   | (384U << RCC_PLLCFGR_PLLN_Pos) // N = 384 -> VCO = 384 MHz
                   | (1U << RCC_PLLCFGR_PLLP_Pos)   // P = 4 (register value 1) -> 96 MHz
                   | RCC_PLLCFGR_PLLSRC_HSE;

    RCC->CR |= RCC_CR_PLLON;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CR & RCC_CR_PLLRDY) == 0U)
        {
            if (--t == 0U)
            {
                return;
            }
        }
    }

    // Flash latency 3 WS for 96 MHz @ 3.3V
    FLASH->ACR = FLASH_ACR_LATENCY_3WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;

    // AHB = SYSCLK, APB1 = SYSCLK/2, APB2 = SYSCLK
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_SW_PLL;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        {
            if (--t == 0U)
            {
                return;
            }
        }
    }

    SystemCoreClock = 96000000U;
}
