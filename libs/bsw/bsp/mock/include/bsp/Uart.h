/********************************************************************************
 * Copyright (c) 2024 Accenture
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <bsp/uart/UartConcept.h>
#include <gmock/gmock.h>

namespace bsp
{

class Uart
{
public:
    // Integration interface
    enum class Id : size_t;
    MOCK_METHOD(void, init, ());
    MOCK_METHOD(bool, isInitialized, ());

    // TODO: Figure out a way to mock singleton pattern properly
    static Uart& getInstance(Id const)
    {
        static Uart instance;
        return instance;
    }

    // Application interface
    MOCK_METHOD(size_t, write, (::etl::span<uint8_t const>), (const));
    MOCK_METHOD(size_t, read, (::etl::span<uint8_t>));
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp

#include "bsp/uart/UartConfig.h"
