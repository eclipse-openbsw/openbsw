#pragma once

#include <bsp/uart/UartConcept.h>
#include <gmock/gmock.h>

namespace bsp
{

class UartMock
{
public:
    MOCK_METHOD1(write, size_t(::etl::span<uint8_t const> const data));
    MOCK_METHOD1(read, size_t(::etl::span<uint8_t> data));
};

BSP_UART_CONCEPT_CHECKER(UartMock)

} // namespace bsp
