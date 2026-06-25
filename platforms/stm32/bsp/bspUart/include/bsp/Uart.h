/********************************************************************************
 * Copyright (c) 2026 An Dao
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

#pragma once

#include <bsp/uart/UartConcept.h>

namespace bsp
{

class Uart
{
public:
    enum class Id : size_t;

    size_t write(::etl::span<uint8_t const> const data);
    size_t read(::etl::span<uint8_t> data);
    void init();
    bool isInitialized() const;
    bool waitForTxReady();

    static Uart& getInstance(Id id);

    struct UartConfig;
    Uart(Uart::Id id);

private:
    bool writeByte(uint8_t data);

    UartConfig const& _uartConfig;
    static UartConfig const _uartConfigs[];
};

BSP_UART_CONCEPT_CHECKER(Uart)

} // namespace bsp

#include <bsp/uart/UartConfig.h>
