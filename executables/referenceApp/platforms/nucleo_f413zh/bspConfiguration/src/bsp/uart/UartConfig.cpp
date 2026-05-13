// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

// NUCLEO-F413ZH: USART3 on PD8 (TX) / PD9 (RX), AF7
// ST-LINK VCP, 115200 baud @ 48 MHz APB1

#include <bsp/UartParams.h>
#include <bsp/uart/UartConfig.h>
#include <etl/error_handler.h>

namespace bsp
{

Uart::UartConfig const Uart::_uartConfigs[] = {
    {
        USART3,               // usart
        GPIOD,                // gpioPort
        8U,                   // txPin (PD8)
        9U,                   // rxPin (PD9)
        7U,                   // af (AF7)
        417U,                 // brr (48000000 / 115200 = 416.67 -> 417)
        RCC_AHB1ENR_GPIODEN,  // rccGpioEnBit
        RCC_APB1ENR_USART3EN, // rccUsartEnBit
        &RCC->AHB1ENR,        // rccGpioEnReg
        &RCC->APB1ENR,        // rccUsartEnReg
    },
};

static Uart instances[] = {
    Uart(Uart::Id::TERMINAL),
};

Uart& Uart::getInstance(Id id)
{
    ETL_ASSERT(
        id < Id::INVALID, ETL_ERROR_GENERIC("UartId::INVALID is not a valid Uart identifier"));
    static_assert(
        NUMBER_OF_UARTS == static_cast<size_t>(etl::size(instances)),
        "Not enough Uart instances defined");
    return instances[static_cast<size_t>(id)];
}

} // namespace bsp
