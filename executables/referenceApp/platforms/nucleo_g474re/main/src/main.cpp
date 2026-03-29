// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

/**
 * \file main.cpp
 * \brief Platform entry point for NUCLEO-G474RE (STM32G474RE, FDCAN)
 */

#include "async/Config.h"
#include "clock/clockConfig.h"
#include "lifecycle/StaticBsp.h"
#include "mcu/mcu.h"
#include "systems/CanSystem.h"

#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>

extern void app_main();

extern "C"
{
void SystemInit()
{
    configurePll();

    // LED ON: PA5 = LD2 (output push-pull)
    static constexpr uint8_t LED_PIN = 5U;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (LED_PIN * 2U));
    GPIOA->MODER |= (1U << (LED_PIN * 2U)); // GPIO output mode
    GPIOA->BSRR = (1U << LED_PIN);

    // Early USART2 for pre-main boot messages (PA2 AF7, 115200 @ 170 MHz)
    // BRR = 170 000 000 / 115 200 = 1475.69 → 1476
    static constexpr uint8_t USART_TX_PIN = 2U;
    static constexpr uint8_t USART_TX_AF  = 7U; // AF7 = USART2_TX on PA2
    static constexpr uint32_t USART_BRR_115200_170MHZ = 1476U;

    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    (void)RCC->APB1ENR1; // Read-back for clock propagation
    (void)RCC->APB1ENR1;
    GPIOA->MODER &= ~(3U << (USART_TX_PIN * 2U));
    GPIOA->MODER |= (2U << (USART_TX_PIN * 2U)); // Alternate function mode
    GPIOA->AFR[0] &= ~(0xFU << (USART_TX_PIN * 4U));
    GPIOA->AFR[0] |= (static_cast<uint32_t>(USART_TX_AF) << (USART_TX_PIN * 4U));
    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    USART2->BRR = USART_BRR_115200_170MHZ;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
    while ((USART2->ISR & USART_ISR_TEACK) == 0) {} // Wait for TX enable ack
}

// HardFault handler — dumps faulting PC/LR/CFSR on USART2
void HardFault_Handler(void)
{
    uint32_t* sp;
    __asm volatile("tst lr, #4\n"
                   "ite eq\n"
                   "mrseq %0, msp\n"
                   "mrsne %0, psp\n"
                   : "=r"(sp));
    uint32_t pc   = sp[6];
    uint32_t lr   = sp[5];
    uint32_t cfsr = *reinterpret_cast<uint32_t volatile*>(0xE000ED28);

    auto hf_putc = [](char c)
    {
        while ((USART2->ISR & USART_ISR_TXE_TXFNF) == 0) {}
        USART2->TDR = c;
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
    hf_hex(cfsr);
    hf_puts("\r\n");
    while ((USART2->ISR & USART_ISR_TC) == 0) {}

    while (1)
    {
        GPIOA->BSRR = (1U << 5U);
        for (int volatile d = 0; d < 50000; ++d) {}
        GPIOA->BSRR = (1U << (5U + 16U));
        for (int volatile d = 0; d < 50000; ++d) {}
    }
}

void setupApplicationsIsr(void)
{
    // FDCAN IRQs are enabled later in CanSystem::run() after transceiver init
    ENABLE_INTERRUPTS();
}
} // extern "C"

namespace platform
{
StaticBsp staticBsp;

StaticBsp& getStaticBsp() { return staticBsp; }

::etl::typed_storage<::systems::CanSystem> canSystem;

void platformLifecycleAdd(::lifecycle::LifecycleManager& lifecycleManager, uint8_t const level)
{
    if (level == 2U)
    {
        lifecycleManager.addComponent("can", canSystem.create(TASK_CAN), level);
    }
}
} // namespace platform

namespace systems
{
::can::ICanSystem& getCanSystem() { return *::platform::canSystem; }
} // namespace systems

int main()
{
    ::safety::safeSupervisorConstructor.construct();
    ::platform::staticBsp.init();
    app_main();
    return 1;
}
