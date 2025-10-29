// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include <bsp/UartConfig.h>
#include <etl/error_handler.h>

namespace bsp
{

uint32_t const baudRateConfig[] = {
    (LPUART_BAUD_OSR(15)) + LPUART_BAUD_SBR(26), // = 115200 48MHz FIRC
    (LPUART_BAUD_OSR(9)) + LPUART_BAUD_SBR(8)    // = 2MBit 80MHz PLL
};

Uart::UartConfig const CONFIG_UART[] = {
    {
        *LPUART1,
        bios::Io::UART1_TX,
        bios::Io::UART1_RX,
        baudRateConfig[0],
    },
};

static Uart instances[] = {
    Uart(CONFIG_UART[static_cast<size_t>(Uart::Id::TERMINAL)]),
};

bsp::Uart& Uart::getInstance(Id id)
{
    ETL_ASSERT(
        id < Id::INVALID, ETL_ERROR_GENERIC("UartId::INVALID is not a valid Uart identifier"));
    static_assert(
        NUMBER_OF_UARTS == static_cast<size_t>(etl::size(instances)),
        "Not enough Uart instances defined");
    return instances[static_cast<size_t>(id)];
}

} // namespace bsp
