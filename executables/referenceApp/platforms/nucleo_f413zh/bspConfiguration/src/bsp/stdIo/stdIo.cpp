/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/uart/UartConfig.h"
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
    if (uart.read(etl::span<uint8_t>(&dataByte, 1U)) == 0U)
    {
        return -1;
    }
    return dataByte;
}
