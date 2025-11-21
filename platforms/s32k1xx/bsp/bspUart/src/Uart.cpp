// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include "bsp/UartParams.h"

#include <bsp/clock/boardClock.h>

using bsp::Uart;

static uint32_t const WRITE_TIMEOUT = 1000U;

/**
 * it checks if there is active reading communication
 * @returns - true - there is no active reading data transfer
 */
static bool isRxReady(uint32_t volatile& uart_stat)
{
    if ((uart_stat & LPUART_STAT_OR_MASK) != 0)
    {
        uart_stat = uart_stat | LPUART_STAT_OR_MASK;
    }
    return ((uart_stat & LPUART_STAT_RDRF_MASK) != 0);
}

/**
 * it verifies if the transmission is completed and the hardware is ready for the next transfer
 * @returns - true - if tx transfer is active
 */
static bool isTxActive(uint32_t volatile& uart_stat)
{
    return ((uart_stat & LPUART_STAT_TDRE_MASK) == 0);
}

void Uart::init()
{
    (void)bios::Io::setDefaultConfiguration(_uartConfig.txPin);
    (void)bios::Io::setDefaultConfiguration(_uartConfig.rxPin);
    _uartConfig.uart.GLOBAL = 0;
    _uartConfig.uart.CTRL   = 0;

    _uartConfig.uart.PINCFG = 0;
    _uartConfig.uart.BAUD   = 0;
    _uartConfig.uart.BAUD   = _uartConfig.baudRate;
    _uartConfig.uart.STAT   = 0xFFFFFFFFU;
    _uartConfig.uart.STAT   = 0;
    _uartConfig.uart.MODIR  = 0;

    _uartConfig.uart.FIFO  = 0;
    _uartConfig.uart.WATER = 0;
    // Last
    _uartConfig.uart.CTRL  = LPUART_CTRL_RE(1U) + LPUART_CTRL_TE(1U);
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytes_read = 0;

    auto const& DATA = _uartConfig.uart.DATA;

    while (isRxReady(_uartConfig.uart.STAT) && (bytes_read < data.size()))
    {
        data[bytes_read] = DATA & 0xFFU;
        bytes_read++;
    }

    return bytes_read;
}

size_t Uart::write(::etl::span<uint8_t const> const data)
{
    size_t counter = 0;

    while (counter < data.size())
    {
        if (!writeByte(data[counter]))
        {
            break;
        }
        counter++;
    }

    return counter;
}

bool Uart::writeByte(uint8_t data)
{
    if (!isTxActive(_uartConfig.uart.STAT))
    {
        _uartConfig.uart.DATA = (static_cast<uint32_t>(data) & 0xFFU);
        return waitForTxReady();
    }
    return false;
}

bool Uart::isInitialized() const
{
    return (_uartConfig.uart.CTRL & (LPUART_CTRL_TE_MASK | LPUART_CTRL_RE_MASK)) != 0;
}

bool Uart::waitForTxReady()
{
    uint32_t count = 0U;
    while (isTxActive(_uartConfig.uart.STAT))
    {
        if (++count > WRITE_TIMEOUT)
        {
            return false;
        }
    }
    return true;
}
