// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

// NUCLEO-G474RE: USART2 on PA2 (TX) / PA3 (RX), AF7
// ST-LINK VCP, 115200 baud @ 170 MHz APB1

#include <bsp/UartParams.h>
#include <bsp/uart/UartConfig.h>
#include <etl/error_handler.h>

namespace bsp
{

Uart::UartConfig const Uart::_uartConfigs[] = {
    {
        USART2,                // usart
        GPIOA,                 // gpioPort
        2U,                    // txPin (PA2)
        3U,                    // rxPin (PA3)
        7U,                    // af (AF7)
        1476U,                 // brr (170000000 / 115200 = 1475.69 -> 1476)
        RCC_AHB2ENR_GPIOAEN,   // rccGpioEnBit
        RCC_APB1ENR1_USART2EN, // rccUsartEnBit
        &RCC->AHB2ENR,         // rccGpioEnReg
        &RCC->APB1ENR1,        // rccUsartEnReg
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
