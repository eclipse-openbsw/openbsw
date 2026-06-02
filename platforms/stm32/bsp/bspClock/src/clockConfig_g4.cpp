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
// On timeout the system stays on HSI 16 MHz and SystemCoreClock is NOT
// updated, indicating to downstream code that PLL init failed.
static constexpr uint32_t CLK_TIMEOUT = 1600000U;

extern "C" void configurePll()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    uint32_t volatile dummy = RCC->APB1ENR1; // Read-back for clock enable delay
    (void)dummy;

    // Enable Range 1 boost mode (required for >150 MHz)
    PWR->CR5 &= ~PWR_CR5_R1MODE;
    // Wait for voltage scaling to stabilize
    {
        uint32_t t = CLK_TIMEOUT;
        while ((PWR->SR2 & PWR_SR2_VOSF) != 0U)
        {
            --t;
            if (t == 0U)
            {
                return;
            }
        }
    }

    // Configure PLL: HSI / M=4 * N=85 / R=2 = 170 MHz.
    // HSI instead of HSE: NUCLEO-G474RE solder bridge SB15 may not route
    // ST-LINK V3 MCO to PH0/OSC_IN on all board revisions.
    RCC->PLLCFGR = (3U << RCC_PLLCFGR_PLLM_Pos)    // M = 4 (register value M-1 = 3)
                   | (85U << RCC_PLLCFGR_PLLN_Pos) // N = 85
                   | RCC_PLLCFGR_PLLSRC_HSI
                   | RCC_PLLCFGR_PLLREN; // Enable PLL R output (SYSCLK source)

    RCC->CR |= RCC_CR_PLLON;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CR & RCC_CR_PLLRDY) == 0U)
        {
            --t;
            if (t == 0U)
            {
                return;
            }
        }
    }

    // Flash latency 4 WS for 170 MHz @ 3.3V boost mode
    // Preserve DBG_SWEN (bit 18) - clearing it kills SWD debug flash access
    {
        uint32_t acr = FLASH->ACR;
        acr &= ~FLASH_ACR_LATENCY_Msk; // Clear only latency bits, keep DBG_SWEN etc
        acr |= FLASH_ACR_LATENCY_4WS | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN;
        FLASH->ACR = acr;
    }
    // Verify flash latency readback before switching clock (RM0440 section 3.3.3)
    {
        uint32_t t = CLK_TIMEOUT;
        while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS)
        {
            --t;
            if (t == 0U)
            {
                return;
            }
        }
    }

    // Boost-mode >150 MHz transition: set AHB prescaler /2 before clock switch
    RCC->CFGR = RCC_CFGR_HPRE_DIV2 | RCC_CFGR_SW_PLL;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
        {
            --t;
            if (t == 0U)
            {
                return;
            }
        }
    }

    // Wait >= 1 us for voltage regulator to settle at new frequency
    // At 170/2 = 85 MHz, 100 cycles > 1 us
    for (uint32_t volatile i = 0U; i < 100U; i++) {}

    // Restore AHB prescaler to /1
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) | RCC_CFGR_HPRE_DIV1;

    SystemCoreClock = 170000000U;
}
