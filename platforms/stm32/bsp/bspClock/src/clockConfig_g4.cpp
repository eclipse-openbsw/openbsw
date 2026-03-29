// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file   clockConfig_g4.cpp
 * \brief  PLL configuration for STM32G474RE (170 MHz with boost mode).
 *
 * Clock tree: HSI 16 MHz -> PLL (M=4, N=85, R=2) -> 170 MHz SYSCLK.
 * - APB1 = SYSCLK = 170 MHz
 * - APB2 = SYSCLK = 170 MHz
 * - Flash latency = 4 WS @ 170 MHz / 3.3 V, boost mode enabled
 *
 * HSI is used instead of HSE because NUCLEO-G474RE solder bridge SB15
 * may not route ST-LINK V3 MCO to PH0/OSC_IN on all board revisions.
 *
 * Boost-mode transition sequence per RM0440 section 6.1.5:
 *  1. Set AHB prescaler /2 (required for >150 MHz transition)
 *  2. Switch SYSCLK to PLL
 *  3. Wait SWS = PLL
 *  4. Wait >= 1 us for voltage regulator to settle
 *  5. Restore AHB prescaler /1
 *
 * \note DBG_SWEN (FLASH_ACR bit 18) is preserved via read-modify-write
 *       to avoid killing SWD debug access.
 */

#include <mcu/mcu.h>

uint32_t SystemCoreClock = 16000000U; // Default HSI, updated by configurePll()

/**
 * \brief  Configure the PLL with boost mode for the STM32G474RE.
 *
 * Enables voltage Range 1 boost mode, configures PLL for 170 MHz SYSCLK,
 * sets flash wait states (preserving DBG_SWEN via read-modify-write),
 * and performs the RM0440 section 6.1.5 AHB prescaler transition sequence.
 *
 * \note Called from startup code before main(). Must not use any
 *       RTOS primitives or BSW services.
 */
// Clock stabilization timeout (~100 ms at 16 MHz HSI = 1.6M cycles).
// If any oscillator or PLL fails to lock within this window, the system
// stays on HSI 16 MHz and SystemCoreClock is NOT updated — a clear
// indicator to downstream code that PLL init failed.
static constexpr uint32_t CLK_TIMEOUT = 1600000U;

extern "C" void configurePll()
{
    // Enable power interface clock
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    uint32_t volatile dummy = RCC->APB1ENR1; // Read-back for clock enable delay
    (void)dummy;

    // Set voltage scaling Range 1 (VOS = 01, default after reset)
    // Then enable boost mode (required for >150 MHz)
    PWR->CR5 &= ~PWR_CR5_R1MODE;
    // Wait for voltage scaling to stabilize
    {
        uint32_t t = CLK_TIMEOUT;
        while ((PWR->SR2 & PWR_SR2_VOSF) != 0U) { if (--t == 0U) { return; } }
    }

    // Configure PLL: HSI / M=4 * N=85 / R=2 = 170 MHz
    // VCO input = 16/4 = 4 MHz, VCO output = 4*85 = 340 MHz, PLLCLK = 340/2 = 170 MHz
    RCC->PLLCFGR = (3U << RCC_PLLCFGR_PLLM_Pos)    // M = 4 (register value M-1 = 3)
                   | (85U << RCC_PLLCFGR_PLLN_Pos) // N = 85
                   | RCC_PLLCFGR_PLLSRC_HSI
                   | RCC_PLLCFGR_PLLREN; // Enable PLL R output (SYSCLK source)

    RCC->CR |= RCC_CR_PLLON;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CR & RCC_CR_PLLRDY) == 0U) { if (--t == 0U) { return; } }
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
        while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_4WS) { if (--t == 0U) { return; } }
    }

    // Boost-mode >150 MHz transition: set AHB prescaler /2 before clock switch
    RCC->CFGR = RCC_CFGR_HPRE_DIV2 | RCC_CFGR_SW_PLL;
    {
        uint32_t t = CLK_TIMEOUT;
        while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) { if (--t == 0U) { return; } }
    }

    // Wait >= 1 us for voltage regulator to settle at new frequency
    // At 170/2 = 85 MHz, 100 cycles > 1 us
    for (uint32_t volatile i = 0U; i < 100U; i++) {}

    // Restore AHB prescaler to /1
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_HPRE_Msk) | RCC_CFGR_HPRE_DIV1;

    SystemCoreClock = 170000000U;
}
