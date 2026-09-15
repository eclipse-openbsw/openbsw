/********************************************************************************
 * Copyright (c) 2024 Roland Reichwein
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#include "charInputOutput/charIoSerial.h"
#include "platform/estdint.h"

extern "C" void putByteToStdout(uint8_t const byte)
{
    (void)SerialLogger_putc(static_cast<int32_t>(byte));
}

extern "C" int32_t getByteFromStdin()
{
    uint8_t const byte = static_cast<uint8_t>(SerialLogger_getc());
    return (byte != 0U) ? byte : -1;
}
