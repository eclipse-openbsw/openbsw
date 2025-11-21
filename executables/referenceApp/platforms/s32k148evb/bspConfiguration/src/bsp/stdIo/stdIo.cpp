// Copyright 2024 Accenture.

#include "bsp/uart/UartConfig.h"
#include "charInputOutput/charIoSerial.h"
#include "platform/estdint.h"

extern "C" void putByteToStdout(uint8_t const byte)
{
    static bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
    uart.write(etl::span<uint8_t const>(&byte, 1U));
}

extern "C" int32_t getByteFromStdin()
{
    static bsp::Uart& uart = bsp::Uart::getInstance(bsp::Uart::Id::TERMINAL);
    uint8_t dataByte       = 0;
    etl::span<uint8_t> data(&dataByte, 1U);
    uart.read(data);
    if (data.size() == 0)
    {
        return -1;
    }
    return data[0];
}
