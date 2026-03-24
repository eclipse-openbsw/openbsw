// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file main.cpp
 * \brief Platform entry point for NUCLEO-F413ZH (STM32F413ZH, bxCAN)
 */

#include "async/Config.h"
#include "clock/clockConfig.h"
#include "lifecycle/StaticBsp.h"
#include "mcu/mcu.h"
#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif

#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>

extern void app_main();

// Early diagnostic via raw USART3 (bypasses BSP driver)
void diag_puts(char const* s)
{
    for (; *s; ++s)
    {
        while ((USART3->SR & USART_SR_TXE) == 0) {}
        USART3->DR = *s;
    }
}

// putchar_ used by etl::print
extern "C" void putchar_(char c)
{
    while ((USART3->SR & USART_SR_TXE) == 0) {}
    USART3->DR = c;
}

extern "C"
{
void SystemInit()
{
    configurePll();

    // LED ON: PB0 = LD1 (green) on NUCLEO-F413ZH
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;
    GPIOB->MODER &= ~(3U << (0U * 2U));
    GPIOB->MODER |= (1U << (0U * 2U));
    GPIOB->BSRR = (1U << 0U);

    // Early USART3 for pre-main boot messages (PD8 AF7, 115200 @ 48 MHz APB1)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    (void)RCC->APB1ENR;

    // PD8 = AF7 (USART3_TX)
    GPIOD->MODER &= ~(3U << (8U * 2U));
    GPIOD->MODER |= (2U << (8U * 2U));
    GPIOD->AFR[1] &= ~(0xFU << ((8U - 8U) * 4U));
    GPIOD->AFR[1] |= (7U << ((8U - 8U) * 4U));

    USART3->CR1 = 0;
    USART3->CR2 = 0;
    USART3->CR3 = 0;
    USART3->BRR = 417U; // 48 MHz / 115200 = 416.67
    USART3->CR1 = USART_CR1_TE | USART_CR1_UE;
    while ((USART3->SR & USART_SR_TC) == 0) {}

    // Print reset cause from RCC->CSR (using correct ST defines)
    {
        uint32_t csr = RCC->CSR;
        auto si_putc = [](char c)
        {
            while ((USART3->SR & USART_SR_TXE) == 0) {}
            USART3->DR = c;
        };
        auto si_puts = [&](char const* s)
        {
            for (; *s; ++s)
            {
                si_putc(*s);
            }
        };
        si_puts("[RST] ");
        if (csr & RCC_CSR_LPWRRSTF)
        {
            si_puts("LPWR ");
        }
        if (csr & RCC_CSR_WWDGRSTF)
        {
            si_puts("WWDG ");
        }
        if (csr & RCC_CSR_IWDGRSTF)
        {
            si_puts("IWDG ");
        }
        if (csr & RCC_CSR_SFTRSTF)
        {
            si_puts("SFT ");
        }
        if (csr & RCC_CSR_PORRSTF)
        {
            si_puts("POR ");
        }
        if (csr & RCC_CSR_PINRSTF)
        {
            si_puts("PIN ");
        }
        if (csr & RCC_CSR_BORRSTF)
        {
            si_puts("BOR ");
        }
        // Also print raw CSR for verification
        si_puts("CSR=0x");
        {
            char const h[] = "0123456789ABCDEF";
            for (int i = 28; i >= 0; i -= 4)
            {
                si_putc(h[(csr >> i) & 0xF]);
            }
        }
        si_puts("\r\n");
        // Clear reset flags
        RCC->CSR |= RCC_CSR_RMVF;
    }
}

// HardFault handler
void HardFault_Handler(void)
{
    IWDG->KR = 0xAAAAU;
    uint32_t* sp;
    __asm volatile("tst lr, #4\n"
                   "ite eq\n"
                   "mrseq %0, msp\n"
                   "mrsne %0, psp\n"
                   : "=r"(sp));
    uint32_t pc = sp[6];
    uint32_t lr = sp[5];

    auto hf_putc = [](char c)
    {
        while ((USART3->SR & USART_SR_TXE) == 0) {}
        USART3->DR = c;
    };
    auto hf_puts = [&](char const* s)
    {
        for (; *s; ++s)
        {
            hf_putc(*s);
        }
    };
    auto hf_hex = [&](uint32_t v)
    {
        char const h[] = "0123456789ABCDEF";
        for (int i = 28; i >= 0; i -= 4)
        {
            hf_putc(h[(v >> i) & 0xF]);
        }
    };

    hf_puts("\r\n!!! HARDFAULT !!!\r\n");
    hf_puts("  PC=0x");
    hf_hex(pc);
    hf_puts("  LR=0x");
    hf_hex(lr);
    hf_puts("  CFSR=0x");
    hf_hex(SCB->CFSR);
    hf_puts("  HFSR=0x");
    hf_hex(SCB->HFSR);
    hf_puts("  SP=0x");
    hf_hex((uint32_t)sp);
    hf_puts("\r\n");
    while ((USART3->SR & USART_SR_TC) == 0) {}

    while (1)
    {
        IWDG->KR = 0xAAAAU;
        GPIOB->ODR ^= (1U << 0U);
        for (int volatile d = 0; d < 200000; ++d) {}
    }
}

void setupApplicationsIsr(void)
{
    // CAN IRQs are enabled later in CanSystem::run() after transceiver init
    ENABLE_INTERRUPTS();
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

#ifdef PLATFORM_SUPPORT_CAN
::etl::typed_storage<::systems::CanSystem> canSystem;
#endif

void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    (void)lifecycleManager;
#ifdef PLATFORM_SUPPORT_CAN
    if (level == 2U)
    {
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
    }
#endif
    (void)level;
}
} // namespace platform

#ifdef PLATFORM_SUPPORT_CAN
namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems
#endif

int main()
{
    diag_puts("[diag] main() enter\r\n");
    IWDG->KR = 0xAAAAU;

    ::safety::safeSupervisorConstructor.construct();
    diag_puts("[diag] SafeSupervisor constructed\r\n");

    ::platform::staticBsp.init();
    diag_puts("[diag] staticBsp.init() done\r\n");
    IWDG->KR = 0xAAAAU;

    app_main();
    diag_puts("[diag] app_main() returned\r\n");
    return 1;
}
