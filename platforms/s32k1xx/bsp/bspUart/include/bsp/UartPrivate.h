/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "bsp/Uart.h"

#include <io/Io.h>
#include <mcu/mcu.h>

namespace bsp
{

struct Uart::UartConfig
{
    LPUART_Type& uart;
    bios::Io::PinId const txPin;
    bios::Io::PinId const rxPin;
    uint32_t const baudRate;
};

} // namespace bsp
