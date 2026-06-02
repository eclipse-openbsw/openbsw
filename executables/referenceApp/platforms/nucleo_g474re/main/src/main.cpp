/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

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

    // Early USART2 TX (PA2 AF7, 115200 @ 170 MHz), usable before the BSP
    // UART driver is initialized.
    // BRR = 170 000 000 / 115 200 = 1475.69 -> 1476
    static constexpr uint8_t USART_TX_PIN             = 2U;
    static constexpr uint8_t USART_TX_AF              = 7U; // AF7 = USART2_TX on PA2
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
