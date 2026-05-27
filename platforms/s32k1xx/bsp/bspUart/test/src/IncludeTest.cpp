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

#include <gtest/gtest.h>

namespace
{
TEST(BspTest, IncludeCheck)
{
    bsp::Uart uartMock;
    uartMock.write(::etl::span<uint8_t const>{});
    uartMock.read(::etl::span<uint8_t>{});
}

} // anonymous namespace
