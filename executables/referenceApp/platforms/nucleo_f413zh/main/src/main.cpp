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
#ifdef PLATFORM_SUPPORT_CAN
#include "systems/CanSystem.h"
#endif

#include <etl/alignment.h>
#include <lifecycle/LifecycleManager.h>
#include <safeSupervisor/SafeSupervisor.h>

extern void app_main();

extern "C"
{
void SystemInit()
{
    configurePll();

    // LED ON: PB0 = LD1 (green) on NUCLEO-F413ZH
    static constexpr uint8_t LED_PIN = 0U;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR; // Read-back for clock propagation
    GPIOB->MODER &= ~(3U << (LED_PIN * 2U));
    GPIOB->MODER |= (1U << (LED_PIN * 2U)); // GPIO output mode
    GPIOB->BSRR = (1U << LED_PIN);

    // Early USART3 TX (PD8 AF7, 115200 @ 48 MHz APB1), usable before the BSP
    // UART driver is initialized.
    // BRR = 48 000 000 / 115 200 = 416.67 -> 417
    static constexpr uint8_t USART_TX_PIN            = 8U;
    static constexpr uint8_t USART_TX_AF             = 7U; // AF7 = USART3_TX on PD8
    static constexpr uint32_t USART_BRR_115200_48MHZ = 417U;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    (void)RCC->APB1ENR;

    // PD8 = AF7 (USART3_TX) - pin 8 uses AFR[1], index = pin - 8 = 0
    GPIOD->MODER &= ~(3U << (USART_TX_PIN * 2U));
    GPIOD->MODER |= (2U << (USART_TX_PIN * 2U)); // Alternate function mode
    GPIOD->AFR[1] &= ~(0xFU << ((USART_TX_PIN - 8U) * 4U));
    GPIOD->AFR[1] |= (static_cast<uint32_t>(USART_TX_AF) << ((USART_TX_PIN - 8U) * 4U));

    USART3->CR1 = 0;
    USART3->CR2 = 0;
    USART3->CR3 = 0;
    USART3->BRR = USART_BRR_115200_48MHZ;
    USART3->CR1 = USART_CR1_TE | USART_CR1_UE;
    while ((USART3->SR & USART_SR_TC) == 0) {} // Wait for TX complete
}

// NVIC priorities for peripheral interrupts are configured by the owning
// systems during board bring-up.
void setupApplicationsIsr(void) {}
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
    // IWDG refresh - a no-op unless the watchdog is already running (it is
    // enabled later by the SafetyManager).
    IWDG->KR = 0xAAAAU;

    ::safety::safeSupervisorConstructor.construct();
    ::platform::staticBsp.init();
    IWDG->KR = 0xAAAAU;

    app_main();
    return 1;
}
