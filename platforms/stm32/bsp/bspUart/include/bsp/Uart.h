// Copyright 2024 Contributors to the Eclipse Foundation
//
// SPDX-License-Identifier: EPL-2.0

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
