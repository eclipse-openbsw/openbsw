// Copyright 2024 Accenture.
// Copyright 2025 BMW AG

#include "bsp/UartPrivate.h"

#include <bsp/clock/boardClock.h>
#include <util/estd/assert.h>

using bsp::Uart;

static uint32_t const WRITE_TIMEOUT = 1000U;

void Uart::init()
{
    (void)bios::Io::setDefaultConfiguration(_uartConfig.txPin);
    (void)bios::Io::setDefaultConfiguration(_uartConfig.rxPin);
    _uartConfig.uart.GLOBAL = 0;
    _uartConfig.uart.CTRL   = 0;

    _uartConfig.uart.PINCFG = 0;
    _uartConfig.uart.BAUD   = 0;
    _uartConfig.uart.BAUD   = _uartConfig.baudRate[0].baud;
    _uartConfig.uart.STAT   = 0xFFFFFFFFU;
    _uartConfig.uart.STAT   = 0;
    _uartConfig.uart.MODIR  = 0;

    _uartConfig.uart.FIFO  = 0;
    _uartConfig.uart.WATER = 0;
    // Last
    _uartConfig.uart.CTRL  = LPUART_CTRL_RE(1U) + LPUART_CTRL_TE(1U);
}

bool Uart::isRxReady() const
{
    if ((_uartConfig.uart.STAT & LPUART_STAT_OR_MASK) != 0)
    {
        _uartConfig.uart.STAT = _uartConfig.uart.STAT | LPUART_STAT_OR_MASK;
    }
    return ((_uartConfig.uart.STAT & LPUART_STAT_RDRF_MASK) != 0);
}

size_t Uart::read(::etl::span<uint8_t> data)
{
    size_t bytes_read = 0;

    if (data.size() == 0U)
    {
        return 0;
    }

    while (isRxReady() && (bytes_read < data.size()))
    {
        data[bytes_read] = _uartConfig.uart.DATA & 0xFFU;
        bytes_read++;
    }

    return bytes_read;
}

bool Uart::isTxActive() const { return ((_uartConfig.uart.STAT & LPUART_STAT_TDRE_MASK) == 0); }

size_t Uart::write(::etl::span<uint8_t const> const& data)
{
    size_t counter = 0;

    if (data.size() == 0U)
    {
        return 0;
    }

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
    if (!isTxActive())
    {
        _uartConfig.uart.DATA = (static_cast<uint32_t>(data) & 0xFFU);
        uint32_t count        = 0U;
        while (isTxActive())
        {
            if (++count > WRITE_TIMEOUT)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
